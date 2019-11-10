/***************************************************
*		版权声明
*
*	本操作系统名为：MINE
*	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
*	只允许个人学习以及公开交流使用
*
*	代码最终所有权及解释权归田宇所有；
*
*	本模块作者：	田宇
*	EMail:		345538255@qq.com
*
*
***************************************************/

#ifndef __STRING_H__

#define __STRING_H__

/*
		From => To memory copy Num bytes
*/

extern void * memcpy(void *From,void * To,long Num);


/*
		FirstPart = SecondPart		=>	 0
		FirstPart > SecondPart		=>	 1
		FirstPart < SecondPart		=>	-1
*/

extern int memcmp(void * FirstPart,void * SecondPart,long Count);

/*
		set memory at Address with C ,number is Count
*/

extern void * memset(void * Address,unsigned char C,long Count);

/*
		string copy
*/

extern char * strcpy(char * Dest,char * Src);


/*
		string copy number bytes
*/

extern char * strncpy(char * Dest,char * Src,long Count);


/*
		string cat Dest + Src
*/

extern char * strcat(char * Dest,char * Src);


/*
		string compare FirstPart and SecondPart
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/

extern int strcmp(char * FirstPart,char * SecondPart);


/*
		string compare FirstPart and SecondPart with Count Bytes
		FirstPart = SecondPart =>  0
		FirstPart > SecondPart =>  1
		FirstPart < SecondPart => -1
*/

extern int strncmp(char * FirstPart,char * SecondPart,long Count);
/*

*/

extern int strlen(char * String);


#endif
