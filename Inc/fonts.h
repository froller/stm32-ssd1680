#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

/**
 * @struct SSD1680_FontTypeDef
 * Font description structure
 */
typedef struct {
  const uint8_t width;	/**< Font width in pixels */
  const uint8_t height;	/**< Font height in pixels */
  const uint8_t data[];	/**< Font data. Expected to be 256 * width / 8 * height bytes */
} SSD1680_FontTypeDef;

extern const SSD1680_FontTypeDef cp866_8x8;		/**< 8x8 font, CP866, regular orientation */
extern const SSD1680_FontTypeDef cp866_8x8_r;	/**< 8x8 font, CP866, right orientation */
extern const SSD1680_FontTypeDef cp866_8x14;	/**< 8x14 font, CP866, regular orientation, deprecated */
extern const SSD1680_FontTypeDef cp866_8x16;	/**< 8x16 font, CP866, regular orientation */
extern const SSD1680_FontTypeDef cp866_8x16_r;	/**< 8x16 font, CP866, right orientation */

#endif // __FONTS_H__
