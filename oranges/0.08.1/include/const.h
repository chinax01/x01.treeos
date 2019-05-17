#ifndef _CONST_H
#define _CONST_H

// hd
#define DIOCTL_GET_GEO		1

#define SECTOR_SIZE			512
#define SECTOR_BITS			(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9

#define NO_DEV			0
#define DEV_FLOPPY		1
#define DEV_CDROM		2
#define DEV_HD			3
#define DEV_CHAR_TTY	4
#define DEV_SCSI		5

#define MAJOR_SHIFT		8
#define MAKE_DEV(a,b)	( ((a) << MAJOR_SHIFT) | (b) )
#define MAJOR(x)		( ((x) >> MAJOR_SHIFT) & 0xFF )
#define MINOR(x)		( (x) & 0xFF )

#define MINOR_hd1a		0x10
#define MINOR_hd2a		0x20
#define MINOR_hd2b		0x21
#define MINOR_hd3a		0x30
#define MINOR_hd4a		0x40

#define MINOR_BOOT		0x21
#define ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)

#define INVALID_INODE	0
#define ROOT_INODE		1

#define MAX_DRIVES			2
#define NR_PART_PER_DRIVE	4
#define NR_SUB_PER_PART		16
#define NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

#define MAX_PRIM			(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)
#define MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

#define P_PRIMARY	0
#define P_EXTENDED	1

#define ORANGES_PART	0x99
#define NO_PART			0x00
#define EXT_PART		0x05

#define NR_FILES		64
#define NR_FILE_DESC	64
#define NR_INODE		64
#define NR_SUPER_BLOCK	8

#define I_TYPE_MASK		0170000
#define I_REGULAR		0100000
#define I_BLOCK_SPECIAL	0060000
#define I_DIRECTORY		0040000
#define I_CHAR_SPECIAL	0020000
#define I_NAMED_PIPE	0010000

#define is_special(m)	( (((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) ||  \
	(((m) & I_TYPE_MASK) == I_CHAR_SPECIAL) )

#define NR_DEFAULT_FILE_SECTS	2048
	
// message
enum MessageType {
	HARD_INT = 1,
	GET_TICKS,

	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL
};

#define SEND		1
#define RECEIVE 	2
#define BOTH		3

#define SENDING		0x02
#define RECEIVING	0x04

#define CNT 		u.m3.m3i2
#define REQUEST		u.m3.m3i2
#define PROC_NR		u.m3.m3i3
#define DEVICE		u.m3.m3i4
#define POSITION	u.m3.m3l1
#define BUF			u.m3.m3p2
#define RETVAL		u.m3.m3i1

// misc
#define FALSE	0
#define TRUE	1

#define max(a,b)	( (a) > (b) ? (a) : (b) )
#define min(a,b)	( (a) < (b) ? (a) : (b) )

#define vir2phys(segbase, vir)	( ((unsigned int)(segbase)) + ((unsigned int)(vir)) )
#define proc2pid(p)		((p) - proc_table)

#define ASSERT
#ifdef ASSERT
void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp)		if (exp); \
	else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else 
#define assert(exp)	
#endif 

#define MAG_CH_PANIC	'\002'
#define MAG_CH_ASSERT	'\003'

#define STR_DEFAULT_LEN		1024

// pm
#define GDT_SIZE	128
#define IDT_SIZE 	256

#define	INDEX_DUMMY		0	// ┓
#define	INDEX_FLAT_C	1	// ┣ LOADER 里面已经确定了的.
#define	INDEX_FLAT_RW	2	// ┃
#define	INDEX_VIDEO		3	// ┛
#define INDEX_TSS		4
#define INDEX_LDT_FIRST	5

#define LDT_SIZE	2

#define INDEX_LDT_C		0
#define INDEX_LDT_RW 	1

