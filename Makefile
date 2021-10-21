EDK2_TOOLS = $(PWD)/edk2/BaseTools/Source/C/bin
EDK2 = $(PWD)/edk2
BUILD = $(PWD)/build
TOOLS = $(PWD)/tools
SRC = $(PWD)/src
GENFFS = $(EDK2_TOOLS)/GenFfs
GENSEC = $(EDK2_TOOLS)/GenSec
GUIDSUB = $(TOOLS)/guidsub
PKGBUILD = $(PWD)/edk2/Build/MdeModule/RELEASE_GCC5/X64/src
PCIBUSBUILD = $(PWD)/edk2/Build/MdeModule/RELEASE_GCC5/X64/MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe/OUTPUT

.EXPORT_ALL_VARIABLES:
	EDK_TOOLS_PATH = $(PWD)/edk2/BaseTools

default: 
	@echo "Available targets:"

$(GUIDSUB): $(TOOLS)/guidsub.c
	gcc $(dir $@)/guidsub.c -o $@ -luuid

$(EDK2)/.configured:
	cd edk2 && git submodule update --init
	cd edk2 && bash -c '. edksetup.sh BaseTools'
	cd edk2 && patch -p1 --forward < ../patches/edk2-with-serial-debug.patch || true
	echo "ACTIVE_PLATFORM       = MdeModulePkg/MdeModulePkg.dsc" > edk2/Conf/target.txt
	echo "TARGET                = RELEASE" >> edk2/Conf/target.txt
	echo "TARGET_ARCH           = X64" >> edk2/Conf/target.txt
	echo "TOOL_CHAIN_CONF       = Conf/tools_def.txt" >> edk2/Conf/target.txt
	echo "TOOL_CHAIN_TAG        = GCC5" >> edk2/Conf/target.txt
	echo "BUILD_RULE_CONF = Conf/build_rule.txt"  >> edk2/Conf/target.txt
	cd edk2 && make -C BaseTools
	touch $@

linux-edk2: $(EDK2)/.configured
	cd edk2 && bash -c '. edksetup.sh BaseTools && build'
	cp -u $(PKGBUILD)/PciDxeShim/PciDxeShim/OUTPUT/PciDxeShim.efi $(BUILD)
	cp -u $(PKGBUILD)/PciDxeShim/PciDxeShim/OUTPUT/PciDxeShim.depex $(BUILD)
	cp -u $(PKGBUILD)/PciHotPlug/PciHotPlug/OUTPUT/PciHotPlug.efi $(BUILD)
	cp -u $(PKGBUILD)/PciHotPlug/PciHotPlug/OUTPUT/PciHotPlug.depex $(BUILD)
	cp -u $(PCIBUSBUILD)/PciBusDxe.efi $(BUILD)

$(BUILD)/PciBusDxe.ffs: $(BUILD)/PciBusDxe.efi $(GUIDSUB)
	mkdir -p build
	cp -f $< $(basename $@).Sub.efi
	#$(GUIDSUB) -f $(basename $@).Sub.efi -o 18A031AB-B443-4D1A-A5C0-0C09261E9F71 -n E8AD4538-0A8D-46E6-8A1F-0903B79A91BB
	#$(GUIDSUB) -f $(basename $@).Sub.efi -o 2F707EBB-4A1A-11D4-9A38-0090273FC14D -n F9E627D2-482F-49E9-A165-F022C96AF184
	#$(GUIDSUB) -f $(basename $@).Sub.efi -o CF8034BE-6768-4D8B-B739-7CCE683A9FBE -n 35F37E0E-3EB1-453A-A5AD-4B4C15A63C18
	$(GENSEC) -o $(basename $@).sec_ui -s EFI_SECTION_USER_INTERFACE -n "`cat $(PCIBUSBUILD)/$(basename $(notdir $@)).inf | grep -m1 BASE_NAME | cut -f 2 -d '=' | xargs`"
	$(GENSEC) $(basename $@).Sub.efi -s EFI_SECTION_PE32 -o $(basename $@).sec_pe32
	$(GENSEC) -s EFI_SECTION_COMPRESSION -c PI_STD $(basename $@).sec_pe32 $(basename $@).sec_ui -o $(basename $@).sec_compressed
	$(GENFFS) -i $(basename $@).sec_compressed -t EFI_FV_FILETYPE_DRIVER -g `cat $(PCIBUSBUILD)/$(basename $(notdir $@)).inf | grep -m1 FILE_GUID | cut -f 2 -d '=' | xargs` -s -o $(basename $@).ffs

$(BUILD)/%.ffs: $(BUILD)/%.efi $(BUILD)/%.depex
	mkdir -p build
	$(GENSEC) -s EFI_SECTION_DXE_DEPEX $(basename $@).depex -o $(basename $@).sec_depex
	$(GENSEC) -o $(basename $@).sec_ui -s EFI_SECTION_USER_INTERFACE -n "`cat $(SRC)/$(basename $(notdir $@))/$(basename $(notdir $@)).inf | grep -m1 BASE_NAME | cut -f 2 -d '=' | xargs`"
	$(GENSEC) $< -s EFI_SECTION_PE32 -o $(basename $@).sec_pe32
	$(GENSEC) -s EFI_SECTION_COMPRESSION -c PI_STD $(basename $@).sec_pe32 $(basename $@).sec_ui -o $(basename $@).sec_compressed
	$(GENFFS) -i $(basename $@).sec_depex -i $(basename $@).sec_compressed -t EFI_FV_FILETYPE_DRIVER -g `cat $(SRC)/$(basename $(notdir $@))/$(basename $(notdir $@)).inf | grep -m1 FILE_GUID | cut -f 2 -d '=' | xargs` -s -o $(basename $@).ffs

linux-efi-ffs: linux-edk2 $(BUILD)/PciHotPlug.ffs $(BUILD)/PciDxeShim.ffs $(BUILD)/PciBusDxe.ffs

clean: 
	rm -f $(GUIDSUB)