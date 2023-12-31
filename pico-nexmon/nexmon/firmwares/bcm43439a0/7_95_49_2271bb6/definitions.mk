NEXMON_CHIP=CHIP_VER_BCM43439a0
NEXMON_CHIP_NUM=`$(NEXMON_ROOT)/buildtools/scripts/getdefine.sh $(NEXMON_CHIP)`
NEXMON_FW_VERSION=FW_VER_7_95_49_2271bb6
NEXMON_FW_VERSION_NUM=`$(NEXMON_ROOT)/buildtools/scripts/getdefine.sh $(NEXMON_FW_VERSION)`

NEXMON_ARCH=armv7-m

RAM_FILE=43439A0.bin
RAMSTART=0x0
RAMSIZE=0x80000

ROM_FILE=rom.bin
ROMSTART=0x800000
ROMSIZE=0xA0000

WLC_UCODE_WRITE_BL_HOOK_ADDR=0x216A8
HNDRTE_RECLAIM_0_END_PTR=0x1888
HNDRTE_RECLAIM_0_END=0x36A8C 

PATCHSIZE=0x2000
PATCHSTART=$$(($(HNDRTE_RECLAIM_0_END) - $(PATCHSIZE)))

# original ucode start and size
UCODESTART=0x2B87C
UCODESIZE=0xABCD

# original save restore source code start and size
TEMPLATERAMSTART_PTR=0x2B77C
TEMPLATERAMSTART=0x3644C
TEMPLATERAMSIZE=0x640

FP_DATA_END_PTR=0x15CA0
FP_CONFIG_BASE_PTR_1=0x16F80
FP_CONFIG_END_PTR_1=0x16F7C
FP_CONFIG_BASE_PTR_2=0x171F8
FP_CONFIG_END_PTR_2=0x171F4
# can start at the end of the firmware, it will be overwritten after it is read
FP_CONFIG_BASE=$$(($(PATCHSTART) - $(FP_CONFIG_SIZE)))
FP_DATA_BASE=0x800
FP_CONFIG_SIZE=0xC00
FP_CONFIG_ORIGBASE=0x1000
FP_CONFIG_ORIGEND=0x1390

VERSION_PTR_1=0x4DEC
VERSION_PTR_2=0x1E3AC
VERSION_PTR_3=0x18194
