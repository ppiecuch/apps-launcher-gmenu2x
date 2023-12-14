#ifndef HW_OPENDINGUX_H
#define HW_OPENDINGUX_H


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
	if (FILE *f = fopen("/dev/mmcblk1p1", "r")) {
		fclose(f);
		return MMC_INSERT;
	}
	return MMC_REMOVE;
}

uint8_t getUDCStatus() {
	int val = -1;
	if (FILE *f = fopen("/sys/class/power_supply/usb/online", "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
		if (val == 1) {
			return UDC_CONNECT;
		}
	}
	return UDC_REMOVE;
}

void setUDCStatus(uint8_t udc) {
	if (udc == UDC_REMOVE) {
		INFO("USB Disconnected. Disabling devices...");
		system("/usr/bin/retrofw stop");
		return;
	}

	INFO("Enabling networking device");
	system("/usr/bin/retrofw network on");
	gmenu2x->inetIcon = "skin:imgs/inet.png";
}

uint8_t getTVOutStatus() {
	return TV_REMOVE;
}

int16_t getBatteryLevel(bool raw = false) {
	int val = -1;
	FILE *f;
	if (getUDC() == UDC_REMOVE && (f = fopen("/sys/class/power_supply/battery/capacity", "r"))) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	if (raw) return val;
	if ((val > 10000) || (val < 0)) return 6;
	if (val > 90) return 5; // 100%
	if (val > 75) return 4; // 80%
	if (val > 55) return 3; // 55%
	if (val > 30) return 2; // 30%
	if (val > 15) return 1; // 15%
	return 0; // 0% :(
}

uint8_t getBatteryStatus(int32_t val, int32_t min, int32_t max) {
	return 6;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

void enableTerminal() {
	/* Enable the framebuffer console */
	char c = '1';
	int fd = open("/sys/devices/virtual/vtconsole/vtcon1/bind", O_WRONLY);
	if (fd) {
		write(fd, &c, 1);
		close(fd);
	}

	fd = open("/dev/tty1", O_RDWR);
	if (fd) {
		ioctl(fd, VT_ACTIVATE, 1);
		close(fd);
	}
}

void setScaleMode(unsigned int mode) {
	if (FILE *f = fopen("/sys/devices/platform/jz-lcd.0/keep_aspect_ratio", "a")) {
		fprintf(f, "%d", (mode == 1));
		fclose(f);
	}

	if (FILE *f = fopen("/sys/devices/platform/jz-lcd.0/integer_scaling", "a")) {
		fprintf(f, "%d", (mode == 2));
		fclose(f);
	}
}

void setBacklight(int val) {
	if (FILE *f = fopen("/sys/class/backlight/pwm-backlight/brightness", "a")) {
		fprintf(f, "%0.0f", val * (255.0f / 100.0f)); // fputs(val, f);
		fclose(f);
	}

	if (FILE *f = fopen("/sys/class/graphics/fb0/blank", "a")) {
		fprintf(f, "%d", val <= 0);
		fclose(f);
	}
}

int16_t getBacklight() {
	int val = -1;
	if (FILE *f = fopen("/sys/class/backlight/pwm-backlight/brightness", "r")) {
		fscanf(f, "%i", &val);
		fclose(f);
	}
	return val * (100.0f / 255.0f);
}

void setVolume(int val) {
	val = val * (63.0f / 100.0f);

	int hp = 0;
	char cmd[96];

	if (val > 32) {
		hp = 31;
		val -= 32;
	} else if (val > 1) {
		hp = val;
		val = 1;
	}

	sprintf(cmd, "amixer set Headphone %d; amixer set PCM %d", hp, val);
	system(cmd);
}

int getVolume() {
	int pcm = 0, hp = 0;
	// if (FILE *f = popen("alsa-getvolume default PCM", "r")) {
	if (FILE *f = popen("amixer get PCM | grep -i \"Playback [0-9] \\[\" | cut -f 5 -d \" \" | head -n 1", "r")) {
		fscanf(f, "%i", &pcm);
		pclose(f);
	}
	// if (FILE *f = popen("alsa-getvolume default Headphone", "r")) {
	if (FILE *f = popen("amixer get Headphone | grep -i \"Playback [0-9] \\[\" | cut -f 5 -d \" \" | head -n 1", "r")) {
		fscanf(f, "%i", &hp);
		pclose(f);
	}

	return (pcm + hp) * (100.0f / 63.0f);
}

string hwPreLinkLaunch() {
	system("[ -d /home/retrofw ] && mount -o remount,sync /home/retrofw");
	return "";
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

	if (file_exists("/usr/bin/retrofw")) {
		system("[ -d /home/retrofw ] && mount -o remount,async /home/retrofw");
		opk = "retrofw";
		ipk = true;
	} else {
		opk = "gcw0";
		ipk = false;
	}

	if (FILE *f = fopen("/sys/devices/platform/jz-lcd.0/allow_downscaling", "a")) {
		fprintf(f, "1");
		fclose(f);
	}

#if defined(OPK_SUPPORT)
	system("umount -fl /mnt");
#endif
	}

	uint16_t getBatteryLevel() {
		return 6;
	};
};

#endif
