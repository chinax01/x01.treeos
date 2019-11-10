#ifndef __STDDEF_H__
#define __STDDEF_H__

#ifndef	_PTRDIFF_T
#define _PTRDIFF_T
typedef long ptrdiff_t;
#endif 

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long size_t;
#endif 

#undef NULL
#define NULL 	( (void*)0 )

#define offsetof(TYPE, MEMBER)	((size_t) &((TYPE*)0)))

#endif //__STDDEF_H__