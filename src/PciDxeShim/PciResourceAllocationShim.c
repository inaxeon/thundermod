#include "PciDxeShim.h"
/**
 * File: PciResourceAllocationShim.c
 * Author: Matthew Millman
 *
 * Shim layer for EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL_GUID
 * 
 * Function headers taken from EDK2
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

/**

  Enter a certain phase of the PCI enumeration process.

  @param This   The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL instance.
  @param Phase  The phase during enumeration.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_INVALID_PARAMETER  Wrong phase parameter passed in.
  @retval EFI_NOT_READY          Resources have not been submitted yet.

**/
EFI_STATUS
EFIAPI
NotifyPhase(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE Phase)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "NotifyPhase()\n"));
  return OriginalProtocol->NotifyPhase(OriginalProtocol, Phase);
}

/**

  Return the device handle of the next PCI root bridge that is associated with
  this Host Bridge.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  Returns the device handle of the next PCI Root Bridge.
                           On input, it holds the RootBridgeHandle returned by the most
                           recent call to GetNextRootBridge().The handle for the first
                           PCI Root Bridge is returned if RootBridgeHandle is NULL on input.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_NOT_FOUND          Next PCI root bridge not found.
  @retval EFI_INVALID_PARAMETER  Wrong parameter passed in.

**/
EFI_STATUS
EFIAPI
GetNextRootBridge(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN OUT EFI_HANDLE *RootBridgeHandle)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "GetNextRootBridge()\n"));
  return OriginalProtocol->GetNextRootBridge(OriginalProtocol, RootBridgeHandle);
}

/**

  Returns the attributes of a PCI Root Bridge.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The device handle of the PCI Root Bridge
                           that the caller is interested in.
  @param Attributes        The pointer to attributes of the PCI Root Bridge.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_INVALID_PARAMETER  Attributes parameter passed in is NULL or
                                 RootBridgeHandle is not an EFI_HANDLE
                                 that was returned on a previous call to
                                 GetNextRootBridge().

**/
EFI_STATUS
EFIAPI
GetAttributes(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    OUT UINT64 *Attributes)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "GetAttributes()\n"));
  return OriginalProtocol->GetAllocAttributes(OriginalProtocol, RootBridgeHandle, Attributes);
}

/**

  This is the request from the PCI enumerator to set up
  the specified PCI Root Bridge for bus enumeration process.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The PCI Root Bridge to be set up.
  @param Configuration     Pointer to the pointer to the PCI bus resource descriptor.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_OUT_OF_RESOURCES   Not enough pool to be allocated.
  @retval EFI_INVALID_PARAMETER  RootBridgeHandle is not a valid handle.

**/
EFI_STATUS
EFIAPI
StartBusEnumeration(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    OUT VOID **Configuration)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "GetAttributes()\n"));
  return OriginalProtocol->StartBusEnumeration(OriginalProtocol, RootBridgeHandle, Configuration);
}

/**

  This function programs the PCI Root Bridge hardware so that
  it decodes the specified PCI bus range.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The PCI Root Bridge whose bus range is to be programmed.
  @param Configuration     The pointer to the PCI bus resource descriptor.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_INVALID_PARAMETER  Wrong parameters passed in.

**/
EFI_STATUS
EFIAPI
SetBusNumbers(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    IN VOID *Configuration)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "SetBusNumbers()\n"));
  return OriginalProtocol->SetBusNumbers(OriginalProtocol, RootBridgeHandle, Configuration);
}

/**

  Submits the I/O and memory resource requirements for the specified PCI Root Bridge.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The PCI Root Bridge whose I/O and memory resource requirements.
                           are being submitted.
  @param Configuration     The pointer to the PCI I/O and PCI memory resource descriptor.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_INVALID_PARAMETER  Wrong parameters passed in.
**/
EFI_STATUS
EFIAPI
SubmitResources(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    IN VOID *Configuration)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "SubmitResources()\n"));
  return OriginalProtocol->SubmitResources(OriginalProtocol, RootBridgeHandle, Configuration);
}

/**

  This function returns the proposed resource settings for the specified
  PCI Root Bridge.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The PCI Root Bridge handle.
  @param Configuration     The pointer to the pointer to the PCI I/O
                           and memory resource descriptor.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_OUT_OF_RESOURCES   Not enough pool to be allocated.
  @retval EFI_INVALID_PARAMETER  RootBridgeHandle is not a valid handle.

**/
EFI_STATUS
EFIAPI
GetProposedResources(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    OUT VOID **Configuration)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "GetProposedResources()\n"));
  return OriginalProtocol->GetProposedResources(OriginalProtocol, RootBridgeHandle, Configuration);
}

/**

  This function is called for all the PCI controllers that the PCI
  bus driver finds. Can be used to Preprogram the controller.

  @param This              The EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_ PROTOCOL instance.
  @param RootBridgeHandle  The PCI Root Bridge handle.
  @param PciAddress        Address of the controller on the PCI bus.
  @param Phase             The Phase during resource allocation.

  @retval EFI_SUCCESS            Succeed.
  @retval EFI_INVALID_PARAMETER  RootBridgeHandle is not a valid handle.

**/
EFI_STATUS
EFIAPI
PreprocessController(
    IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
    IN EFI_HANDLE RootBridgeHandle,
    IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS PciAddress,
    IN EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE Phase)
{
  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *OriginalProtocol = FindResourceAllocationProtocolMappingBySubstitute(This);
  DEBUG((DEBUG_INFO, "PreprocessController()\n"));
  return OriginalProtocol->PreprocessController(OriginalProtocol, RootBridgeHandle, PciAddress, Phase);
}
