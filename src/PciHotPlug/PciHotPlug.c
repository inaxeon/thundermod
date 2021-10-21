/**
 * File: PciHotPLug.c
 * Author: Matthew Millman
 *
 * Work in progress
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.
 * 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "PciHotPlug.h"

#define PADDING_BUS (1)
#define PADDING_NONPREFETCH_MEM (1)
#define PADDING_PREFETCH_MEM (1)
#define PADDING_IO (1)
#define PADDING_NUM (PADDING_BUS + PADDING_NONPREFETCH_MEM + PADDING_PREFETCH_MEM + PADDING_IO)

GLOBAL_REMOVE_IF_UNREFERENCED EFI_HPC_LOCATION mPcieLocation[1];

#define HPC_COUNT 1

// AIC Root port
#define RP_BUS      0x00
#define RP_DEV      0x1C
#define RP_FUNC     0x04

// Thunderbolt port 1
#define HP1_BUS      0x05
#define HP1_DEV      0x01
#define HP1_FUNC     0x00

// Thunderbolt port 2
#define HP2_BUS      0x05
#define HP2_DEV      0x04
#define HP2_FUNC     0x00


typedef struct _EFI_GLOBAL_NVS_AREA_PROTOCOL
{
  VOID *Area;
} EFI_GLOBAL_NVS_AREA_PROTOCOL;


EFI_GUID gEfiGlobalNvsAreaProtocolGuid = { 0x74e1e48, 0x8132, 0x47a1, { 0x8c, 0x2c, 0x3f, 0x14, 0xad, 0x9a, 0x66, 0xdc }};

GLOBAL_REMOVE_IF_UNREFERENCED PCIE_HOT_PLUG_DEVICE_PATH mHotplugPcieDevicePathTemplate = {
    ACPI,
    PCI(0xFF, 0xFF), // Dummy Device no & Function no
    END};

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  UINTN RpDev;
  UINTN RpFunc;
  PCI_HOT_PLUG_INSTANCE *PciHotPlug;
  PCIE_HOT_PLUG_DEVICE_PATH *HotplugPcieDevicePath;

  HotplugPcieDevicePath = NULL;
  HotplugPcieDevicePath = AllocatePool(sizeof(PCIE_HOT_PLUG_DEVICE_PATH));
  ASSERT(HotplugPcieDevicePath != NULL);
  if (HotplugPcieDevicePath == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }

  RpDev = RP_DEV;
  RpFunc = RP_FUNC;

  CopyMem(HotplugPcieDevicePath, &mHotplugPcieDevicePathTemplate, sizeof(PCIE_HOT_PLUG_DEVICE_PATH));
  HotplugPcieDevicePath->PciRootPortNode.Device = (UINT8)RpDev;    // Update real Device no
  HotplugPcieDevicePath->PciRootPortNode.Function = (UINT8)RpFunc; // Update real Function no

  mPcieLocation[0].HpcDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)HotplugPcieDevicePath;
  mPcieLocation[0].HpbDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)HotplugPcieDevicePath;

  DEBUG((DEBUG_INFO, "PciHotPlug (PCH RP#) : Bus 0x00, Device 0x%x, Function 0x%x is added to the Hotplug Device Path list \n", RpDev, RpFunc));

  PciHotPlug = AllocatePool(sizeof(PCI_HOT_PLUG_INSTANCE));
  ASSERT(PciHotPlug != NULL);
  if (PciHotPlug == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize driver private data.
  //
  ZeroMem(PciHotPlug, sizeof(PCI_HOT_PLUG_INSTANCE));

  PciHotPlug->Signature = PCI_HOT_PLUG_DRIVER_PRIVATE_SIGNATURE;
  PciHotPlug->HotPlugInitProtocol.GetRootHpcList = GetRootHpcList;
  PciHotPlug->HotPlugInitProtocol.InitializeRootHpc = InitializeRootHpc;
  PciHotPlug->HotPlugInitProtocol.GetResourcePadding = GetResourcePadding;

  Status = gBS->InstallProtocolInterface(
      &PciHotPlug->Handle,
      &gEfiPciHotPlugInitProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &PciHotPlug->HotPlugInitProtocol);
  ASSERT_EFI_ERROR(Status);

  return Status;
}

/**
  This procedure returns a list of Root Hot Plug controllers that require
  initialization during boot process
  @param[in]  This      The pointer to the instance of the EFI_PCI_HOT_PLUG_INIT protocol.
  @param[out] HpcCount  The number of Root HPCs returned.
  @param[out] HpcList   The list of Root HPCs. HpcCount defines the number of elements in this list.
  @retval EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
GetRootHpcList(
    IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL *This,
    OUT UINTN *HpcCount,
    OUT EFI_HPC_LOCATION **HpcList)
{
  EFI_STATUS Status;
  DEBUG((DEBUG_ERROR, "GetRootHpcList()\n"));
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

  *HpcCount = HPC_COUNT;
  *HpcList = mPcieLocation;

  // Disgraceful NVS hack. TODO: Figure out how to do this properly.
  // gEfiGlobalNvsAreaProtocolGuid should be depex'd but can't be bothered at present.
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **)&GlobalNvsArea
                  );

  ASSERT_EFI_ERROR(Status);

  // Let _OSC() tell the O/S hotplug is useable
  *(UINT8 *)(GlobalNvsArea->Area + 0x71) = 0x01;

  DEBUG((DEBUG_INFO, "GlobalNvsArea: %p 0x71: 0x%02X\n", GlobalNvsArea->Area, *(UINT8 *)(GlobalNvsArea->Area + 0x71)));

  return EFI_SUCCESS;
}

/**
  This procedure Initializes one Root Hot Plug Controller
  This process may casue initialization of its subordinate buses
  @param[in]  This            The pointer to the instance of the EFI_PCI_HOT_PLUG_INIT protocol.
  @param[in]  HpcDevicePath   The Device Path to the HPC that is being initialized.
  @param[in]  HpcPciAddress   The address of the Hot Plug Controller function on the PCI bus.
  @param[in]  Event           The event that should be signaled when the Hot Plug Controller initialization is complete. Set to NULL if the caller wants to wait until the entire initialization process is complete. The event must be of the type EFI_EVT_SIGNAL.
  @param[out] HpcState        The state of the Hot Plug Controller hardware. The type EFI_Hpc_STATE is defined in section 3.1.
  @retval   EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
InitializeRootHpc(
    IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL *This,
    IN EFI_DEVICE_PATH_PROTOCOL *HpcDevicePath,
    IN UINT64 HpcPciAddress,
    IN EFI_EVENT Event, OPTIONAL OUT EFI_HPC_STATE *HpcState)
{
  if (Event)
  {
    gBS->SignalEvent(Event);
  }

  *HpcState = EFI_HPC_STATE_INITIALIZED;

  DEBUG((DEBUG_ERROR, "InitializeRootHpc()\n"));

  return EFI_SUCCESS;
}

/**
  Returns the resource padding required by the PCI bus that is controlled by the specified Hot Plug Controller.
  @param[in]  This           The pointer to the instance of the EFI_PCI_HOT_PLUG_INIT protocol. initialized.
  @param[in]  HpcDevicePath  The Device Path to the Hot Plug Controller.
  @param[in]  HpcPciAddress  The address of the Hot Plug Controller function on the PCI bus.
  @param[out] HpcState       The state of the Hot Plug Controller hardware. The type EFI_HPC_STATE is defined in section 3.1.
  @param[out] Padding        This is the amount of resource padding required by the PCI bus under the control of the specified Hpc. Since the caller does not know the size of this buffer, this buffer is allocated by the callee and freed by the caller.
  @param[out] Attribute      Describes how padding is accounted for.
  @retval     EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
GetResourcePadding(
    IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL *This,
    IN EFI_DEVICE_PATH_PROTOCOL *HpcDevicePath,
    IN UINT64 HpcPciAddress,
    OUT EFI_HPC_STATE *HpcState,
    OUT VOID **Padding,
    OUT EFI_HPC_PADDING_ATTRIBUTES *Attributes)
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *PaddingResource;
  UINT8 RsvdExtraBusNum = 0;

  UINTN RpBus;
  UINTN RpDev;
  UINTN RpFunc;

  DEBUG((DEBUG_INFO, "GetResourcePadding() : Start\n"));

  RpBus = (UINTN)((HpcPciAddress >> 24) & 0xFF);
  RpDev = (UINTN)((HpcPciAddress >> 16) & 0xFF);
  RpFunc = (UINTN)((HpcPciAddress >> 8) & 0xFF);

  DEBUG((DEBUG_INFO, "GetResourcePadding : Rootport Bus 0x%x, Device 0x%x, Function 0x%x \n", RpBus, RpDev, RpFunc));

  PaddingResource = AllocatePool((PADDING_NUM * sizeof(EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR)) + sizeof(EFI_ACPI_END_TAG_DESCRIPTOR));
  ASSERT(PaddingResource != NULL);
  if (PaddingResource == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }

  *Padding = (VOID *)PaddingResource;

  //
  // Padding for bus
  //
  ZeroMem(PaddingResource, (PADDING_NUM * sizeof(EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR)) + sizeof(EFI_ACPI_END_TAG_DESCRIPTOR));
  *Attributes = EfiPaddingPciBus;

  if ((RpBus == RP_BUS && RpDev == RP_DEV && RpFunc == RP_FUNC))
  {
    UINT16 RsvdPcieMegaMem = 10;
    UINT8 PcieMemAddrRngMax = 0;
    UINT16 RsvdPciePMegaMem = 10;
    UINT8 PciePMemAddrRngMax = 0;
    UINT8 RsvdPcieKiloIo = 4;

    DEBUG((DEBUG_INFO, "GetResourcePadding : Padding for root bridge\n"));


    RsvdPcieMegaMem = 512;
    // PcieMemAddrRngMax = TbtPcieMemAddrRngMax;
    RsvdPciePMegaMem = 512;
    // PciePMemAddrRngMax = TbtPciePMemAddrRngMax;

    //
    // Padding for non-prefetchable memory
    //
    PaddingResource->Desc = 0x8A;
    PaddingResource->Len = 0x2B;
    PaddingResource->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
    PaddingResource->GenFlag = 0x0;
    PaddingResource->AddrSpaceGranularity = 32;
    PaddingResource->SpecificFlag = 0;
    //
    // Pad non-prefetchable
    //
    PaddingResource->AddrRangeMin = 0;
    PaddingResource->AddrLen = RsvdPcieMegaMem * 0x100000;
    PaddingResource->AddrRangeMax = (1 << PcieMemAddrRngMax) - 1;

    //
    // Padding for prefetchable memory
    //
    PaddingResource++;
    PaddingResource->Desc = 0x8A;
    PaddingResource->Len = 0x2B;
    PaddingResource->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
    PaddingResource->GenFlag = 0x0;
    PaddingResource->AddrSpaceGranularity = 32;
    //
    // Padding for prefetchable memory
    //
    PaddingResource->AddrRangeMin = 0;
    PaddingResource->AddrLen = RsvdPciePMegaMem * 0x100000;
    //
    // Pad 16 MB of MEM
    //
    PaddingResource->AddrRangeMax = (1 << PciePMemAddrRngMax) - 1;
    //
    // Alignment
    //
    // Padding for I/O
    //
    PaddingResource++;
    PaddingResource->Desc = 0x8A;
    PaddingResource->Len = 0x2B;
    PaddingResource->ResType = ACPI_ADDRESS_SPACE_TYPE_IO;
    PaddingResource->GenFlag = 0x0;
    PaddingResource->SpecificFlag = 0;
    PaddingResource->AddrRangeMin = 0;
    PaddingResource->AddrLen = RsvdPcieKiloIo * 0x400;
    //
    // Pad 4K of IO
    //
    PaddingResource->AddrRangeMax = 1;
    //
    // Alignment
    //
    PaddingResource++;

    *HpcState = EFI_HPC_STATE_INITIALIZED | EFI_HPC_STATE_ENABLED;
  }
  else if ((RpBus == HP1_BUS && RpDev == HP1_DEV && RpFunc == HP1_FUNC) || (RpBus == HP2_BUS && RpDev == HP2_DEV && RpFunc == HP2_FUNC))
  {
    DEBUG((DEBUG_INFO, "GetResourcePadding : Padding for downstream bridge\n"));

    RsvdExtraBusNum = 20;

    PaddingResource->Desc = 0x8A;
    PaddingResource->Len = 0x2B;
    PaddingResource->ResType = ACPI_ADDRESS_SPACE_TYPE_BUS;
    PaddingResource->GenFlag = 0x0;
    PaddingResource->SpecificFlag = 0;
    PaddingResource->AddrRangeMin = 0;
    PaddingResource->AddrRangeMax = 0;
    PaddingResource->AddrLen = RsvdExtraBusNum;

    PaddingResource++;

    *HpcState = EFI_HPC_STATE_INITIALIZED | EFI_HPC_STATE_ENABLED;
  }

  // Terminate the entries.
  //
  ((EFI_ACPI_END_TAG_DESCRIPTOR *)PaddingResource)->Desc = ACPI_END_TAG_DESCRIPTOR;
  ((EFI_ACPI_END_TAG_DESCRIPTOR *)PaddingResource)->Checksum = 0x0;

  return EFI_SUCCESS;
}