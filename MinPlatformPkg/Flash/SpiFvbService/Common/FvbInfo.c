/**@file
  Defines data structure that is the volume header found.
  These data is intent to decouple FVB driver with FV header.

Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "SpiFvbServiceCommon.h"

#define FIRMWARE_BLOCK_SIZE         0x10000
#define FVB_MEDIA_BLOCK_SIZE        FIRMWARE_BLOCK_SIZE

#define NV_STORAGE_BASE_ADDRESS     PcdGet32(PcdFlashNvStorageVariableBase)
#define SYSTEM_NV_BLOCK_NUM         ((PcdGet32(PcdFlashNvStorageVariableSize)+ PcdGet32(PcdFlashNvStorageFtwWorkingSize) + PcdGet32(PcdFlashNvStorageFtwSpareSize))/ FVB_MEDIA_BLOCK_SIZE)

typedef struct {
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  EFI_FIRMWARE_VOLUME_HEADER  FvbInfo;
  EFI_FV_BLOCK_MAP_ENTRY      End[1];
} EFI_FVB2_MEDIA_INFO;

//
// This data structure contains a template of all correct FV headers, which is used to restore
// Fv header if it's corrupted.
//
EFI_FVB2_MEDIA_INFO mPlatformFvbMediaInfo[] = {
  //
  // Systen NvStorage FVB
  //
  {
    0, //NV_STORAGE_BASE_ADDRESS,
    {
      {0,}, //ZeroVector[16]
      EFI_SYSTEM_NV_DATA_FV_GUID,
      0, //FVB_MEDIA_BLOCK_SIZE * SYSTEM_NV_BLOCK_NUM,
      EFI_FVH_SIGNATURE,
      0x0004feff, // check MdePkg/Include/Pi/PiFirmwareVolume.h for details on EFI_FVB_ATTRIBUTES_2
      sizeof (EFI_FIRMWARE_VOLUME_HEADER) + sizeof (EFI_FV_BLOCK_MAP_ENTRY),
      0,    //CheckSum which will be calucated dynamically.
      0,    //ExtHeaderOffset
      {0,}, //Reserved[1]
      2,    //Revision
      {
        {
          0, //SYSTEM_NV_BLOCK_NUM,
          FVB_MEDIA_BLOCK_SIZE,
        }
      }
    },
    {
      {
        0,
        0
      }
    }
  }
};

EFI_STATUS
GetFvbInfo (
  IN  EFI_PHYSICAL_ADDRESS         FvBaseAddress,
  OUT EFI_FIRMWARE_VOLUME_HEADER   **FvbInfo
  )
{
  UINTN                       Index;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;

  //Need to update to use dynamic PCD for FW volume locations.
  mPlatformFvbMediaInfo[0].BaseAddress = NV_STORAGE_BASE_ADDRESS;
  mPlatformFvbMediaInfo[0].FvbInfo.FvLength = FVB_MEDIA_BLOCK_SIZE * SYSTEM_NV_BLOCK_NUM;
  mPlatformFvbMediaInfo[0].FvbInfo.BlockMap[0].NumBlocks =SYSTEM_NV_BLOCK_NUM;

  for (Index = 0; Index < sizeof (mPlatformFvbMediaInfo) / sizeof (EFI_FVB2_MEDIA_INFO); Index++) {
    if (mPlatformFvbMediaInfo[Index].BaseAddress == FvBaseAddress) {
      FvHeader = &mPlatformFvbMediaInfo[Index].FvbInfo;

      //
      // Update the checksum value of FV header.
      //
      FvHeader->Checksum = CalculateCheckSum16 ( (UINT16 *) FvHeader, FvHeader->HeaderLength);

      *FvbInfo = FvHeader;

      DEBUG ((DEBUG_INFO, "BaseAddr: 0x%lx \n", FvBaseAddress));
      DEBUG ((DEBUG_INFO, "FvLength: 0x%lx \n", (*FvbInfo)->FvLength));
      DEBUG ((DEBUG_INFO, "HeaderLength: 0x%x \n", (*FvbInfo)->HeaderLength));
      DEBUG ((DEBUG_INFO, "Header Checksum: 0x%X\n", (*FvbInfo)->Checksum));
      DEBUG ((DEBUG_INFO, "FvBlockMap[0].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[0].NumBlocks));
      DEBUG ((DEBUG_INFO, "FvBlockMap[0].BlockLength: 0x%x \n", (*FvbInfo)->BlockMap[0].Length));
      DEBUG ((DEBUG_INFO, "FvBlockMap[1].NumBlocks: 0x%x \n", (*FvbInfo)->BlockMap[1].NumBlocks));
      DEBUG ((DEBUG_INFO, "FvBlockMap[1].BlockLength: 0x%x \n\n", (*FvbInfo)->BlockMap[1].Length));

      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}
