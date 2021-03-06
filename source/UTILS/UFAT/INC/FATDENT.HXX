/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    fatdent.hxx

Abstract:

    This class models a FAT directory entry.

Author:

    Norbert P. Kusters (norbertk) 4-Dec-90

--*/

#if !defined(FAT_DIRENT_DEFN)

#define FAT_DIRENT_DEFN

DECLARE_CLASS( FAT_DIRENT );
DECLARE_CLASS( WSTRING );
DECLARE_CLASS( WSTRING        );
DECLARE_CLASS( TIMEINFO );

typedef struct _SHORT_FAT_DIRENT {

    UCHAR   Name[11];
    UCHAR   Attributes[1];
    UCHAR   Reserved[6];
    UCHAR   LastAccessDate[2];
    UCHAR   EaHandle[2];
    UCHAR   Time[2];
    UCHAR   Date[2];
    UCHAR   FirstCluster[2];
    UCHAR   Size[4];
};

DEFINE_TYPE( _SHORT_FAT_DIRENT, SHORT_FAT_DIRENT );

typedef struct _LONG_FAT_DIRENT {

    UCHAR   Ordinal[1];
    UCHAR   Name1[10];
    UCHAR   Attribute[1];
    UCHAR   Type[1];
    UCHAR   Checksum[1];
    UCHAR   Name2[12];
    UCHAR   FirstCluster[2];
    UCHAR   Name3[4];
};

DEFINE_TYPE( _LONG_FAT_DIRENT, LONG_FAT_DIRENT );

#define LONG_DIRENT_TYPE_NAME   0
#define LONG_DIRENT_TYPE_CLASS  1

class FAT_DIRENT : public OBJECT {

	public:

		DECLARE_CONSTRUCTOR( FAT_DIRENT );

		VIRTUAL
		~FAT_DIRENT(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN OUT  PVOID   Dirent
            );

        NONVIRTUAL
        VOID
        Clear(
            );

