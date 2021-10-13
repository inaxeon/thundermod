EDK2_TOOLS = $(PWD)/edk2/BaseTools/Source/C/bin
BUILD = $(PWD)/build
SRC = $(PWD)/src
GENFFS = $(EDK2_TOOLS)/GenFfs
GENSEC = $(EDK2_TOOLS)/GenSec
PKGBUILD = $(PWD)/edk2/Build/MdeModule/RELEASE_GCC5/X64/src

.EXPORT_ALL_VARIABLES:
	EDK_TOOLS_PATH = $(PWD)/edk2/BaseTools

default: 
	@echo "Available targets:"

linux-edk2-base:
	cd edk2 && git submodule update --init
	cd edk2 && bash -c '. edksetup.sh BaseTools'
	echo "ACTIVE_PLATFORM       = MdeModulePkg/MdeModulePkg.dsc" > edk2/Conf/target.txt
	echo "TARGET                = RELEASE" >> edk2/Conf/target.txt
	echo "TARGET_ARCH           = X64" >> edk2/Conf/target.txt
	echo "TOOL_CHAIN_CONF       = Conf/tools_def.txt" >> edk2/Conf/target.txt
	echo "TOOL_CHAIN_TAG        = GCC5" >> edk2/Conf/target.txt
	echo "BUILD_RULE_CONF = Conf/build_rule.txt"  >> edk2/Conf/target.txt
	cd edk2 && make -C BaseTools

linux-edk2:
	cd edk2 && patch -p1 --forward < ../patches/edk2-with-serial-debug.patch || true
	cd edk2 && bash -c '. edksetup.sh BaseTools && build'

$(BUILD)/%.ffs: linux-edk2
	mkdir -p build
	$(GENSEC) -s EFI_SECTION_DXE_DEPEX $(PKGBUILD)/$(basename $(notdir $@))/$(basename $(notdir $@))/OUTPUT/$(basename $(notdir $@)).depex -o $(basename $@).sec_depex
	$(GENSEC) -o $(basename $@).sec_ui -s EFI_SECTION_USER_INTERFACE -n "`cat $(SRC)/$(basename $(notdir $@))/$(basename $(notdir $@)).inf | grep BASE_NAME | cut -f 2 -d '=' | xargs`"
	$(GENSEC) $(PKGBUILD)/$(basename $(notdir $@))/$(basename $(notdir $@))/OUTPUT/$(basename $(notdir $@)).efi -s EFI_SECTION_PE32 -o $(basename $@).sec_pe32
	$(GENSEC) -s EFI_SECTION_COMPRESSION -c PI_STD $(basename $@).sec_pe32 $(basename $@).sec_ui -o $(basename $@).sec_compressed
	$(GENFFS) -i $(basename $@).sec_depex -i $(basename $@).sec_compressed -t EFI_FV_FILETYPE_DRIVER -g `cat $(SRC)/$(basename $(notdir $@))/$(basename $(notdir $@)).inf | grep FILE_GUID | cut -f 2 -d '=' | xargs` -s -o $(basename $@).ffs

linux-efi-ffs: $(BUILD)/PciHotPlug.ffs $(BUILD)/PciDxeShim.ffs

clean: 
