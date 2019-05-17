#include <const.h>
#include <type.h>
#include <proto.h>
#include <global.h>
#include <hd.h>

static void init_hd();
static void hd_identify(int drive);
static void hd_cmd_out(struct hd_cmd* cmd);
static void interrupt_wait();
static void print_identify_info(u16* hdinfo);
static int waitfor(int mask, int val, int timeout);

static u8 hd_status;
static u8 hdbuf[SECTOR_SIZE * 2];

void task_hd()
{
	struct Message msg;
	init_hd();

	while(1) {
		send_rec(RECEIVE, ANY, &msg);
		int src = msg.source;
		switch(msg.type) {
		case DEV_OPEN:
			hd_identify(0);
			break;
		default:
			dump_msg("Unknown message in task_hd():", &msg);
			spin("In task_hd(): invalid message type");
			break;
		}
		send_rec(SEND, src, &msg);
	}
}

static void init_hd()
{
	u8* nrDriver = (u8*)(0x475);
	printl("NrDriver: %d.\n", *nrDriver);
	assert(*nrDriver);

	put_irq_handler(AT_WINI_IRQ, hd_handler);
	enable_irq(CASCADE_IRQ);
	enable_irq(AT_WINI_IRQ);
}

static void hd_identify(int drive)
{
	struct hd_cmd cmd;
	cmd.device = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
		
	interrupt_wait();
	port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	print_identify_info((u16*)hdbuf);
}

static void print_identify_info(u16* hdinfo)
{
	int i, k;
	char s[64];

	struct ident_info_ascii {
		int idx;
		int len;
		char* desc;
	} iinfo[] = { {10, 20, "HD SN"}, {27,40,"HD Model"} };

	for (k = 0; k < sizeof(iinfo) / sizeof(iinfo[0]); k++) {
		char* p = (char*)&hdinfo[iinfo[k].idx];
		for (i = 0; i < iinfo[k].len / 2; i++) {
			s[i*2+1] = *p++;
			s[i*2] = *p++;
		}
		s[i*2] = 0;
		printl("%s: %s\n", iinfo[k].desc, s);
	}

	int capabilities = hdinfo[49];
	printl("LBA supported: %s\n", (capabilities & 0x0200) ? "Yes" : "No");
	int cmd_set_supported = hdinfo[83];
	printl("LBA48 supported: %s\n", (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = ((int)hdinfo[61] << 16) + hdinfo[60];
	printl("HD size: %dMB\n", sectors * 512 / 1000000);
}
static void interrupt_wait()
{
	struct Message msg;
	send_rec(RECEIVE, INTERRUPT, &msg);
}

static void hd_cmd_out(struct hd_cmd* cmd)
{
	if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
		panic("hd error");

	out_byte(REG_DEV_CTRL, 0);
	out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NSECTOR, cmd->count);
	out_byte(REG_LBA_LOW, cmd->lba_low);
	out_byte(REG_LBA_MID, cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE, cmd->device);
	out_byte(REG_CMD, cmd->command);
	
}

static int waitfor(int mask, int val, int timeout)
{
	int t = get_ticks();
	while ( ((get_ticks() - t) * 1000 / HZ) < timeout )
		if ( (in_byte(REG_STATUS) & mask) == val )
			return 1;
	return 0;
}

void hd_handler(int irq)
{
	hd_status = in_byte(REG_STATUS);
	inform_int(TASK_HD);
}