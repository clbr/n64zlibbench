#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>
#include "foo.h"
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

volatile u32 frames;

u32 adler32(u32 adler, const u8 *buf, u16 len);

#define SIZE 7651

display_context_t lockVideo(int wait)
{
	display_context_t dc;

	if (wait)
		while (!(dc = display_lock()));
	else
		dc = display_lock();
	return dc;
}

void unlockVideo(display_context_t dc)
{
	if (dc)
		display_show(dc);
}

/* text functions */
void printText(display_context_t dc, char *msg, int x, int y)
{
	if (dc)
		graphics_draw_text(dc, x*8, y*8, msg);
}

/* vblank callback */
void vblCallback(void)
{
	frames++;
}

void delay(u32 cnt)
{
	u32 then = frames + cnt;
	while (then > frames);
}

/* initialize console hardware */
void init_n64(void)
{
	/* enable interrupts (on the CPU) */
	init_interrupts();

	/* Initialize peripherals */
	display_init(RESOLUTION_640x240, DEPTH_32_BPP, 2, GAMMA_NONE, ANTIALIAS_OFF);

	register_VI_handler(vblCallback);

	controller_init();
}

static u8 buf[SIZE];
int zlib_unpack(void *dest, const unsigned outLen, const void *src, const unsigned srcLen);

/* main code entry point */
int main(void)
{
	const u32 expected = 0x55ad5848;
	u32 sum = 0, i, start;

	display_context_t disp;

	init_n64();

	{
		u32 color;

		disp = lockVideo(1);
		color = graphics_make_color(0xCC, 0xCC, 0xCC, 0xFF);
		graphics_fill_screen(disp, color);

		color = graphics_make_color(0x00, 0x00, 0x00, 0xFF);
		graphics_set_color(color, 0);

		printText(disp, "Calculating...", 4, 4);

		unlockVideo(disp);

		delay(1);
	}

	start = frames;
int ret;
	// Decompress
	for (i = 0; i < 3200; i++)
		ret = zlib_unpack(buf, SIZE, out_bin, sizeof(out_bin));

	sum = adler32(0, NULL, 0);
	sum = adler32(sum, buf, SIZE);

	start = frames - start;

	{
		char textbuf[64];
		u32 color;

		disp = lockVideo(1);
		color = graphics_make_color(0xCC, 0xCC, 0xCC, 0xFF);
		graphics_fill_screen(disp, color);

		color = graphics_make_color(0x00, 0x00, 0x00, 0xFF);
		graphics_set_color(color, 0);

		sprintf(textbuf, "%lu blanks (%lu s)", start, start / 60);
		printText(disp, textbuf, 4, 4);

		sprintf(textbuf, "checksum %lx", sum);
		printText(disp, textbuf, 4, 5);

		sprintf(textbuf, "expected %lx, ret %d", expected, ret);
		printText(disp, textbuf, 4, 6);

		unlockVideo(disp);

		delay(1);
	}

	while (1);

	return 0;
}
