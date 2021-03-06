/*++

Copyright (c) 2000 Microsoft Corporation

Module Name:

    wdatexit.c

Abstract:

    This file contains a version of atexit() designed specifically for
    use in Win16 DLLs.  It allows a Windows DLL to register functions
    to be called when a task that's using the DLLs exits (as opposed to
    standard atexit(), which goes away when the DLL exits).

Author:

    Danny Glasser (dannygl) - 25-Aug-2000

Revision History:

--*/

#include <windows.h>
#include <toolhelp.h>

#include <sysinc.h>
#include <rpc.h>
#include <rpcwin.h>

// Defined in ndr20\rpcssm.cxx
void NdrRpcDeleteAllocationContext();


// Prototypes for functions defined below
BOOL FAR PASCAL NotificationStart(void);
BOOL CALLBACK _loadds WinDLLExitHandlerCallback(WORD wID, DWORD dwData);


// Local definitions and data
#define TASK_LIST_INCREMENT         4
#define MAX_EXIT_LIST_FUNCTIONS     4   // Change to 32 to match ANSI

typedef struct
{
    HTASK id;
    int count;
    DLL_ATEXIT_FUNCTION func[MAX_EXIT_LIST_FUNCTIONS];
} TASK_EXIT_LIST;

// Dynamically-alloc'ed array of tasks
INTERNAL_VARIABLE TASK_EXIT_LIST *task_array = NULL;
// Size of array
INTERNAL_VARIABLE int task_array_size = 0;

INTERNAL_VARIABLE BOOL notified = FALSE;
INTERNAL_VARIABLE WORD selInstanceDLL;


INTERNAL_FUNCTION TASK_EXIT_LIST *
FindTaskEntry(
    HTASK task_id,
    BOOL create_entry
    )
{
    static TASK_EXIT_LIST *last_task = NULL;    // One-entry cache

    TASK_EXIT_LIST *task_ptr;
    int i, first_empty;

    // See if last task matches
    if (last_task != NULL && last_task->id == task_id)
        {
        task_ptr = last_task;
        }
    else
        {
        // Look for an entry
        for (i = 0, task_ptr = task_array, first_empty = -1;
             i < task_array_size;
             i++, task_ptr++)
            {
            if (task_ptr->id == task_id)
                break;

            if (create_entry && first_empty == -1 && task_ptr->id == 0)
                first_empty = i;
            }

        // Did we find an entry for this task?
        if (i == task_array_size)
            {
            // We didn't find one; should we create one?
            if (create_entry)
                {
                // If we found an empty entry, then use it
                if (first_empty != -1)
                    {
                    task_ptr = &task_array[first_empty];
                    }
                else
                    {
                    // No empty entry; enlarge the array
                    TASK_EXIT_LIST *new_array;
                    int new_array_size;

                    new_array_size = task_array_size + TASK_LIST_INCREMENT;

                    // Alloc or realloc, as appropriate
                    if (task_array == NULL)
                        {
                        new_array = (TASK_EXIT_LIST *)
                                    LocalAlloc(LPTR, new_array_size
                                                       * sizeof(*task_array));
                        }
                    else
                        {
                        new_array = (TASK_EXIT_LIST *)
                                    LocalReAlloc((HLOCAL) task_array,
                                                 new_array_size
                                                   * sizeof(*task_array),
                                                 LMEM_ZEROINIT);
                        }

                    ASSERT(new_array != NULL);

                    // Return immediately if the allocation failed
                    if (new_array == NULL)
                        return NULL;


                    // Set the global values
                    task_array = new_array;
                    task_array_size = new_array_size;

                    // The task element is the first of the newly-allocated
                    // ones; its index is the old value of task_array_size,
                    // which is equal to <i>.
                    task_ptr = &task_array[i];
                    }

                // If we get here, we have an entry for a new task

                // Register the notification callback
                if (! NotificationStart())
                    return NULL;

                // Set the <id> field for the task entry
                task_ptr->id = task_id;

                ASSERT(task_ptr->count == 0);

                } // end of new entry creation
            else
                {
                // We shouldn't create an entry
                task_ptr = NULL;
                }

            } // end of "if (i == task_array_size)"

        } // end of entry searching

    // Reset last_task and return
    if (task_ptr != NULL)
        last_task = task_ptr;

    return task_ptr;
}


BOOL PASCAL FAR
WinDLLAtExit(
    DLL_ATEXIT_FUNCTION exitfunc
    )
{
    HTASK task_id;
    TASK_EXIT_LIST *task_ptr;

    // Get task handle
    task_id = GetCurrentTask();

    ASSERT(task_id != NULL);

    // Look up task entry; create a new one if necessary
    task_ptr = FindTaskEntry(task_id, TRUE);

    ASSERT(task_ptr != NULL);

    if (task_ptr == NULL)
        return FALSE;

    // See if there's room for another function
    if (task_ptr->count < MAX_EXIT_LIST_FUNCTIONS)
        {
        task_ptr->func[task_ptr->count++] = exitfunc;
        }
    else
        {
        return FALSE;
        }

    // We're done
    return TRUE;
}


BOOL FAR PASCAL
NotificationStart(
    void
    )
{
    if (! notified)
    {
        // Convert DLL instance handle to a selector.  This is a workaround
        // for a ToolHelp bug that occurs on Win 3.0.
        selInstanceDLL = GlobalHandleToSel(hInstanceDLL);

        ASSERT(selInstanceDLL != 0);

        notified = NotifyRegister((HTASK) selInstanceDLL,
                                  WinDLLExitHandlerCallback,
                                  NF_NORMAL);

        ASSERT(notified);
    }

    return notified;
}


BOOL FAR PASCAL
NotificationStop(
    void
    )
{
    if (notified)
    {
        notified = ! NotifyUnRegister((HTASK) selInstanceDLL);

        ASSERT(! notified);
    }

    return ! notified;
}


BOOL CALLBACK __loadds
WinDLLExitHandlerCallback(
    WORD wID,
    DWORD dwData
    )
{
    // We only care about when the task exits
    // BUGBUG - What about when the DLL is released with FreeLibrary?
    if (wID == NFY_EXITTASK)
        {
        HTASK task_id;
        TASK_EXIT_LIST *task_ptr;

        // Get task handle
        task_id = GetCurrentTask();

        ASSERT(task_id != NULL);

        // Look for an existing entry
        task_ptr = FindTaskEntry(task_id, FALSE);

        if (task_ptr != NULL)
            {
            ASSERT(task_ptr->count > 0);

            // Call the exit list functions in LIFO order
            while (task_ptr->count)
                {
                (*(task_ptr->func[--task_ptr->count])) (task_id);
                }

            // Reset the entry
            task_ptr->id = 0;
            }

        // Delete any bindings that this task did not free.

        CloseBindings(task_id);

        // Delete the memory allocated (if any) allocated with rpcssm.

        NdrRpcDeleteAllocationContext();
        }

    return FALSE;
}
