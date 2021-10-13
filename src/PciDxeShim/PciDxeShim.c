/**
 * File: PciDxeShim.c
 * Author: Matthew Millman
 *
 * "PciBus" -> "PciHostBridge" Shim Driver.
 * 
 * This driver is intended to observe calls between the aforementioned drivers for debugging / reverse
 * engineering purposes.
 *
 * For this shim to work, the following three hex sequences in the original "PciBus" DXE must be substituted:
 *
 * Replace : AB 31 A0 18 43 B4 1A 4D A5 C0 0C 09 26 1E 9F 71 (EFI_DRIVER_BINDING_PROTOCOL_GUID)
 * With    : 38 45 AD E8 8D 0A E6 46 8A 1F 09 03 B7 9A 91 BB (Substitute protcol consumed by this driver)
 * 
 * Replace : BB 7E 70 2F 1A 4A D4 11 9A 38 00 90 27 3F C1 4D (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID)
 * With    : D2 27 E6 F9 2F 48 E9 49 A1 65 F0 22 C9 6A F1 84 (Substitute protcol consumed by this driver)
 * 
 * Replace : BE 34 80 CF 68 67 8b 4d B7 39 7C CE 68 3A 9F BE (EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL_GUID)
 * With    : 0E 7E F3 35 B1 3E 3A 45 A5 AD 4B 4C 15 A6 3C 18 (Substitute protcol consumed by this driver)
 * 
 * Some function headers taken from EDK2
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

#include "PciDxeShim.h"

EFI_GUID gDriverBindingProtocolSubstituteGuid = {0xE8AD4538, 0x0A8D, 0x46E6, {0x8A, 0x1F, 0x09, 0x03, 0xB7, 0x9A, 0x91, 0xBB}};
EFI_GUID gPciRootBridgeIoProtocolGuidSubstituteGuid = {0xF9E627D2, 0x482F, 0x49E9, {0xA1, 0x65, 0xF0, 0x22, 0xC9, 0x6A, 0xF1, 0x84}};
EFI_GUID gEfiPciHostBrgResAllocProtocolSubstituteGuid = {0x35F37E0E, 0x3EB1, 0x453A, {0xA5, 0xAD, 0x4B, 0x4C, 0x15, 0xA6, 0x3C, 0x18}};

EFI_DRIVER_BINDING_PROTOCOL *gDriverBindingSubstituteProtocol = NULL;

EFI_DRIVER_BINDING_PROTOCOL gPciBusDriverBinding = {
    PciBusDriverBindingSupported,
    PciBusDriverBindingStart,
    PciBusDriverBindingStop,
    0xa,
    NULL,
    NULL};

LIST_ENTRY RootBridgeIoProtocolList;

EFI_STATUS EFIAPI PciDxeShimMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  SerialPortInitialize();

  DEBUG((DEBUG_INFO, "PciDxeShim: Starting up\n"));

  InitializeListHead(&RootBridgeIoProtocolList);

  // Pretend we're the PciBus driver, so we can hook the entry points

  Status = EfiLibInstallDriverBindingComponentName2(
      ImageHandle,
      SystemTable,
      &gPciBusDriverBinding,
      ImageHandle,
      &gPciBusComponentName,
      &gPciBusComponentName2);

  ASSERT_EFI_ERROR(Status);

  DEBUG((DEBUG_INFO, "PciDxeShim: Startup complete\n"));

  return EFI_SUCCESS;
}

/**
  Create root bridge I/O protocol mapping structure, and install protocol.

  @param  This                          Protocol instance pointer.
  @param  Controller                    Handle of device to test.
  @param  rootBridgeIoProtocolMapping   Pointer to a pointer to store a root bridge I/O protocol mapping

  @retval EFI_SUCCESS         Protocol successfully installed
  @retval other               Something went wrong.

**/
EFI_STATUS InstallRootBridgeIoProtocolProtocol(EFI_DRIVER_BINDING_PROTOCOL *This, EFI_HANDLE Controller, RootBridgeIoProtocolMapping **rootBridgeIoProtocolMapping)
{
  EFI_STATUS Status;

  DEBUG((DEBUG_INFO, "InstallRootBridgeIoProtocolProtocol()\n"));

  RootBridgeIoProtocolMapping *newIoMapping = AllocateZeroPool(sizeof(RootBridgeIoProtocolMapping));

  newIoMapping->BindingProtocol = This;
  newIoMapping->Controller = Controller;
  newIoMapping->IsOpen = FALSE;

  newIoMapping->SubstitutedProtocol.PollMem = RootBridgeIoPollMem;
  newIoMapping->SubstitutedProtocol.PollIo = RootBridgeIoPollIo;
  newIoMapping->SubstitutedProtocol.Mem.Read = RootBridgeIoMemRead;
  newIoMapping->SubstitutedProtocol.Mem.Write = RootBridgeIoMemWrite;
  newIoMapping->SubstitutedProtocol.Io.Read = RootBridgeIoIoRead;
  newIoMapping->SubstitutedProtocol.Io.Write = RootBridgeIoIoWrite;
  newIoMapping->SubstitutedProtocol.CopyMem = RootBridgeIoCopyMem;
  newIoMapping->SubstitutedProtocol.Pci.Read = RootBridgeIoPciRead;
  newIoMapping->SubstitutedProtocol.Pci.Write = RootBridgeIoPciWrite;
  newIoMapping->SubstitutedProtocol.Map = RootBridgeIoMap;
  newIoMapping->SubstitutedProtocol.Unmap = RootBridgeIoUnmap;
  newIoMapping->SubstitutedProtocol.AllocateBuffer = RootBridgeIoAllocateBuffer;
  newIoMapping->SubstitutedProtocol.FreeBuffer = RootBridgeIoFreeBuffer;
  newIoMapping->SubstitutedProtocol.Flush = RootBridgeIoFlush;
  newIoMapping->SubstitutedProtocol.GetAttributes = RootBridgeIoGetAttributes;
  newIoMapping->SubstitutedProtocol.SetAttributes = RootBridgeIoSetAttributes;
  newIoMapping->SubstitutedProtocol.Configuration = RootBridgeIoConfiguration;

  Status = gBS->InstallMultipleProtocolInterfaces(
      &Controller,
      &gPciRootBridgeIoProtocolGuidSubstituteGuid, &newIoMapping->SubstitutedProtocol,
      NULL);

  ASSERT_EFI_ERROR(Status);

  newIoMapping->HostBridgeHandle = Controller;
  *rootBridgeIoProtocolMapping = newIoMapping;

  return Status;
}

