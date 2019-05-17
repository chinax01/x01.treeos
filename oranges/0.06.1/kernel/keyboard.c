#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <keyboard.h>
#include <keymap.h>

static KB_INPUT kb_in;
static int code_with_E0 = 0;
static int shift_l, shift_r;
static int alt_l, alt_r;
static int ctrl_l, ctrl_r;
static int caps_lock, num_lock, scroll_lock;
static int column;

static unsigned char get_from_kbuf();

void keyboard_handler(int irq)
{
	unsigned char scan_code = in_byte(KB_DATA);

	if (kb_in.count < KB_IN_BYTES) {
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}
}

void init_keyboard()
{
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	shift_l = shift_r = 0;
	ctrl_l = ctrl_r = 0;
	alt_l = alt_r = 0;

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

void keyboard_read()
{
	unsigned char scan_code;
	char output[2];
	int make;

	unsigned int key = 0;
	unsigned int* keyrow;

	if (kb_in.count > 0) {
		disable_int();
		scan_code = *(kb_in.p_tail);
		kb_in.p_tail++;
		if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_tail = kb_in.buf;
		}
		kb_in.count--;
		enable_int();
		
		if (scan_code == 0xE1) {
			int i;
			unsigned char pausebrk_scode[] = { 0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5 };
			int is_pausebreak = 1;
			for (i = 0; i < 6; i++) {
				if (get_from_kbuf() != pausebrk_scode[i]) {
					is_pausebreak = 0;
					break;
				}
			}
			if (is_pausebreak)
				key = PAUSEBREAK;
		} else if (scan_code == 0xE0) {
			scan_code = get_from_kbuf();
			if (scan_code == 0x2A) {
				if (get_from_kbuf() == 0xE0) {
					if (get_from_kbuf() == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			} else if (scan_code == 0xB7) {
				if (get_from_kbuf() == 0xE0) {
					if (get_from_kbuf() == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
			if (key == 0) {
				code_with_E0 = 1;
			}
		} 

		if (key != PAUSEBREAK && key != PRINTSCREEN) {
			make = (scan_code & FLAG_BREAK) ? FALSE : TRUE;
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			column = 0;
			if (shift_l || shift_r) {
				column = 1;
			}
			if (code_with_E0) {
				column = 2;
				code_with_E0 = 0;
			}
			key = keyrow[column];

			switch (key) {
			case SHIFT_L:
				shift_l = make;
				key = 0;
				break;
			case SHIFT_R:
				shift_r = make;
				break;
			case CTRL_L:
				ctrl_l = make;
				break;
			case CTRL_R:
				ctrl_r = make;
				break;
			case ALT_L:
				alt_l = make;
				break;
			case ALT_R:
				alt_r = make;
				break;
			default:
				break;
			}

			if (make) {
				key |= shift_l ? FLAG_SHIFT_L : 0;
				key |= shift_r ? FLAG_SHIFT_R : 0;
				key |= ctrl_l ? FLAG_CTRL_L : 0;
				key |= ctrl_r ? FLAG_CTRL_R : 0;
				key |= alt_l ? FLAG_ALT_L : 0;
				key |= alt_r ? FLAG_ALT_R : 0;

				in_process(key);
			}
		}
	}
}

static unsigned char get_from_kbuf()
{
	unsigned char scan_code;
	while (kb_in.count <= 0) {}

	disable_int();
	scan_code = *(kb_in.p_tail);
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	enable_int();

	return scan_code;
}