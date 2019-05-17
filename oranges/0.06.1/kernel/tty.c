#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <keyboard.h>

void task_tty()
{
	while(1) {
		keyboard_read();
	}
}

void in_process(unsigned int key)
{
	char output[2] = { '\0', '\0' };
	if (!(key & FLAG_EXT)) {
		output[0] = key & 0xFF;
		disp_str(output);

		disable_int();
		out_byte(CRTC_ADDR_REG, CURSOR_H);
		out_byte(CRTC_DATA_REG, ((disp_pos / 2) >> 8) & 0xFF);
		out_byte(CRTC_ADDR_REG, CURSOR_L);
		out_byte(CRTC_DATA_REG, (disp_pos / 2) & 0xFF);
		enable_int();
	} else {
		int raw_code = key & MASK_RAW;
		switch (raw_code) {
		case UP:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				disable_int();
				out_byte(CRTC_ADDR_REG, START_ADDR_H);
				out_byte(CRTC_DATA_REG, ((80 * 15) >> 8) & 0xFF);
				out_byte(CRTC_ADDR_REG, START_ADDR_L);
				out_byte(CRTC_DATA_REG, (80*15) & 0xFF);
				enable_int();
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				disable_int();
				out_byte(CRTC_ADDR_REG, START_ADDR_H);
				out_byte(CRTC_DATA_REG, 0 & 0xFF);
				out_byte(CRTC_ADDR_REG, START_ADDR_L);
				out_byte(CRTC_DATA_REG, 0 & 0xFF);
				enable_int();
			}
			break;
		default:
			break;
		}

	}
}