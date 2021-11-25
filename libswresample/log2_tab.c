#ifndef __ANDROID__
#include "libavutil/log2_tab.c"
#else
#include <stdint.h>
extern const uint8_t ff_log2_tab[256];
#endif