/**
  Uninstall the substitute root bridge I/O protocol and free the mapping structure.

  @param  Handle                            Protocol instance pointer.
  @param  rootBridgeIoProtocolMapping       Handle of device to test.

  @retval EFI_SUCCESS         Protocol successfully uninstalled
  @retval other               Something went wrong.

**/
EFI_STATUS UninstallRootBridgeIoProtocolProtocol(EFI_HANDLE Controller, RootBridgeIoProtocolMapping *rootBridgeIoProtocolMapping)
{
  EFI_STATUS Status;

  DEBUG((DEBUG_INFO, "UninstallRootBridgeIoProtocolProtocol()\n"));

  Status = gBS->UninstallMultipleProtocolInterfaces(Controller, &gPciRootBridgeIoProtocolGuidSubstituteGuid, &rootBridgeIoProtocolMapping->SubstitutedProtocol, NULL);

  ASSERT_EFI_ERROR(Status);

  if (Status == EFI_SUCCESS)
    FreePool(rootBridgeIoProtocolMapping);

  return Status;
}

/**
  Create resource allocation protocol mapping structure, and install protocol.

  @param  newIoMapping                        Pointer to a newly created root bridge IO protocol mapping, which the resources mapping requires
  @param  resourceAllocationProtocolMapping   Pointer to a pointer to store a resources protocol mapping

  @retval EFI_SUCCESS         Protocol successfully installed
  @retval other               Something went wrong.

**/
EFI_STATUS InstallResourceAllocationProtocol(RootBridgeIoProtocolMapping *newIoMapping, ResourceAllocationProtocolMapping **resourceAllocationProtocolMapping)
{
  EFI_STATUS Status;
  ResourceAllocationProtocolMapping *newResourceAllocationMapping;

  DEBUG((DEBUG_INFO, "InstallResourceAllocationProtocol()\n"));

  newResourceAllocationMapping = AllocateZeroPool(sizeof(ResourceAllocationProtocolMapping));

  newResourceAllocationMapping->SubstitutedProtocol.NotifyPhase = NotifyPhase;
  newResourceAllocationMapping->SubstitutedProtocol.GetNextRootBridge = GetNextRootBridge;
  newResourceAllocationMapping->SubstitutedProtocol.GetAllocAttributes = GetAttributes;
  newResourceAllocationMapping->SubstitutedProtocol.StartBusEnumeration = StartBusEnumeration;
  newResourceAllocationMapping->SubstitutedProtocol.SetBusNumbers = SetBusNumbers;
  newResourceAllocationMapping->SubstitutedProtocol.SubmitResources = SubmitResources;
  newResourceAllocationMapping->SubstitutedProtocol.GetProposedResources = GetProposedResources;
  newResourceAllocationMapping->SubstitutedProtocol.PreprocessController = PreprocessController;

  Status = gBS->InstallMultipleProtocolInterfaces(
      &newIoMapping->SubstitutedProtocol.ParentHandle,
      &gEfiPciHostBrgResAllocProtocolSubstituteGuid, &newResourceAllocationMapping->SubstitutedProtocol,
      NULL);

  ASSERT_EFI_ERROR(Status);

  newIoMapping->Parent = newResourceAllocationMapping;
  *resourceAllocationProtocolMapping = newResourceAllocationMapping;

  return Status;
}

