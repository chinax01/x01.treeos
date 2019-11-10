#ifndef __CONFIG_H__
#define __CONFIG_H__

#define LINUS_HD

#if defined(LINUS_HD)
#define HIGH_MEMORY		(0x800000)
#elif defined(LASU_HD)
#define HIGH_MEMORY		(0x400000)
#else 
#error "must define hd"
#endif 

#if (HIGH_MEMORY >= 0x600000)
#define BUFFER_END	0x200000
#else 
#define BUFFER_END 0xa0000
#endif 

#if defined(LINUS_HD)
#define ROOT_DEV	0x307
#elif defined(LASU_HD)
#define ROOT_DEV	0x302
#else 
#error "must define HD"
#endif 

#if defined(LASU_HD)
#define HD_TYPE		{ 7, 35, 915, 65536, 920, 0  }
#elif defined(LINUS_HD)
#define HD_TYPE		{ 5,17,980,300,980,0 }, { 5,17,980,300,980,0 }
#else 
#error "must define a hard-disk type"
#endif 

#endif //__CONFIG_H__