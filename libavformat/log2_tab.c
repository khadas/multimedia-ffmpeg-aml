#ifdef AMFFMPEG
#include <stdint.h>
extern const uint8_t ff_log2_tab[256];
#else
#ifndef __ANDROID__
#include "libavutil/log2_tab.c"
#endif
#endif