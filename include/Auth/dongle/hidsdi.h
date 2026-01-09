/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    HIDSDI.H

Abstract:

    This module contains the PUBLIC definitions for the
    code that implements the HID dll.

Environment:

    Kernel & user mode

--*/


#ifndef _HIDSDI_H
#define _HIDSDI_H

#include <pshpack4.h>

//#include "wtypes.h"

//#include <windef.h>
//#include <win32.h>
//#include <basetyps.h>

typedef LONG NTSTATUS;
#include "hidusage.h"
#include "hidpi.h"

typedef struct _HIDD_CONFIGURATION {
    PVOID    cookie;
    ULONG    size;
    ULONG    RingBufferSize;
} HIDD_CONFIGURATION, *PHIDD_CONFIGURATION;

typedef struct _HIDD_ATTRIBUTES {
    ULONG   Size; // = sizeof (struct _HIDD_ATTRIBUTES)

    //
    // Vendor ids of this hid device
    //
    USHORT  VendorID;
    USHORT  ProductID;
    USHORT  VersionNumber;

    //
    // Additional fields will be added to the end of this structure.
    //
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;




BOOLEAN __stdcall
HidD_GetPreparsedData (
   HANDLE                  HidDeviceObject,
   PHIDP_PREPARSED_DATA  * PreparsedData
   );
/*++
Routine Description:
    Given a handle to a valid Hid Class Device Object, retrieve the preparsed
    data for the device.  This routine will allocate the appropriately 
    sized buffer to hold this preparsed data.  It is up to client to call
    HidP_FreePreparsedData to free the memory allocated to this structure when
    it is no longer needed.

Arguments:
   HidDeviceObject A handle to a Hid Device that the client obtains using 
                   a call to CreateFile on a valid Hid device string name.
                   The string name can be obtained using standard PnP calls.

   PreparsedData   An opaque data structure used by other functions in this 
                   library to retrieve information about a given device.

Return Value:
   TRUE if successful.
   FALSE otherwise  -- Use GetLastError() to get extended error information
--*/
BOOLEAN __stdcall
HidD_FreePreparsedData (
   PHIDP_PREPARSED_DATA PreparsedData
   );



BOOLEAN __stdcall
HidD_GetFeature (
   HANDLE   HidDeviceObject,
   PVOID ReportBuffer,
    ULONG    ReportBufferLength
   );
/*++
Routine Description:
    Retrieve a feature report from a HID device.

Arguments:
    HidDeviceObject      A handle to a Hid Device Object.
 
    ReportBuffer         The buffer that the feature report should be placed 
                         into.  The first byte of the buffer should be set to
                         the report ID of the desired report
 
    ReportBufferLength   The size (in bytes) of ReportBuffer.  This value 
                         should be greater than or equal to the 
                         FeatureReportByteLength field as specified in the 
                         HIDP_CAPS structure for the device
Return Value:
    TRUE if successful
    FALSE otherwise  -- Use GetLastError() to get extended error information
--*/
BOOLEAN __stdcall
HidD_SetFeature (
   HANDLE   HidDeviceObject,
  PVOID ReportBuffer,
   ULONG    ReportBufferLength
   );
/*++
Routine Description:
    Send a feature report to a HID device.

Arguments:
    HidDeviceObject      A handle to a Hid Device Object.
 
    ReportBuffer         The buffer of the feature report to send to the device
 
    ReportBufferLength   The size (in bytes) of ReportBuffer.  This value 
                         should be greater than or equal to the 
                         FeatureReportByteLength field as specified in the 
                         HIDP_CAPS structure for the device
Return Value:
    TRUE if successful
    FALSE otherwise  -- Use GetLastError() to get extended error information
--*/





#include <poppack.h>

#endif