        NONVIRTUAL
        VOID
        QueryName(
            OUT PWSTRING    Name
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        SetName(
            IN  PCWSTRING   Name
            );

        NONVIRTUAL
        BOOLEAN
        IsValidName(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsDot(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsDotDot(
            ) CONST;

        NONVIRTUAL
        BYTE
        QueryAttributeByte(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsValidTimeStamp(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryTimeStamp(
            OUT LARGE_INTEGER   *TimeStamp
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        SetTimeStamp(
            );

        NONVIRTUAL
        USHORT
        QueryStartingCluster(
            ) CONST;

        NONVIRTUAL
        VOID
        SetStartingCluster(
            IN  USHORT  NewStartingCluster
            );

        NONVIRTUAL
        ULONG
        QueryFileSize(
            ) CONST;

        NONVIRTUAL
        VOID
        SetFileSize(
            IN  ULONG   NewFileSize
            );

        NONVIRTUAL
        USHORT
        QueryEaHandle(
            ) CONST;

        NONVIRTUAL
        VOID
        SetEaHandle(
            IN  USHORT  NewHandle
            );

        NONVIRTUAL
        BOOLEAN
        IsEndOfDirectory(
            ) CONST;

        NONVIRTUAL
        VOID
        SetEndOfDirectory(
            );

        NONVIRTUAL
        BOOLEAN
        IsErased(
            ) CONST;

        NONVIRTUAL
        VOID
        SetErased(
            );

        NONVIRTUAL
        BOOLEAN
        IsHidden(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsSystem(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsVolumeLabel(
            ) CONST;

        NONVIRTUAL
        VOID
        SetVolumeLabel(
            );

        NONVIRTUAL
        BOOLEAN
        IsDirectory(
            ) CONST;

        NONVIRTUAL
        VOID
        SetDirectory(
            );

        NONVIRTUAL
        VOID
        ResetDirectory(
            );

        NONVIRTUAL
        UCHAR
        QueryChecksum(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsLongEntry(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsLongNameEntry(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryLongOrdinal(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsLastLongEntry(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsWellTerminatedLongNameEntry(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryLongNameComponent(
            OUT PWSTRING    NameComponent
            ) CONST;

    private:

		NONVIRTUAL
		VOID
		Construct (
			);

        NONVIRTUAL
        VOID
        Destroy(
            );

        PUCHAR  _dirent;

};


INLINE
VOID
FAT_DIRENT::Clear(
    )
/*++

Routine Description:

    This routine zeros out the directory entry.

Arguments:

    None.

Return Value:

    None.

--*/
{
    memset(_dirent, 0, 32);
}


INLINE
BOOLEAN
FAT_DIRENT::IsDot(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the directory entry is the "."
    entry.

Arguments:

    None.

Return Value:

    FALSE   - The entry is not the "." entry.
    TRUE    - The entry is the "." entry.

--*/
{
    return !memcmp(_dirent, ".          ", 11);
}


INLINE
BOOLEAN
FAT_DIRENT::IsDotDot(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the directory entry is the ".."
    entry.

Arguments:

    None.

Return Value:

    FALSE   - The entry is not the ".." entry.
    TRUE    - The entry is the ".." entry.

--*/
{
    return !memcmp(_dirent, "..         ", 11);
}


INLINE
USHORT
FAT_DIRENT::QueryStartingCluster(
    ) CONST
/*++

Routine Description:

    This routine computes the starting cluster number of the directory entry.

Arguments:

    None.

Return Value:

    The starting cluster number of the directory entry.

--*/
{
    DbgAssert(_dirent);
    return *((PUSHORT) &_dirent[26]);
}


INLINE
VOID
FAT_DIRENT::SetStartingCluster(
    IN  USHORT  NewStartingCluster
    )
/*++

Routine Description:

    This routine sets the starting cluster number for the directory entry.

Arguments:

    NewStartingCluster  - Supplies the starting cluster number for the
                            directory entry.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    *((PUSHORT) &_dirent[26]) = NewStartingCluster;
}


INLINE
ULONG
FAT_DIRENT::QueryFileSize(
    ) CONST
/*++

Routine Description:

    This routine returns the number of bytes in the file.

Arguments:

    None.

Return Value:

    The number of bytes in the file.

--*/
{
    DbgAssert(_dirent);
    return *((PULONG) &_dirent[28]);
}


INLINE
VOID
FAT_DIRENT::SetFileSize(
    IN  ULONG   NewFileSize
    )
/*++

Routine Description:

    This routine sets the file size in the directory entry.

Arguments:

    NewFileSize - Supplies the new file size.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    *((PULONG) &_dirent[28]) = NewFileSize;
}


INLINE
USHORT
FAT_DIRENT::QueryEaHandle(
    ) CONST
/*++

Routine Description:

    This routine returns the EA handle for the file.

Arguments:

    None.

Return Value:

    The EA handle for the file.

--*/
{
    DbgAssert(_dirent);
    return *((PUSHORT) &_dirent[20]);
}


INLINE
VOID
FAT_DIRENT::SetEaHandle(
    IN  USHORT  NewHandle
    )
/*++

Routine Description:

    This routine sets the EA handle for the file.

Arguments:

    NewHandle   - Supplies the EA handle for the file.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    *((PUSHORT) &_dirent[20]) = NewHandle;
}


INLINE
BOOLEAN
FAT_DIRENT::IsEndOfDirectory(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not this directory entry marks
    the end of the directory.

Arguments:

    None.

Return Value:

    FALSE   - This entry does not mark the end of the directory.
    TRUE    - This entry marks the end of the directory.

--*/
{
    DbgAssert(_dirent);
    return _dirent[0] ? FALSE : TRUE;
}


INLINE
VOID
FAT_DIRENT::SetEndOfDirectory(
    )
/*++

Routine Description:

    This routine sets this directory entry marks to the end of the
    directory.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    _dirent[0] = 0;
}


INLINE
BOOLEAN
FAT_DIRENT::IsErased(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the directory entry is erased or not.

Arguments:

    None.

Return Value:

    FALSE   - The directory entry is not erased.
    TRUE    - The directory entry is erased.

--*/
{
    DbgAssert(_dirent);
    return _dirent[0] == 0xE5 ? TRUE : FALSE;
}


INLINE
VOID
FAT_DIRENT::SetErased(
    )
/*++

Routine Description:

    This routine marks the directory entry as erased.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    _dirent[0] = 0xE5;
}


INLINE
BOOLEAN
FAT_DIRENT::IsHidden(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the file is hidden.

Arguments:

    None.

Return Value:

    FALSE   - The file is not hidden.
    TRUE    - The file is hidden.

--*/
{
    DbgAssert(_dirent);
    return _dirent[11]&0x02 ? TRUE : FALSE;
}


INLINE
BOOLEAN
FAT_DIRENT::IsSystem(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the file is a system file.

Arguments:

    None.

Return Value:

    FALSE   - The file is not a system file.
    TRUE    - The file is a system file.

--*/
{
    DbgAssert(_dirent);
    return _dirent[11]&0x04 ? TRUE : FALSE;
}


INLINE
BOOLEAN
FAT_DIRENT::IsVolumeLabel(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the first 11 characters of the
    directory entry are the volume label or not.

Arguments:

    None.

Return Value:

    FALSE   - The directory entry is not a volume label.
    TRUE    - The directory entry is a volume label.

--*/
{
    DbgAssert(_dirent);
    return ((_dirent[11]&0x08) && !IsLongEntry());
}

INLINE
BOOLEAN
FAT_DIRENT::IsLongEntry(
    ) CONST
/*++

Routine Description;

    This routine determines whether this entry is a
    Long Directory Entry.

    The entry is a Long Directory Entry if the attribute
    field has all four low-order bits (read-only, hidden,
    system, and volume-label) set.  The four high-order
    bits are ignored.

Arguments:

    None.

Return Value:

    TRUE if the entry is a Long Name Directory Entry.

--*/
{
    return( (_dirent[11] & 0xF) == 0xF );
}

INLINE
BOOLEAN
FAT_DIRENT::IsLongNameEntry(
    ) CONST
/*++

Routine Description;

    This routine determines whether this entry is a
    Long Name Directory Entry.

    A Long Name directory entry is a Long Directory Entry
    with a type field of LONG_DIRENT_TYPE_LONG_NAME.

Arguments:

    None.

Return Value:

    TRUE if the entry is a Long Name Directory Entry.

--*/
{
    return( IsLongEntry() && (_dirent[12] == LONG_DIRENT_TYPE_NAME) );
}

INLINE
BOOLEAN
FAT_DIRENT::QueryLongOrdinal(
    ) CONST
/*++

Routine Description;

    This method returns the ordinal of a long directory entry.
    If the directory entry is not a long entry, it returns
    0xFF.  Note that this method strips off the Last Long Entry
    flag before returning the ordinal.  To determine if an entry
    is the last of a set of long entries, call IsLastLongEntry.

Arguments:

    None.

Return Value:

    The ordinal of this entry.

--*/
{
    return( IsLongEntry() ? _dirent[0] & 0x3F: 0xFF );
}

INLINE
BOOLEAN
FAT_DIRENT::IsLastLongEntry(
    ) CONST
/*++

Routine Description:

    This method determines whether this entry is the last
    of a set of long directory entries.

Arguments:

    None.

Return Value:

    TRUE if this is the last of a set of long directory entries.

--*/
{
    return( IsLongEntry() ? _dirent[0] & 0x40 : FALSE );
}


INLINE
VOID
FAT_DIRENT::SetVolumeLabel(
    )
/*++

Routine Description:

    This routine sets the directory entry to be a volume label.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    _dirent[11] |= 0x08;
}


INLINE
BOOLEAN
FAT_DIRENT::IsDirectory(
    ) CONST
/*++

Routine Description:

    This routine computes whether or not the directory entry is a directory.

Arguments:

    None.

Return Value:

    FALSE   - The directory entry is not a directory.
    TRUE    - The directory entry is a directory.

--*/
{
    DbgAssert(_dirent);
    return ((_dirent[11]&0x10) && !IsLongEntry());
}


INLINE
VOID
FAT_DIRENT::SetDirectory(
    )
/*++

Routine Description:

    This routine sets the directory entry to be a directory.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    _dirent[11] |= 0x10;
}


INLINE
VOID
FAT_DIRENT::ResetDirectory(
    )
/*++

Routine Description:

    This routine sets the directory entry to not be a directory.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DbgAssert(_dirent);
    _dirent[11] &= ~0x10;
}




INLINE
BYTE
FAT_DIRENT::QueryAttributeByte(
    ) CONST
/*++

Routine Description:

    This routine returns the attribute byte of the directory entry

Arguments:

    None.

Return Value:

    Attribute byte

--*/
{
    DbgAssert(_dirent);
    return _dirent[11];
}






#endif // FAT_DIRENT_DEFN
