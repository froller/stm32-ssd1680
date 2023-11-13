/* vim: set ai et ts=4 sw=4: */
#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
  const uint8_t width;
  const uint8_t height;
  const uint8_t data[];
} SSD1680_FontTypeDef;

extern const SSD1680_FontTypeDef cp866_8x8;
extern const SSD1680_FontTypeDef cp866_8x8_r;
extern const SSD1680_FontTypeDef cp866_8x14;
extern const SSD1680_FontTypeDef cp866_8x16;
extern const SSD1680_FontTypeDef cp866_8x16_r;

#endif // __FONTS_H__
