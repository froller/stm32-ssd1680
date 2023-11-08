# stm32-ssd1680

STM32 HAL-based library for SSD1680 e-paper display

## Synopsys

```C

#include "stm32f1xx_hal.h"
#include "../../SSD1680/Inc/SSD1680.h"

SPI_HandleTypeDef hspi1;
SSD1680_HandleTypeDef hepd;

static void MX_SSD1680_Init(void); 

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SSD1680_Init();

  SSD1680_Border(hepd, ColorWhite);
  SSD1680_Clear(hepd, ColorWhite);

  uint8_t scale[176 / 8 * 4 ] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  SSD1680_SetRegion(hepd, 0, 0, 176, 4, scale, NULL);

  SSD1680_Text(hepd, 0, 16, "Font 8x8", &cp866_8x8);
  SSD1680_Text(hepd, 0, 24, "Font 8x14", &cp866_8x14);
  SSD1680_Text(hepd, 0, 38, "Font 8x16", &cp866_8x16);
}

static void MX_SSD1680_Init(void) {
  hepd.SPI_Handle = &hspi1;
  hepd.CS_Port = DISP_CS_GPIO_Port;
  hepd.CS_Pin = DISP_CS_Pin;
  hepd.DC_Port = DISP_DC_GPIO_Port;
  hepd.DC_Pin = DISP_DC_Pin;
  hepd.RESET_Port = DISP_RST_GPIO_Port;
  hepd.RESET_Pin = DISP_RST_Pin;
  hepd.BUSY_Port = DISP_BUSY_GPIO_Port;
  hepd.BUSY_Pin = DISP_BUSY_Pin;
  hepd.Color_Depth = SSD1680_COLOR_DEPTH;
  hepd.Scan_Mode = SSD1680_X_SCAN_MODE;
  hepd.Resolution_X = 176;
  hepd.Resolution_Y = 264;
  SSD1680_Init(&hepd);
}
```

## See Also

- https://v4.cecdn.yun300.cn/100001_1909185147/SSD1680.pdf

