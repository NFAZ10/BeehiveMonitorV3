#ifndef dash_webapge_h
#define dash_webpage_h

#include <Arduino.h>

#ifndef DASH_USE_BROTLI_COMPRESSION
    #define DASH_USE_BROTLI_COMPRESSION 0
#endif

#if DASH_USE_BROTLI_COMPRESSION == 1
  extern const uint8_t DASH_HTML[112967];
#else
  extern const uint8_t DASH_HTML[188958];
#endif

#endif
