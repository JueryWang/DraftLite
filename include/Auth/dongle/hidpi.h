/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

        HIDPI.H

Abstract:

   Public Interface to the HID parsing library.

Environment:

    Kernel & user mode

--*/

#ifndef   __HIDPI_H__
#define   __HIDPI_H__

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4115) // named type definition in parentheses
#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4214) // bit field types other than int

#include <pshpack4.h>

// Please include "hidsdi.h" to use the user space (dll / parser)
// Please include "hidpddi.h" to use the kernel space parser

//
// Special Link collection values for using the query functions
//
// Root collection references the collection at the base of the link
// collection tree.
// Unspecifies, references all collections in the link collection tree.
//
#define HIDP_LINK_COLLECTION_ROOT ((USHORT) -1)
#define HIDP_LINK_COLLECTION_UNSPECIFIED ((USHORT) 0)


typedef enum _HIDP_REPORT_TYPE
{
    HidP_Input,
    HidP_Output,
    HidP_Feature
} HIDP_REPORT_TYPE;

typedef struct _USAGE_AND_PAGE
{
    USAGE Usage;
    USAGE UsagePage;
} USAGE_AND_PAGE, *PUSAGE_AND_PAGE;

#define HidP_IsSameUsageAndPage(u1, u2) ((* (PULONG) &u1) == (* (PULONG) &u2))

typedef struct _HIDP_BUTTON_CAPS
{
    USAGE    UsagePage;
    UCHAR    ReportID;
    BOOLEAN  IsAlias;

    USHORT   BitField;
    USHORT   LinkCollection;   // A unique internal index pointer

    USAGE    LinkUsage;
    USAGE    LinkUsagePage;

    BOOLEAN  IsRange;
    BOOLEAN  IsStringRange;
    BOOLEAN  IsDesignatorRange;
    BOOLEAN  IsAbsolute;

    ULONG    Reserved[10];
    union {
        struct {
            USAGE    UsageMin,         UsageMax;
            USHORT   StringMin,        StringMax;
            USHORT   DesignatorMin,    DesignatorMax;
            USHORT   DataIndexMin,     DataIndexMax;
        } Range;
        struct  {
            USAGE    Usage,            Reserved1;
            USHORT   StringIndex,      Reserved2;
            USHORT   DesignatorIndex,  Reserved3;
            USHORT   DataIndex,        Reserved4;
        } NotRange;
    };

} HIDP_BUTTON_CAPS, *PHIDP_BUTTON_CAPS;


typedef struct _HIDP_VALUE_CAPS
{
    USAGE    UsagePage;
    UCHAR    ReportID;
    BOOLEAN  IsAlias;

    USHORT   BitField;
    USHORT   LinkCollection;   // A unique internal index pointer

    USAGE    LinkUsage;
    USAGE    LinkUsagePage;

    BOOLEAN  IsRange;
    BOOLEAN  IsStringRange;
    BOOLEAN  IsDesignatorRange;
    BOOLEAN  IsAbsolute;

    BOOLEAN  HasNull;        // Does this channel have a null report   union
    UCHAR    Reserved;
    USHORT   BitSize;        // How many bits are devoted to this value?

    USHORT   ReportCount;    // See Note below.  Usually set to 1.
    USHORT   Reserved2[5];

    ULONG    UnitsExp;
    ULONG    Units;

    LONG     LogicalMin,       LogicalMax;
    LONG     PhysicalMin,      PhysicalMax;

    union {
        struct {
            USAGE    UsageMin,         UsageMax;
            USHORT   StringMin,        StringMax;
            USHORT   DesignatorMin,    DesignatorMax;
            USHORT   DataIndexMin,     DataIndexMax;
        } Range;

        struct {
            USAGE    Usage,            Reserved1;
            USHORT   StringIndex,      Reserved2;
            USHORT   DesignatorIndex,  Reserved3;
            USHORT   DataIndex,        Reserved4;
        } NotRange;
    };
} HIDP_VALUE_CAPS, *PHIDP_VALUE_CAPS;

//
// Notes:
//
// ReportCount:  When a report descriptor declares an Input, Output, or
// Feature main item with fewer usage declarations than the report count, then
// the last usage applies to all remaining unspecified count in that main item.
// (As an example you might have data that required many fields to describe,
// possibly buffered bytes.)  In this case, only one value cap structure is
// allocated for these associtated fields, all with the same usage, and Report
// Count reflects the number of fields involved.  Normally ReportCount is 1.
// To access all of the fields in such a value structure would require using
// HidP_GetUsageValueArray and HidP_SetUsageValueArray.   HidP_GetUsageValue/
// HidP_SetScaledUsageValue will also work, however, these functions will only
// work with the first field of the structure.
//