/**
  Uninstall the substitute resource allocation protocol
  and free the mapping structure

  @param  Handle                            Protocol instance pointer.
  @param  resourceAllocationProtocolMapping Handle of device to test.

  @retval EFI_SUCCESS         Protocol successfully uninstalled
  @retval other               Something went wrong.

**/
EFI_STATUS UninstallResourceAllocationProtocol(EFI_HANDLE Handle, ResourceAllocationProtocolMapping *resourceAllocationProtocolMapping)
{
  EFI_STATUS Status;

  DEBUG((DEBUG_INFO, "UninstallResourceAllocationProtocol()\n"));

  Status = gBS->UninstallMultipleProtocolInterfaces(Handle, &gEfiPciHostBrgResAllocProtocolSubstituteGuid, &resourceAllocationProtocolMapping->SubstitutedProtocol, NULL);

  ASSERT_EFI_ERROR(Status);

  if (Status == EFI_SUCCESS)
    FreePool(resourceAllocationProtocolMapping);

  return Status;
}

/**
  Open protocols to the PciHostBridge driver

  @retval EFI_SUCCESS         Protocols successfully opened
  @retval other               Something went wrong.

**/
EFI_STATUS OpenBaseProtocols()
{
  EFI_STATUS Status;
  LIST_ENTRY *mappingEntry;

  DEBUG((DEBUG_INFO, "OpenBaseProtocols()\n"));

  for (mappingEntry = GetFirstNode(&RootBridgeIoProtocolList); !IsNull(&RootBridgeIoProtocolList, mappingEntry); mappingEntry = GetNextNode(&RootBridgeIoProtocolList, mappingEntry))
  {
    RootBridgeIoProtocolMapping *mapping = (RootBridgeIoProtocolMapping *)mappingEntry;

    if (mapping->IsOpen)
      continue;

    Status = gBS->OpenProtocol(
        mapping->Controller,
        &gEfiPciRootBridgeIoProtocolGuid,
        (VOID **)&mapping->OriginalProtocol,
        mapping->BindingProtocol->DriverBindingHandle,
        mapping->Controller,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (Status == EFI_SUCCESS)
    {
      mapping->IsOpen = TRUE;
      mapping->SubstitutedProtocol.SegmentNumber = mapping->OriginalProtocol->SegmentNumber;
    }
    else
    {
      ASSERT_EFI_ERROR(Status);
      return Status;
    }

    Status = gBS->OpenProtocol(
        mapping->OriginalProtocol->ParentHandle,
        &gEfiPciHostBridgeResourceAllocationProtocolGuid,
        (VOID **)&mapping->Parent->OriginalProtocol,
        mapping->BindingProtocol->DriverBindingHandle,
        mapping->Controller,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (Status != EFI_SUCCESS)
    {
      ASSERT_EFI_ERROR(Status);
      return Status;
    }
  }

  return Status;
}

/**
  Close protocols and free resources used by this driver

  @param  This                Protocol instance pointer.
  @param  Controller          Handle of device to test.

  @retval EFI_SUCCESS         Resources successfully free'd
  @retval other               Something went wrong.

**/
EFI_STATUS CleanupDriver(EFI_DRIVER_BINDING_PROTOCOL *This, EFI_HANDLE Controller)
{
  EFI_STATUS Status;
  LIST_ENTRY *mappingEntry;

  for (mappingEntry = GetFirstNode(&RootBridgeIoProtocolList); !IsNull(&RootBridgeIoProtocolList, mappingEntry); mappingEntry = GetNextNode(&RootBridgeIoProtocolList, mappingEntry))
  {
    RootBridgeIoProtocolMapping *mapping = (RootBridgeIoProtocolMapping *)mappingEntry;

    Status = gBS->CloseProtocol(
        Controller,
        &gEfiPciRootBridgeIoProtocolGuid,
        This->DriverBindingHandle,
        Controller);

    if (Status != EFI_SUCCESS)
    {
      ASSERT_EFI_ERROR(Status);
      return Status;
    }

    Status = gBS->CloseProtocol(
        mapping->OriginalProtocol->ParentHandle,
        &gEfiPciHostBridgeResourceAllocationProtocolGuid,
        This->DriverBindingHandle,
        Controller);

    if (Status != EFI_SUCCESS)
    {
      ASSERT_EFI_ERROR(Status);
      return Status;
    }

    Status = UninstallResourceAllocationProtocol(mapping->SubstitutedProtocol.ParentHandle, mapping->Parent);

    if (Status != EFI_SUCCESS)
      return Status;

    Status = UninstallRootBridgeIoProtocolProtocol(Controller, mapping);

    if (Status != EFI_SUCCESS)
      return Status;
  }

  InitializeListHead(&RootBridgeIoProtocolList);

  return Status;
}

/**
  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  than contains a gEfiPciRootBridgeIoProtocolGuid protocol can be supported.

  This function also has to install the substitute protocols at a specific
  point during initialisation to successfully hook the protocols between
  the two drivers.

  @param  This                Protocol instance pointer.
  @param  Controller          Handle of device to test.
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval EFI_ALREADY_STARTED This driver is already running on this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
PciBusDriverBindingSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE Controller,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath)
{
  EFI_STATUS Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
  RootBridgeIoProtocolMapping *newIoMapping;

  if (FindDriverBindingProtocol() == NULL)
    return EFI_NOT_FOUND;

  //DEBUG((DEBUG_INFO, "PciBusDriverBindingSupported(): This: %p Controller: %p\n", This, Controller));

  // Perform the same protocol availability test as the base driver.
  Status = gBS->OpenProtocol(
      Controller,
      &gEfiPciRootBridgeIoProtocolGuid,
      (VOID **)&PciRootBridgeIo,
      This->DriverBindingHandle,
      Controller,
      EFI_OPEN_PROTOCOL_BY_DRIVER);

  if (Status == EFI_ALREADY_STARTED)
  {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR(Status))
  {
    return Status;
  }

  // Close the I/O Abstraction(s) used to perform the supported test
  gBS->CloseProtocol(
      Controller,
      &gEfiPciRootBridgeIoProtocolGuid,
      This->DriverBindingHandle,
      Controller);

  if (IsProtocolsMapped(This))
  {
    // Protocol install already done. Just call Supported() on base driver.
    return gDriverBindingSubstituteProtocol->Supported(&gPciBusDriverBinding, Controller, RemainingDevicePath);
  }

  // The sequencing of this is tricky...
  // Install substitute root bridge protocol now, because Supported() of the base driver will want it
  Status = InstallRootBridgeIoProtocolProtocol(This, Controller, &newIoMapping);

  if (Status != EFI_SUCCESS)
    return Status;

  Status = gDriverBindingSubstituteProtocol->Supported(&gPciBusDriverBinding, Controller, RemainingDevicePath);

  ASSERT_EFI_ERROR(Status);

  if (Status != EFI_SUCCESS)
  {
    // But it might want other stuff too which isn't available, so pull our protocol and try again next time.
    UninstallRootBridgeIoProtocolProtocol(Controller, newIoMapping);
  }
  else
  {
    ResourceAllocationProtocolMapping *newResourceAllocationMapping;

    // It is now safe to install the resource allocation protocol substitute
    if (InstallResourceAllocationProtocol(newIoMapping, &newResourceAllocationMapping) == EFI_SUCCESS)
    {
      newIoMapping->Parent = newResourceAllocationMapping;
      InsertTailList(&RootBridgeIoProtocolList, &newIoMapping->Link);
    }
  }

  return Status;
}

/**
  Start this driver on ControllerHandle and enumerate Pci bus and start
  all device under PCI bus.

  @param  This                 Protocol instance pointer.
  @param  Controller           Handle of device to bind driver to.
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle.
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
PciBusDriverBindingStart(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE Controller,
    IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath)
{
  EFI_STATUS Status;
  EFI_DRIVER_BINDING_PROTOCOL *OriginalProtocol = FindDriverBindingProtocol();

  DEBUG((DEBUG_INFO, "PciBusDriverBindingStart()\n"));

  Status = OpenBaseProtocols(); // Hook our shim up to the base driver

  if (Status != EFI_SUCCESS)
    return Status;

  return OriginalProtocol->Start(OriginalProtocol, Controller, RemainingDevicePath);
}

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  Controller        Handle of device to stop driver on.
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle.
  @retval other             This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
PciBusDriverBindingStop(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE Controller,
    IN UINTN NumberOfChildren,
    IN EFI_HANDLE *ChildHandleBuffer)
{
  EFI_STATUS Status;
  EFI_DRIVER_BINDING_PROTOCOL *OriginalProtocol = FindDriverBindingProtocol();
  
  DEBUG((DEBUG_INFO, "PciBusDriverBindingStop()\n"));

  Status = OriginalProtocol->Stop(OriginalProtocol, Controller, NumberOfChildren, ChildHandleBuffer);

  if (Status != EFI_SUCCESS)
    return Status;

  Status = CleanupDriver(This, Controller);

  return Status;
}

/**
  Get a pointer to the binding protocol in the original PciBus driver

  @retval (pointer)           Pointer to the original binding procol
  @retval NULL                Unable to locate substitute binding procol.
                              Original driver probably not correctly modified.

**/
EFI_DRIVER_BINDING_PROTOCOL *FindDriverBindingProtocol()
{
  if (gDriverBindingSubstituteProtocol != NULL)
    return gDriverBindingSubstituteProtocol;

  gBS->LocateProtocol(&gDriverBindingProtocolSubstituteGuid, NULL, (VOID **)&gDriverBindingSubstituteProtocol);

  if (gDriverBindingSubstituteProtocol != NULL)
    return gDriverBindingSubstituteProtocol;

  return NULL;
}

/**
  Check if the protocols have been mapped for this driver.

  @param  Protocol            Binding protocol handle for this driver

  @retval TRUE                Protocols installed
  @retval FALSE               Protocols not yet installed

**/
BOOLEAN IsProtocolsMapped(EFI_DRIVER_BINDING_PROTOCOL *Protocol)
{
  LIST_ENTRY *mappingEntry;

  for (mappingEntry = GetFirstNode(&RootBridgeIoProtocolList); !IsNull(&RootBridgeIoProtocolList, mappingEntry); mappingEntry = GetNextNode(&RootBridgeIoProtocolList, mappingEntry))
  {
    RootBridgeIoProtocolMapping *mapping = (RootBridgeIoProtocolMapping *)mappingEntry;

    if (mapping->BindingProtocol == Protocol)
      return TRUE;
  }

  return FALSE;
}

/**
  Find a root bridge I/O protocol mapping by pointer to a substitute.

  @param  Substitute          Pointer to a substitute protocol.

  @retval (pointer)           Pointer to original protocol.
  @retval NULL                No protocol found.
                              Original driver probably not correctly modified.

**/
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *FindRootBridgeIoProtocolMappingBySubstitute(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *Substitute)
{
  LIST_ENTRY *mappingEntry;

  for (mappingEntry = GetFirstNode(&RootBridgeIoProtocolList); !IsNull(&RootBridgeIoProtocolList, mappingEntry); mappingEntry = GetNextNode(&RootBridgeIoProtocolList, mappingEntry))
  {
    RootBridgeIoProtocolMapping *mapping = (RootBridgeIoProtocolMapping *)mappingEntry;

    if (&mapping->SubstitutedProtocol == Substitute)
      return mapping->OriginalProtocol;
  }

  DEBUG((DEBUG_ERROR, "FindRootBridgeIoProtocolMappingBySubstitute(): Failed to find protocol\n"));

  return NULL;
}

/**
  Find a resource allocation protocol mapping by pointer to a substitute.

  @param  Substitute          Pointer to a substitute protocol.

  @retval (pointer)           Pointer to original protocol.
  @retval NULL                No protocol found.
                              Original driver probably not correctly modified.

**/
EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *FindResourceAllocationProtocolMappingBySubstitute(EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *Substitute)
{
  LIST_ENTRY *mappingEntry;

  for (mappingEntry = GetFirstNode(&RootBridgeIoProtocolList); !IsNull(&RootBridgeIoProtocolList, mappingEntry); mappingEntry = GetNextNode(&RootBridgeIoProtocolList, mappingEntry))
  {
    RootBridgeIoProtocolMapping *mapping = (RootBridgeIoProtocolMapping *)mappingEntry;

    if (&mapping->Parent->SubstitutedProtocol == Substitute)
      return mapping->Parent->OriginalProtocol;
  }

  DEBUG((DEBUG_ERROR, "FindRootBridgeIoProtocolMappingBySubstitute(): Failed to find protocol\n"));

  return NULL;
}
