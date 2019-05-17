#ifndef _TYPE_H
#define _TYPE_H

struct Descriptor {
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char attr1;
	unsigned char limit_high_attr2;
	unsigned char base_high;
};

struct Gate {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char dcount;
	unsigned char attr;
	unsigned short offset_high;
};

typedef void (*int_handler)();

#endif 