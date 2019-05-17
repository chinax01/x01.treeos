#ifndef _CONFIG_H
#define _CONFIG_H

#define INSTALL_START_SECT		0x8000
#define INSTALL_NR_SECTS		0x800

#define BOOT_PARAM_ADDR			0x900
#define BOOT_PARAM_MAGIC		0xB007
#define BI_MAG			0
#define BI_MEM_SIZE		1
#define BI_KERNEL_FILE	2

#define MINOR_BOOT		MINOR_hd2a

#endif //_CONFIG_H