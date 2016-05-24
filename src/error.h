#pragma once

#ifdef WIN32

#include "stackwalker/StackWalker.h"
#define error_stop(fmt, ...) \
{\
	StackWalker sw; \
	char* callstack=sw.ShowCallstack(); \
	char text[10000];\
	sprintf(text,("" fmt "\n\nCallstack:\n%s [%s:%d]\n%s"),\
	##__VA_ARGS__,__FUNCTION__,__FILE__,  __LINE__, callstack);\
	printf(text);\
	MessageBoxA(0,text,"Error",0); ::exit(0);\
	while(1);;\
};

#else

#include <stdio.h>

#define error_stop(fmt, ...) \
	{ fprintf(stderr, ("" fmt "\n\nTerminate at: %s [%s:%d]\n"), \
	    ##__VA_ARGS__,__FUNCTION__,__FILE__,  __LINE__); \
    ::exit(1); }

#endif
