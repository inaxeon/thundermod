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

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
	return EFI_SUCCESS;
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
GetRootHpcList (
  IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL    *This,
  OUT UINTN                            *HpcCount,
  OUT EFI_HPC_LOCATION                 **HpcList
  )
{
  DEBUG((DEBUG_ERROR, "GetRootHpcList()\n"));

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
InitializeRootHpc (
  IN  EFI_PCI_HOT_PLUG_INIT_PROTOCOL      *This,
  IN  EFI_DEVICE_PATH_PROTOCOL            *HpcDevicePath,
  IN  UINT64                              HpcPciAddress,
  IN  EFI_EVENT                           Event, OPTIONAL
  OUT EFI_HPC_STATE                       *HpcState
  )
{
  if (Event) {
    gBS->SignalEvent (Event);
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
GetResourcePadding (
  IN  EFI_PCI_HOT_PLUG_INIT_PROTOCOL  *This,
  IN  EFI_DEVICE_PATH_PROTOCOL        *HpcDevicePath,
  IN  UINT64                          HpcPciAddress,
  OUT EFI_HPC_STATE                   *HpcState,
  OUT VOID                            **Padding,
  OUT EFI_HPC_PADDING_ATTRIBUTES      *Attributes
  )
{
  // EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *PaddingResource;
  // EFI_STATUS                        Status;
  // UINT8                             RsvdExtraBusNum = 0;
  // UINT16                            RsvdPcieMegaMem = 10;
  // UINT8                             PcieMemAddrRngMax = 0;
  // UINT16                            RsvdPciePMegaMem = 10;
  // UINT8                             PciePMemAddrRngMax = 0;
  // UINT8                             RsvdTbtExtraBusNum = 0;
  // UINT16                            RsvdTbtPcieMegaMem = 10;
  // UINT8                             TbtPcieMemAddrRngMax = 0;
  // UINT16                            RsvdTbtPciePMegaMem = 10;
  // UINT8                             TbtPciePMemAddrRngMax = 0;
  // UINT8                             RsvdPcieKiloIo = 4;
  // BOOLEAN                           SetResourceforTbt = FALSE;
  // UINTN                             RpIndex;
  // UINTN                             RpDev;
  // UINTN                             RpFunc;

  DEBUG ((DEBUG_ERROR, "GetResourcePaddin()\n"));

  return EFI_SUCCESS;
}