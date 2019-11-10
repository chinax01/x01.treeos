#ifndef __HDREG_H__
#define __HDREG_H__

#define HARD_DISK_TYPE	17

#if HARD_DISK_TYPE == 17
#define _CYL	977
#define _HEAD	5
#define __WPCOM	300
#define _LZONE	977
#define _SECT 	17
#define _CTL	0
#elif HARD_DISK_TYPE == 18
#define _CYL	977
#define _HEAD	7
#define __WPCOM	(-1)
#define _LZONE	977
#define _SECT	17
#define _CTL	0
#else
#error Define HARD_DISK_TYPE and parameters, add your own entries as well
#endif

#if __WPCOM >= 0
#define _WPCOM	((__WPCOM) >> 2)
#else
#define _WPCOM	__WPCOM
#endif

// 控制寄存器端口
#define HD_DATA		0x1f0
#define HD_ERROR	0x1f1
#define HD_NSECTOR	0x1f2
#define HD_SECTOR	0x1f3
#define HD_LCYL		0x1f4
#define HD_HCYL		0x1f5
#define HD_CURRENT	0x1f6
#define HD_STATUS	0x1f7
#define HD_PERCOMP	HD_ERROR
#define HD_COMMAND	HD_STATUS

#define HD_CMD		0x3f6

// bits of HD_STATUS
#define ERR_STAT	0x01
#define INDEX_STAT	0x02
#define ECC_STAT	0x04
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80

// values of HD_COMMAND
#define WIN_RESTORE		0x10
#define WIN_READ		0x20
#define WIN_WRITE		0x30
#define WIN_VERIFY		0x40
#define WIN_FORMAT		0x50
#define WIN_INIT		0x60
#define WIN_SEEK		0x70
#define WIN_DIAGNOSE	0x90
#define WIN_SPECIFY		0x91

// bits for HD_ERROR
#define MARK_ERR		0x01
#define TRK0_ERR		0x02
#define ABRT_ERR		0x04
#define ID_ERR			0x10
#define ECC_ERR			0x40
#define BBD_ERR			0x80

struct  partition
{
	unsigned char boot_ind;
	unsigned char head;
	unsigned char sector;
	unsigned char cyl;
	unsigned char sys_ind;
	unsigned char end_head;
	unsigned char end_sector;
	unsigned char end_cyl;
	unsigned int start_sect;
	unsigned int nr_sects;
};

struct hd_driveid {
	unsigned short config;
	unsigned short cyls;
	unsigned short reserved2;
	unsigned short heads;
	unsigned short track_bytes;
	unsigned short sector_bytes;
	unsigned short sectors;
	unsigned short vendor0;
	unsigned short vendor1;
	unsigned short vendor2;
	unsigned char secrial_no[20];
	unsigned short buf_type;
	unsigned short buf_size;
	unsigned short ecc_bytes;
	unsigned char fw_rev[8];
	unsigned char model[40];
	unsigned char max_multsect;
	unsigned char vendor3;
	unsigned short dword_io;
	unsigned char vendor4;
	unsigned char capability;
	unsigned short reserved50;
	unsigned char vendor5;
	unsigned char tPIO;
	unsigned char vendor6;
	unsigned char tDMA;
	unsigned short field_valid;
	unsigned short cur_cyls;
	unsigned short cur_heads;
	unsigned short cur_sectors;
	unsigned short cur_capacity0;
	unsigned short cur_capacity1;
	unsigned char multsect;
	unsigned char multsect_valid;
	unsigned int lba_capacity;
	unsigned short dma_lword;
	unsigned short dma_mword;
	unsigned short eide_pio_modes;
	unsigned short eide_dma_min;
	unsigned short eide_dma_time;
	unsigned short eide_pio;
	unsigned short eide_pio_iordy;
	unsigned short word69;
	unsigned short word70;
	unsigned short word71;
	unsigned short word72;
	unsigned short word73;
	unsigned short word74;
	unsigned short word75;
	unsigned short word76;
	unsigned short word77;
	unsigned short word78;
	unsigned short word79;
	unsigned short word80;
	unsigned short word81;
	unsigned short command_sets;
	unsigned short word83;
	unsigned short word84;
	unsigned short word85;
	unsigned short word86;
	unsigned short word87;
	unsigned short dma_ultra;
	unsigned short word89;
	unsigned short word90;
	unsigned short word91;
	unsigned short word92;
	unsigned short word93;
	unsigned short word94;
	unsigned short word95;
	unsigned short word96;
	unsigned short word97;
	unsigned short word98;
	unsigned short word99;
	unsigned short word100;
	unsigned short word101;
	unsigned short word102;
	unsigned short word103;
	unsigned short word104;
	unsigned short word105;
	unsigned short word106;
	unsigned short word107;
	unsigned short word108;
	unsigned short word109;
	unsigned short word110;
	unsigned short word111;
	unsigned short word112;
	unsigned short word113;
	unsigned short word114;
	unsigned short word115;
	unsigned short word116;
	unsigned short word117;
	unsigned short word118;
	unsigned short word119;
	unsigned short word120;
	unsigned short word121;
	unsigned short word122;
	unsigned short word123;
	unsigned short word124;
	unsigned short word125;
	unsigned short word126;
	unsigned short word127;
	unsigned short security;
	unsigned short reserved[127];
} id_first[1];


#endif //__HDREG_H__