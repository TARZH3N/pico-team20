/***************************************************************************
 *                                                                         *
 *          ###########   ###########   ##########    ##########           *
 *         ############  ############  ############  ############          *
 *         ##            ##            ##   ##   ##  ##        ##          *
 *         ##            ##            ##   ##   ##  ##        ##          *
 *         ###########   ####  ######  ##   ##   ##  ##    ######          *
 *          ###########  ####  #       ##   ##   ##  ##    #    #          *
 *                   ##  ##    ######  ##   ##   ##  ##    #    #          *
 *                   ##  ##    #       ##   ##   ##  ##    #    #          *
 *         ############  ##### ######  ##   ##   ##  ##### ######          *
 *         ###########    ###########  ##   ##   ##   ##########           *
 *                                                                         *
 *            S E C U R E   M O B I L E   N E T W O R K I N G              *
 *                                                                         *
 * This file is part of NexMon.                                            *
 *                                                                         *
 * Copyright (c) 2016 NexMon Team                                          *
 *                                                                         *
 * NexMon is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * NexMon is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with NexMon. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 **************************************************************************/

#pragma NEXMON targetregion "patch"

#include <firmware_version.h>
#include <wrapper.h>	// wrapper definitions for functions that already exist in the firmware
#include <structs.h>	// structures that are used by the code in the firmware
#include <patcher.h>
#include <helper.h>
#include <capabilities.h>      // capabilities included in a nexmon patch

extern unsigned char ucode1_compressed_bin[];
extern unsigned int ucode1_compressed_bin_len;
extern unsigned char ucode2_compressed_bin[];
extern unsigned int ucode2_compressed_bin_len;

int capabilities = 0;

// Hook the call to wlc_ucode_write in wlc_ucode_download
__attribute__((at(WLC_UCODE_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_ucode_write_compressed, wlc_ucode_write_compressed_args);

__attribute__((at(UCODE1START_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(ucode1start_ptr, ucode1_compressed_bin);

__attribute__((at(UCODE1SIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(ucode1size_ptr, &ucode1_compressed_bin_len);

__attribute__((at(UCODE2START_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(ucode2start_ptr, ucode2_compressed_bin);

__attribute__((at(UCODE2SIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(ucode2size_ptr, &ucode2_compressed_bin_len);

__attribute__((at(HNDRTE_RECLAIM_0_END_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(hndrte_reclaim_0_end, PATCHSTART);

// Moving template ram to another place in the ucode region
#if TEMPLATERAMSTART0_PTR != 0
extern unsigned char templateram0_bin[];

__attribute__((at(TEMPLATERAMSTART0_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(templateram0_bin, templateram0_bin);
#endif

#if TEMPLATERAMSTART1_PTR != 0
extern unsigned char templateram1_bin[];

__attribute__((at(TEMPLATERAMSTART1_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(templateram1_bin, templateram1_bin);
#endif

#if TEMPLATERAMSTART2_PTR != 0
extern unsigned char templateram2_bin[];

__attribute__((at(TEMPLATERAMSTART2_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(templateram2_bin, templateram2_bin);
#endif

// Moving vasip firmware to another place in the ucode region
#if VASIPSTART_PTR != 0
extern unsigned char vasip_bin[];

__attribute__((at(VASIPSTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(vasip_bin, vasip_bin);
#endif

// Deactivate the no execution bit (XN) for data area to allow to place firmware patches there
__attribute__((at(0x1D2B30, "", CHIP_VER_BCM4375b1, FW_VER_18_41_8_9_sta)))
__attribute__((at(0x1c55b8, "", CHIP_VER_ALL, FW_VER_18_38_18_sta)))
__attribute__((naked))
void
xn_patch(void)
{
	asm("bic r2, r2, 0x1000\n");
}
