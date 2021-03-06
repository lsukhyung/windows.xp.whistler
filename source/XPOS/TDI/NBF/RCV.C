/*++

Copyright (c) 2000, 1990, 2000  Microsoft Corporation

Module Name:

    rcv.c

Abstract:

    This module contains code which performs the following TDI services:

        o   TdiReceive
        o   TdiReceiveDatagram

Author:

    David Beaver (dbeaver) 1-July-2000

Environment:

    Kernel mode

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


NTSTATUS
NbfTdiReceive(
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine performs the TdiReceive request for the transport provider.

Arguments:

    Irp - I/O Request Packet for this request.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS status;
    PTP_CONNECTION connection;
    KIRQL oldirql;
    PIO_STACK_LOCATION irpSp;

    //
    // verify that the operation is taking place on a connection. At the same
    // time we do this, we reference the connection. This ensures it does not
    // get removed out from under us. Note also that we do the connection
    // lookup within a try/except clause, thus protecting ourselves against
    // really bogus handles
    //

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    connection = irpSp->FileObject->FsContext;

    //
    // Check that this is really a connection.
    //

    if ((connection->Size != sizeof (TP_CONNECTION)) ||
        (connection->Type != NBF_CONNECTION_SIGNATURE)) {
#if DBG
        NbfPrint2 ("TdiReceive: Invalid Connection %lx Irp %lx\n", connection, Irp);
#endif
        return STATUS_INVALID_CONNECTION;
    }

    //
    // Initialize bytes transferred here.
    //

    Irp->IoStatus.Information = 0;              // reset byte transfer count.

    // This reference is removed by NbfDestroyRequest.

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_C_SPIN_LOCK (&connection->SpinLock);

    if ((connection->Flags & CONNECTION_FLAGS_READY) == 0) {

        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        Irp->IoStatus.Status = connection->Status;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

        status = STATUS_PENDING;

    } else {

        //
        // Once the reference is in, LinkSpinLock will be valid.
        //

        NbfReferenceConnection("TdiReceive request", connection, CREF_RECEIVE_IRP);
        RELEASE_DPC_C_SPIN_LOCK (&connection->SpinLock);

        ACQUIRE_DPC_SPIN_LOCK (connection->LinkSpinLock);

        IRP_RECEIVE_IRP(irpSp) = Irp;
        IRP_RECEIVE_REFCOUNT(irpSp) = 1;

        //
        // Insert onto the receive queue, and make the IRP
        // cancellable.
        //

        InsertTailList (&connection->ReceiveQueue,&Irp->Tail.Overlay.ListEntry);

#if DBG
        NbfReceives[NbfReceivesNext].Irp = Irp;
        NbfReceives[NbfReceivesNext].Request = NULL;
        NbfReceives[NbfReceivesNext].Connection = (PVOID)connection;
        NbfReceivesNext = (NbfReceivesNext++) % TRACK_TDI_LIMIT;
#endif

        //
        // Set this the quick way, without the cancel lock.
        //

        Irp->CancelRoutine = NbfCancelReceive;

        //
        // If this IRP has been cancelled, then call the
        // cancel routine.
        //

        if (Irp->Cancel) {

            (VOID)RemoveHeadList (&connection->ReceiveQueue);

#if DBG
            NbfCompletedReceives[NbfCompletedReceivesNext].Irp = Irp;
            NbfCompletedReceives[NbfCompletedReceivesNext].Request = NULL;
            NbfCompletedReceives[NbfCompletedReceivesNext].Status = STATUS_CANCELLED;
            {
                ULONG i,j,k;
                PUCHAR va;
                PMDL mdl;

                mdl = Irp->MdlAddress;

                NbfCompletedReceives[NbfCompletedReceivesNext].Contents[0] = (UCHAR)0;

                i = 1;
                while (i<TRACK_TDI_CAPTURE) {
                    if (mdl == NULL) break;
                    va = MmGetSystemAddressForMdl (mdl);
                    j = MmGetMdlByteCount (mdl);

                    for (i=i,k=0;i<TRACK_TDI_CAPTURE&k<j;i++,k++) {
                        NbfCompletedReceives[NbfCompletedReceivesNext].Contents[i] = *va++;
                    }
                    mdl = mdl->Next;
                }
            }

            NbfCompletedReceivesNext = (NbfCompletedReceivesNext++) % TRACK_TDI_LIMIT;
#endif

            Irp->CancelRoutine = (PDRIVER_CANCEL)NULL;
            NbfCompleteReceiveIrp (Irp, STATUS_CANCELLED, 0);

            RELEASE_DPC_SPIN_LOCK (connection->LinkSpinLock);

        } else {

            //
            // This call releases the spinlock, and references the
            // connection first it if needs to access it after
            // releasing the lock.
            //

            AwakenReceive (connection);             // awaken if sleeping.

        }

        status = STATUS_PENDING;

    }

    KeLowerIrql (oldirql);

    return status;
} /* TdiReceive */


NTSTATUS
NbfTdiReceiveDatagram(
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine performs the TdiReceiveDatagram request for the transport
    provider. Receive datagrams just get queued up to an address, and are
    completed when a DATAGRAM or DATAGRAM_BROADCAST frame is received at
    the address.

Arguments:

    Irp - I/O Request Packet for this request.

Return Value:

    NTSTATUS - status of operation.

--*/

{
    NTSTATUS status;
    KIRQL oldirql;
    PTP_ADDRESS address;
    PTP_ADDRESS_FILE addressFile;
    PIO_STACK_LOCATION irpSp;

    //
    // verify that the operation is taking place on an address. At the same
    // time we do this, we reference the address. This ensures it does not
    // get removed out from under us. Note also that we do the address
    // lookup within a try/except clause, thus protecting ourselves against
    // really bogus handles
    //

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    addressFile = irpSp->FileObject->FsContext;

    status = NbfVerifyAddressObject (addressFile);

    if (!NT_SUCCESS (status)) {
        return status;
    }

#if DBG
    if (((PTDI_REQUEST_KERNEL_RECEIVEDG)(&irpSp->Parameters))->ReceiveLength > 0) {
        ASSERT (Irp->MdlAddress != NULL);
    }
#endif

    address = addressFile->Address;

    ACQUIRE_SPIN_LOCK (&address->SpinLock,&oldirql);
    if ((address->Flags & (ADDRESS_FLAGS_STOPPING | ADDRESS_FLAGS_CONFLICT)) != 0) {

        RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = (address->Flags & ADDRESS_FLAGS_STOPPING) ?
                    STATUS_NETWORK_NAME_DELETED : STATUS_DUPLICATE_NAME;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

    } else {

        //
        // Set this the quick way, without the cancel lock.
        //

        Irp->CancelRoutine = NbfCancelReceiveDatagram;

        //
        // If this IRP has been cancelled, then call the
        // cancel routine.
        //

        if (Irp->Cancel) {

            RELEASE_SPIN_LOCK (&address->SpinLock, oldirql);

            Irp->CancelRoutine = (PDRIVER_CANCEL)NULL;
            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

        } else {

            NbfReferenceAddress ("Receive datagram", address, AREF_REQUEST);
            InsertTailList (&addressFile->ReceiveDatagramQueue,&Irp->Tail.Overlay.ListEntry);
            RELEASE_SPIN_LOCK (&address->SpinLock,oldirql);

        }

    }

    NbfDereferenceAddress ("Temp rcv datagram", address, AREF_VERIFY);

    return STATUS_PENDING;

} /* TdiReceiveDatagram */

