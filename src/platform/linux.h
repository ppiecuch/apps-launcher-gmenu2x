#ifndef HW_PC_H
#define HW_PC_H

#ifndef __APPLE__
#include <linux/soundcard.h>

volatile uint16_t *memregs;
uint8_t memdev = 0;
int SOUND_MIXER_READ = SOUND_MIXER_READ_PCM;
int SOUND_MIXER_WRITE = SOUND_MIXER_WRITE_PCM;
#endif // __APPLE__

int32_t setTVoff() {
	return 0;
}

uint16_t getDevStatus() {
	FILE *f;
	char buf[10000];
	if ((f = fopen("/proc/bus/input/devices", "r"))) { // if ((f = fopen("/proc/bus/input/handlers", "r"))) {
		size_t sz = fread(buf, sizeof(char), 10000, f);
		fclose(f);
		return sz;
	}
	return 0;
}

uint8_t getMMCStatus() {
	return MMC_REMOVE;
}

uint8_t getUDCStatus() {
	return UDC_REMOVE;
}

uint8_t getTVOutStatus() {
	return TV_REMOVE;
}

int16_t getBatteryLevel() {
	return 0;
}

uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
	return 6;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

// https://github.com/paradigmic/cpushow/blob/master/pcd8544_rpi.c
#ifdef RASPBERRY_PI
#include <sys/sysinfo.h>
bool getSysInfo(float &cpuload, uint64_t &totalram, uint64_t &procs) {
	struct sysinfo sys_info;
	if(sysinfo(&sys_info) != 0) {
		return false;
	}

	cpuload = ((float)sys_info.loads[0])/(1<<SI_LOAD_SHIFT); // cpu info
	totalram = sys_info.freeram / 1024 / 1024; // freeram
	procs = sys_info.procs; // processes

	return true;
}
#else
bool getSysInfo(float &cpuload, uint64_t &totalram, uint64_t &procs) {
	return false;
}
#endif // RASPBERRY_PI

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	numJoy = getDevStatus();
	INFO("%s:%d: %s\n"
		"  - devStatus: %d\n"
		"  - batteryLevel: %d\n"
		"  - MMCStatus: %d\n"
		"  - UDCStatus: %d\n"
		, __FILE__, __LINE__, __func__, getDevStatus(), getBatteryLevel(), getMMCStatus(), getUDCStatus());
	if (numJoyPrev != numJoy) {
		numJoyPrev = numJoy;
		InputManager::pushEvent(JOYSTICK_CONNECT);
	}

	return 0;
}

class GMenuNX : public GMenu2X {
private:
	void hwInit() {
		CPU_MENU = 528;
		CPU_LINK = 600;
		CPU_MAX = 700;
		CPU_MIN = 500;
		CPU_STEP = 5;

		w = 320;
		h = 240;
	}

	uint16_t getBatteryLevel() {
		return 6;
	};
};

#endif