//
// The link collection tree consists of an array of LINK_COLLECTION_NODES
// where the index into this array is the same as the collection number.
//
// Given a collection A which contains a subcollection B, A is defined to be
// the parent B, and B is defined to be the child.
//
// Given collections A, B, and C where B and C are children of A, and B was
// encountered before C in the report descriptor, B is defined as a sibling of
// C.  (This implies, of course, that if B is a sibling of C, then C is NOT a
// sibling of B).
//
// B is defined as the NextSibling of C if and only if there exists NO
// child collection of A, call it D, such that B is a sibling of D and D
// is a sibling of C.
//
// E is defined to be the FirstChild of A if and only if for all children of A,
// F, that are not equivalent to E, F is a sibling of E.
// (This implies, of course, that the does not exist a child of A, call it G,
// where E is a sibling of G).  In other words the first sibling is the last
// link collection found in the list.
//
// In other words, if a collection B is defined within the definition of another
// collection A, B becomes a child of A.  All collections with the same parent
// are considered siblings.  The FirstChild of the parent collection, A, will be
// last collection defined that has A as a parent.  The order of sibling pointers
// is similarly determined.  When a collection B is defined, it becomes the
// FirstChild of it's parent collection.  The previously defined FirstChild of the
// parent collection becomes the NextSibling of the new collection.  As new
// collections with the same parent are discovered, the chain of sibling is built.
//
// With that in mind, the following describes conclusively a data structure
// that provides direct traversal up, down, and accross the link collection
// tree.
//
//
typedef struct _HIDP_LINK_COLLECTION_NODE
{
    USAGE    LinkUsage;
    USAGE    LinkUsagePage;
    USHORT   Parent;
    USHORT   NumberOfChildren;
    USHORT   NextSibling;
    USHORT   FirstChild;
    ULONG    CollectionType: 8;  // As defined in 6.2.2.6 of HID spec
    ULONG    IsAlias : 1; // This link node is an allias of the next link node.
    ULONG    Reserved: 23;
    PVOID    UserContext; // The user can hang his coat here.
} HIDP_LINK_COLLECTION_NODE, *PHIDP_LINK_COLLECTION_NODE;

//
// When a link collection is described by a delimiter, alias link collection
// nodes are created.  (One for each usage within the delimiter).
// The parser assigns each capability description listed above only one
// link collection.
//
// If a control is defined within a collection defined by
// delimited usages, then that control is said to be within multiple link
// collections, one for each usage within the open and close delimiter tokens.
// Such multiple link collecions are said to be aliases.  The first N-1 such
// collections, listed in the link collection node array, have their IsAlias
// bit set.  The last such link collection is the link collection index used
// in the capabilities described above.
// Clients wishing to set a control in an aliased collection, should walk the
// collection array once for each time they see the IsAlias flag set, and use
// the last link collection as the index for the below accessor functions.
//
// NB: if IsAlias is set, then NextSibling should be one more than the current
// link collection node index.
//

typedef PUCHAR  PHIDP_REPORT_DESCRIPTOR;
typedef struct _HIDP_PREPARSED_DATA * PHIDP_PREPARSED_DATA;

typedef struct _HIDP_CAPS
{
    USAGE    Usage;
    USAGE    UsagePage;
    USHORT   InputReportByteLength;
    USHORT   OutputReportByteLength;
    USHORT   FeatureReportByteLength;
    USHORT   Reserved[17];

    USHORT   NumberLinkCollectionNodes;

    USHORT   NumberInputButtonCaps;
    USHORT   NumberInputValueCaps;
    USHORT   NumberInputDataIndices;

    USHORT   NumberOutputButtonCaps;
    USHORT   NumberOutputValueCaps;
    USHORT   NumberOutputDataIndices;

    USHORT   NumberFeatureButtonCaps;
    USHORT   NumberFeatureValueCaps;
    USHORT   NumberFeatureDataIndices;
} HIDP_CAPS, *PHIDP_CAPS;

