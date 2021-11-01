#ifdef AMFFMPEG
#include <stdint.h>
extern uint8_t ff_reverse[256];
#else
#ifndef __ANDROID__
#include "libavutil/reverse.c"
#endif
#endif