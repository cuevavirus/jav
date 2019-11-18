/*
volume_profile - volume.c
Copyright (C) 2019 浅倉麗子

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <psp2/avconfig.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/registrymgr.h>
#include "volume.h"

#define SLEEP_TIME (50 * 1000)
#define SLEEP sceKernelDelayThread(SLEEP_TIME)
#define SOUND_REG "/CONFIG/SOUND"

extern int sceAVConfigWriteRegSystemVol(int vol);
extern int sceAVConfigGetVolCtrlEnable(int *unk, int *muted, int *avls);

extern int sceAVConfigChangeReg(int k, int v);
#define AVCONFIG_REG_BT_MIC 0
#define AVCONFIG_REG_AVLS 1
#define AVCONFIG_REG_SOUND_OUTPUT 2
#define AVCONFIG_REG_AVLS_TIMER 3
#define AVCONFIG_REG_SPEAKER_MUTE 4

int get_output(void) {
	SceCtrlData cd;
	while (sceCtrlPeekBufferPositive(0, &cd, 1) < 0) { SLEEP; }
	return !!(cd.buttons & SCE_CTRL_HEADPHONE);
}

int get_volume(void) {
	int vol;
	while (sceAVConfigGetSystemVol(&vol) < 0) { SLEEP; }
	return vol;
}

void set_volume(int vol) {
	// sceAVConfigSetSystemVol can return 0 but volume is still not set
	while (sceAVConfigSetSystemVol(vol) < 0 || get_volume() != vol) { SLEEP; }
	while (sceAVConfigWriteRegSystemVol(vol) < 0) { SLEEP; }
}

int get_muted(void) {
	int r1, muted, r3;
	while (sceAVConfigGetVolCtrlEnable(&r1, &muted, &r3) < 0) { SLEEP; }
	return muted;
}

void mute_on(void) {
	// sceAVConfigMuteOn can return 0 but still not muted
	while (sceAVConfigMuteOn() < 0 || !get_muted()) { SLEEP; }
}

int get_avls(void) {
	int r1, r2, avls;
	while (sceAVConfigGetVolCtrlEnable(&r1, &r2, &avls) < 0) { SLEEP; }
	return avls;
}

void set_avls(int v) {
	while (sceAVConfigChangeReg(AVCONFIG_REG_AVLS, v) < 0) { SLEEP; }
	while (sceRegMgrSetKeyInt(SOUND_REG, "avls", v) < 0) { SLEEP; }
}

void disable_avls_timer(void) {
	while (sceAVConfigChangeReg(AVCONFIG_REG_AVLS_TIMER, 0) < 0) { SLEEP; }
	int ret;
	while (sceRegMgrGetKeyInt(SOUND_REG, "avls_timer", &ret) < 0) { SLEEP; }
	if (ret != 0) {
		while (sceRegMgrSetKeyInt(SOUND_REG, "avls_timer", 0) < 0) { SLEEP; }
	}
}

int get_speaker_mute(void) {
	int ret;
	while(sceRegMgrGetKeyInt(SOUND_REG, "speaker_mute", &ret) < 0) { SLEEP; }
	return ret;
}

void set_speaker_mute(int v) {
	while (sceAVConfigChangeReg(AVCONFIG_REG_SPEAKER_MUTE, v) < 0) { SLEEP; }
	while (sceRegMgrSetKeyInt(SOUND_REG, "speaker_mute", v) < 0) { SLEEP; }
}