typedef struct _HIDP_DATA
{
    USHORT  DataIndex;
    USHORT  Reserved;
    union {
        ULONG   RawValue; // for values
        BOOLEAN On; // for buttons MUST BE TRUE for buttons.
    };
} HIDP_DATA, *PHIDP_DATA;
//
// The HIDP_DATA structure is used with HidP_GetData and HidP_SetData
// functions.
//
// The parser contiguously assigns every control (button or value) in a hid
// device a unique data index from zero to NumberXXXDataIndices -1 , inclusive.
// This value is found in the HIDP_BUTTON_CAPS and HIDP_VALUE_CAPS structures.
//
// Most clients will find the Get/Set Buttons / Value accessor functions
// sufficient to their needs, as they will allow the clients to access the
// data known to them while ignoring the other controls.
//
// More complex clients, which actually read the Button / Value Caps, and which
// do a value add service to these routines (EG Direct Input), will need to
// access all the data in the device without interest in the individual usage
// or link collection location.  These are the clients that will find
// HidP_Data useful.
//

typedef struct _HIDP_UNKNOWN_TOKEN
{
    UCHAR  Token;
    UCHAR  Reserved[3];
    ULONG  BitField;
} HIDP_UNKNOWN_TOKEN, *PHIDP_UNKNOWN_TOKEN;

typedef struct _HIDP_EXTENDED_ATTRIBUTES
{
    UCHAR   NumGlobalUnknowns;
    UCHAR   Reserved [3];
    PHIDP_UNKNOWN_TOKEN  GlobalUnknowns;
    // ... Additional attributes
    ULONG   Data [1]; // variableLength  DO NOT ACCESS THIS FIELD
} HIDP_EXTENDED_ATTRIBUTES, *PHIDP_EXTENDED_ATTRIBUTES;


//
// Define NT Status codes with Facility Code of FACILITY_HID_ERROR_CODE
//

// FACILITY_HID_ERROR_CODE defined in ntstatus.h
#ifndef FACILITY_HID_ERROR_CODE
#define FACILITY_HID_ERROR_CODE 0x11
#endif

#define HIDP_ERROR_CODES(SEV, CODE) \
        ((NTSTATUS) (((SEV) << 28) | (FACILITY_HID_ERROR_CODE << 16) | (CODE)))

#define HIDP_STATUS_SUCCESS                  (HIDP_ERROR_CODES(0x0,0))
#define HIDP_STATUS_NULL                     (HIDP_ERROR_CODES(0x8,1))
#define HIDP_STATUS_INVALID_PREPARSED_DATA   (HIDP_ERROR_CODES(0xC,1))
#define HIDP_STATUS_INVALID_REPORT_TYPE      (HIDP_ERROR_CODES(0xC,2))
#define HIDP_STATUS_INVALID_REPORT_LENGTH    (HIDP_ERROR_CODES(0xC,3))
#define HIDP_STATUS_USAGE_NOT_FOUND          (HIDP_ERROR_CODES(0xC,4))
#define HIDP_STATUS_VALUE_OUT_OF_RANGE       (HIDP_ERROR_CODES(0xC,5))
#define HIDP_STATUS_BAD_LOG_PHY_VALUES       (HIDP_ERROR_CODES(0xC,6))
#define HIDP_STATUS_BUFFER_TOO_SMALL         (HIDP_ERROR_CODES(0xC,7))
#define HIDP_STATUS_INTERNAL_ERROR           (HIDP_ERROR_CODES(0xC,8))
#define HIDP_STATUS_I8042_TRANS_UNKNOWN      (HIDP_ERROR_CODES(0xC,9))
#define HIDP_STATUS_INCOMPATIBLE_REPORT_ID   (HIDP_ERROR_CODES(0xC,0xA))
#define HIDP_STATUS_NOT_VALUE_ARRAY          (HIDP_ERROR_CODES(0xC,0xB))
#define HIDP_STATUS_IS_VALUE_ARRAY           (HIDP_ERROR_CODES(0xC,0xC))
#define HIDP_STATUS_DATA_INDEX_NOT_FOUND     (HIDP_ERROR_CODES(0xC,0xD))
#define HIDP_STATUS_DATA_INDEX_OUT_OF_RANGE  (HIDP_ERROR_CODES(0xC,0xE))
#define HIDP_STATUS_BUTTON_NOT_PRESSED       (HIDP_ERROR_CODES(0xC,0xF))
#define HIDP_STATUS_REPORT_DOES_NOT_EXIST    (HIDP_ERROR_CODES(0xC,0x10))
#define HIDP_STATUS_NOT_IMPLEMENTED          (HIDP_ERROR_CODES(0xC,0x20))

//
// We blundered this status code.
//
#define HIDP_STATUS_I8242_TRANS_UNKNOWN HIDP_STATUS_I8042_TRANS_UNKNOWN

#include <poppack.h>

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4115)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif

#endif

