#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <keyboard.h>
#include <keymap.h>

static struct kb_inbuf kb_in;
static int code_with_E0;
static int shift_l, shift_r;
static int alt_l, alt_r;
static int ctrl_l, ctrl_r;
static int caps_lock, num_lock, scroll_lock;
static int column;

static unsigned char get_from_kbuf();
static void kb_wait();
static void kb_ack();
static void set_leds();

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

	key_pressed = 1;
}

void init_keyboard()
{
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;

	shift_l = shift_r = 0;
	ctrl_l = ctrl_r = 0;
	alt_l = alt_r = 0;

	caps_lock = 0;
	num_lock = 1;
	scroll_lock = 0;

	column = 0;

	set_leds();

	put_irq_handler(KEYBOARD_IRQ, keyboard_handler);
	enable_irq(KEYBOARD_IRQ);
}

void keyboard_read(struct TTY* tty)
{
	unsigned char scan_code;
	int make;

	unsigned int key = 0;
	unsigned int* keyrow;

	while (kb_in.count > 0) {
		code_with_E0 = 0;
		scan_code = get_from_kbuf();

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
			code_with_E0 = 1;
			scan_code = get_from_kbuf();
			if (scan_code == 0x2A) {
				code_with_E0 = 0;
				if ((scan_code = get_from_kbuf()) == 0xE0) {
					code_with_E0 = 1;
					if ((scan_code = get_from_kbuf()) == 0x37) {
						key = PRINTSCREEN;
						make = 1;
					}
				}
			} else if (scan_code == 0xB7) {
				code_with_E0 = 0;
				if ((scan_code = get_from_kbuf()) == 0xE0) {
					code_with_E0 = 1;
					if ((scan_code = get_from_kbuf()) == 0xAA) {
						key = PRINTSCREEN;
						make = 0;
					}
				}
			}
		} 

		if (key != PAUSEBREAK && key != PRINTSCREEN) {
			int caps;
			make = (scan_code & FLAG_BREAK) ? 0 : 1;
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			column = 0;
			
			caps = shift_l || shift_r;
			if (caps_lock) {
				if (keyrow[0] >= 'a' && keyrow[0] <= 'z') {
					caps = !caps;
				}
			}
			if (caps) {
				column = 1;
			}

			if (code_with_E0)
				column = 2;

			key = keyrow[column];

			switch (key) {
			case SHIFT_L:
				shift_l = make;
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
			case CAPS_LOCK:
				if (make) {
					caps_lock = !caps_lock;
					set_leds();
				}
				break;
			case NUM_LOCK:
				if (make) {
					num_lock = !num_lock;
					set_leds();
				}
				break;
			case SCROLL_LOCK:
				if (make) {
					scroll_lock = !scroll_lock;
					set_leds();
				}
				break;
			default:
				break;
			}

			if (make) {
				int pad = 0;
				if (key >= PAD_SLASH && key <= PAD_9) {
					pad = 1;
					switch (key) {
					case PAD_SLASH:
						key = '/';
						break;
					case PAD_STAR:
						key = '*';
						break;
					case PAD_MINUS:
						key = '-';
						break;
					case PAD_PLUS:
						key = '+';
						break;
					case PAD_ENTER:
						key = ENTER;
						break;
					default:
						if (num_lock && key >= PAD_0 && key <= PAD_9) {
							key = key - PAD_0 + '0';
						} else if (num_lock && PAD_DOT == key) {
							key = '.';
						} else {
							switch (key) {
							case PAD_HOME:
								key = HOME;
								break;
							case PAD_END:
								key = END;
								break;
							case PAD_PAGEUP:
								key = PAGEUP;
								break;
							case PAD_PAGEDOWN:
								key = PAGEDOWN;
								break;
							case PAD_INS:
								key = INSERT;
								break;
							case PAD_UP:
								key = UP;
								break;
							case PAD_DOWN:
								key = DOWN;
								break;
							case PAD_LEFT:
								key = LEFT;
								break;
							case PAD_RIGHT:
								key = RIGHT;
								break;
							case PAD_DOT:
								key = DELETE;
								break;
							default:
								break;
							}
						}
						break;
					}
				}
				key |= shift_l ? FLAG_SHIFT_L : 0;
				key |= shift_r ? FLAG_SHIFT_R : 0;
				key |= ctrl_l ? FLAG_CTRL_L : 0;
				key |= ctrl_r ? FLAG_CTRL_R : 0;
				key |= alt_l ? FLAG_ALT_L : 0;
				key |= alt_r ? FLAG_ALT_R : 0;
				key |= pad ? FLAG_PAD : 0;
				
				in_process(tty, key);
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

static void kb_wait()
{
	unsigned char state;
	do {
		state = in_byte(KB_CMD);
	} while (state & 0x02);
}

static void kb_ack()
{
	unsigned char read;
	do {
		read = in_byte(KB_DATA);
	} while (read != KB_ACK);
}

static void set_leds()
{
	unsigned char leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
	kb_wait();
	out_byte(KB_DATA, LED_CODE);
	kb_ack();

	kb_wait();
	out_byte(KB_DATA, leds);
	kb_ack();
}