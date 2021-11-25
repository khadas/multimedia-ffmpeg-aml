#ifndef __ANDROID__
#include "libavutil/reverse.c"
#else
#include <stdint.h>
extern uint8_t ff_reverse[256];
#endif