/* 选择子 */
#define	SELECTOR_DUMMY		   0		// ┓
#define	SELECTOR_FLAT_C		0x08		// ┣ LOADER 里面已经确定了的.
#define	SELECTOR_FLAT_RW	0x10		// ┃
#define	SELECTOR_VIDEO		(0x18+3)	// ┛<-- RPL=3
#define SELECTOR_TSS		0x20
#define SELECTOR_LDT_FIRST	0x28

#define	SELECTOR_KERNEL_CS	SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW
#define SELECTOR_KERNEL_GS 	SELECTOR_VIDEO

// Selector Attributes
#define SA_RPL_MASK		0xFFFC
#define SA_RPL0			0
#define SA_RPL1			1
#define SA_RPL2			2
#define SA_RPL3			3

#define SA_TI_MASK		0xFFFB
#define SA_TIG			0
#define SA_TIL			4

#define RPL_KRNL		SA_RPL0
#define RPL_TASK		SA_RPL1
#define RPL_USER		SA_RPL3

#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 中断向量 */
#define INT_M_CTL		0x20
#define INT_M_CTLMASK	0x21
#define INT_S_CTL		0xA0
#define INT_S_CTLMASK	0xA1

#define EOI		0x20

#define INT_VECTOR_IRQ0		0x20
#define INT_VECTOR_IRQ8		0x28

#define INT_VECTOR_SYS_CALL	0x90

#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT	0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT			0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

#define TIMER0		0x40
#define TIMER_MODE	0x43
#define RATE_GENERATOR	0x34
#define TIMER_FREQ		1193182L
#define HZ				100

#define NR_IRQ		16

#define	CLOCK_IRQ		0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ		2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ		3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ		4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ		5	/* xt winchester */
#define	FLOPPY_IRQ		6	/* floppy disk */
#define	PRINTER_IRQ		7
#define	AT_WINI_IRQ		14	/* at winchester */

#define NR_SYSCALL	2

#define INVALID_DRIVER	-20
#define INTERRUPT 		-10
#define TASK_TTY		0
#define TASK_SYS		1
#define TASK_HD			2
#define TASK_FS			3
#define TASK_MM			4
#define ANY				(NR_TASKS + NR_PROCS + 10)
#define NO_TASK			(NR_TASKS + NR_PROCS + 20)

// keyboard
#define KB_DATA		0x60
#define KB_CMD		0x64

#define NR_CONSOLES		3
#define TTY_IN_BYTES	256

// tty
#define CRTC_ADDR_REG	0x3D4
#define CRTC_DATA_REG	0x3D5
#define START_ADDR_H	0xC
#define START_ADDR_L	0xD
#define CURSOR_H		0xE
#define CURSOR_L		0xF
#define V_MEM_BASE		0xB8000
#define V_MEM_SIZE		0x8000

#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define MAKE_COLOR(x,y) (x | y) /* MAKE_COLOR(Background,Foreground) */

#define DEFAULT_CHAR_COLOR 		(MAKE_COLOR(BLACK, WHITE))
#define GRAY_CHAR				(MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR				(MAKE_COLOR(BLUE, RED) | BRIGHT)

#define SCR_UP		1
#define SCR_DN		-1

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH	80

#define LED_CODE	0xED
#define KB_ACK		0xFA

// proc
#define NR_TASKS	4
#define NR_PROCS	3
#define FIRST_PROC	proc_table[0]
#define LAST_PROC	proc_table[NR_TASKS+NR_PROCS-1]

#define STACK_SIZE_TESTA	0x8000
#define STACK_SIZE_TESTB	0x8000
#define STACK_SIZE_TESTC	0x8000
#define STACK_SIZE_TTY		0x8000
#define STACK_SIZE_SYS		0x8000
#define STACK_SIZE_HD		0x8000
#define STACK_SIZE_FS		0x8000

#define STACK_SIZE_TOTAL	(STACK_SIZE_TESTA + STACK_SIZE_TESTB + STACK_SIZE_TESTC + \
	STACK_SIZE_TTY + STACK_SIZE_SYS + STACK_SIZE_HD + STACK_SIZE_FS)

#endif 