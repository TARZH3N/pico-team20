include definitions.mk

all: ucode.bin templateram.bin flashpatches.c

ucode.bin: $(RAM_FILE) definitions.mk
	@printf "\033[0;31m  EXTRACTING UCODE\033[0m\n"
	$(Q)dd if=$< of=$@ bs=1 skip=$$(($(UCODESTART)-$(RAMSTART))) count=$$(($(UCODESIZE))) 2>/dev/null

templateram.bin: $(RAM_FILE) definitions.mk
	@printf "\033[0;31m  EXTRACTING TEMPLATERAM\033[0m\n"
	$(Q)dd if=$< of=$@ bs=1 skip=$$(($(TEMPLATERAMSTART)-$(RAMSTART))) count=$$(($(TEMPLATERAMSIZE))) 2>/dev/null

flashpatches.c: $(RAM_FILE) definitions.mk
	@printf "\033[0;31m  EXTRACTING FLASHPATCHES\033[0m\n"
	$(Q)printf "#include <patcher.h>\n\n" > flashpatches.c
	$(Q)$(NEXMON_ROOT)/buildtools/flash_patch_extractor/fpext -x -r $< -s $(RAMSTART) -b $(FP_CONFIG_ORIGBASE) -e $(FP_CONFIG_ORIGEND) >> $@

rom.bin: ../rom.bin
	@printf "\033[0;31m  APPLYING FLASHPATCHES TO CLEAN ROM\033[0m\n"
	$(Q)$(NEXMON_ROOT)/buildtools/flash_patch_extractor/fpext -x -r $(RAM_FILE) -s $(RAMSTART) -b $(FP_CONFIG_ORIGBASE) -e $(FP_CONFIG_ORIGEND) -i $< -o $@ -t $(ROMSTART) > /dev/null

complete_mfg.bin: bcmdhd_mfg.bin_c0 rom.bin
	@printf "\033[0;31m  MERGING %s and rom.bin into %s\033[0m\n" $< $@
	$(Q)dd if=rom.bin of=$@ bs=1 seek=$$(($(ROMSTART))) conv=notrunc 2>/dev/null
	$(Q)dd if=$< of=$@ bs=1 seek=$$(($(RAMSTART))) conv=notrunc 2>/dev/null

complete_sta.bin: bcmdhd_sta.bin_c0 rom.bin
	@printf "\033[0;31m  MERGING %s and rom.bin into %s\033[0m\n" $< $@
	$(Q)dd if=rom.bin of=$@ bs=1 seek=$$(($(ROMSTART))) conv=notrunc 2>/dev/null
	$(Q)dd if=$< of=$@ bs=1 seek=$$(($(RAMSTART))) conv=notrunc 2>/dev/null

clean:
	@printf "\033[0;31m  CLEANING\033[0m\n"
	$(Q)rm -f ucode.bin templateram.bin flashpatches.c rom.bin complete_sta.bin complete_mfg.bin
