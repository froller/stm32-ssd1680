/*
 * SSD1680.h
 *
 *  Created on: Jan 30, 2023
 *      Author: Alexander Frolov <alex.froller@gmail.com>
 */

#ifndef INC_SSD1680_H_
#define INC_SSD1680_H_

#include "stm32f1xx_hal.h"
#include "fonts.h"

#define SSD1680_GATE_SCAN 0x01
#define SSD1680_GATE_VOLTAGE 0x03
#define SSD1680_SOURCE_VOLTAGE 0x04
#define SSD1680_BOOSTER_SOFT_START 0x0C
#define SSD1680_GATE_SCAN_START 0x0F
#define SSD1680_DATA_ENTRY_MODE 0x11
#define SSD1680_SW_RESET 0x12
#define SSD1680_SELECT_TEMP_SENSOR 0x18
#define SSD1680_READ_TEMP 0x1B
#define SSD1680_MASTER_ACTIVATION 0x20
#define SSD1680_UPDATE_CONTROL_1 0x21
#define SSD1680_UPDATE_CONTROL_2 0x22
#define SSD1680_WRITE_BLACK 0x24
#define SSD1680_WRITE_RED 0x26
#define SSD1680_READ 0x27
#define SSD1680_VCOM_VOLTAGE 0x2C
#define SSD1680_READ_USER_ID 0x2E
#define SSD1680_BORDER 0x3C
#define SSD1680_RAM_READ_OPT 0x41
#define SSD1680_RAM_X_RANGE 0x44
#define SSD1680_RAM_Y_RANGE 0x45
#define SSD1680_PATTERN_RED 0x46
#define SSD1680_PATTERN_BLACK 0x47
#define SSD1680_RAM_X 0x4E
#define SSD1680_RAM_Y 0x4F
#define SSD1680_NOP 0x7F

enum SSD1680_Color {
  ColorBlack = 0,
  ColorWhite,
  ColorRed,
  ColorAnotherRed
};

enum SSD1680_Pattern {
  Pattern8 = 0,
  Pattern16,
  Pattern32,
  Pattern64,
  Pattern128,
  Pattern256,
  PatternSolid = 7
};

enum SSD1680_ScanMode {
  WideScan = 0,
  NarrowScan
};

enum SSD1680_RAMBank {
  RAMBlack = 0,
  RAMRed
};

enum SSD1680_DataEntryMode {
  LeftThenUp = 0,
  RightThenUp,
  LeftThenDown,
  RightThenDown,
  UpThenLeft,
  UpThenRight,
  DownThenLeft,
  DownThenRight
};

/**
 * @struct SSD1680_HandleTypeDef
 * SSD1680 handle
 */
typedef struct {
  SPI_HandleTypeDef *SPI_Handle;	/**< SPI handle */
  uint32_t SPI_Timeout;				/**< SPI timeout in ms */
  GPIO_TypeDef *CS_Port;			/**< CS signal GPIO port */
  uint16_t CS_Pin;					/**< CS signal pin number */
  GPIO_TypeDef *DC_Port;			/**< DC signal GPIO port */
  uint16_t DC_Pin;					/**< DC signal pin number */
  GPIO_TypeDef *RESET_Port;			/**< RESET signal GPIO port */
  uint16_t RESET_Pin;				/**< RESET signal pin number */
  GPIO_TypeDef *BUSY_Port;			/**< BUSY signal GPIO port */
  uint16_t BUSY_Pin;				/**< BUSY signal pin number */
  uint8_t Color_Depth;				/**< Color depth. Either 1 or 2 bits. */
  enum SSD1680_ScanMode Scan_Mode;	/**< @brief Source scan mode.
  	  	  	  	  	  	  				Smaller displays like 152x152 uses narrow scan. @see https://v4.cecdn.yun300.cn/100001_1909185147/SSD1680.pdf page 25. */
  uint8_t Resolution_X;				/**< Hirizontal resolution. Must be a multiple of 8. */
  uint16_t Resolution_Y;			/**< Vertical resolution */
  /** @internal */
#if defined(DEBUG)
  GPIO_TypeDef *LED_Port;			/**< Activity LED GPIO port. Safe to set to NULL. */
  uint16_t LED_Pin;					/**< Activity LED pin number */
#endif // DEBUG
  /** @endinternal */
} SSD1680_HandleTypeDef;

// Connectivity
void SSD1680_Reset(SSD1680_HandleTypeDef *hepd);
void SSD1680_Init(SSD1680_HandleTypeDef *hepd);
void SSD1680_Wait(SSD1680_HandleTypeDef *hepd);
// Low level functions
HAL_StatusTypeDef SSD1680_Send(SSD1680_HandleTypeDef *hepd, const uint8_t addr, const uint8_t *pData, const size_t size);
HAL_StatusTypeDef SSD1680_Receive(SSD1680_HandleTypeDef *hepd, const uint8_t addr, uint8_t *pData, const size_t size);
HAL_StatusTypeDef SSD1680_Clear(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Color color);
HAL_StatusTypeDef SSD1680_Checker(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_RAMFill(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Pattern kx, const enum SSD1680_Pattern ky, const enum SSD1680_Pattern rx, const enum SSD1680_Pattern ry, const enum SSD1680_Color color);
HAL_StatusTypeDef SSD1680_Refresh(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_DataEntryMode(SSD1680_HandleTypeDef *hepd, const enum SSD1680_DataEntryMode mode);
HAL_StatusTypeDef SSD1680_GateScanRange(SSD1680_HandleTypeDef *hepd, const uint16_t top, const uint16_t height);
HAL_StatusTypeDef SSD1680_RAMXRange(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint8_t width);
HAL_StatusTypeDef SSD1680_RAMYRange(SSD1680_HandleTypeDef *hepd, const uint16_t top, const uint16_t height);
HAL_StatusTypeDef SSD1680_UpdateControl1(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_UpdateControl2(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_Border(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Color color);
HAL_StatusTypeDef SSD1680_RAMReadOption(SSD1680_HandleTypeDef *hepd, const enum SSD1680_RAMBank ram);
uint16_t SSD1680_ReadTemp(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_ResetRange(SSD1680_HandleTypeDef *hepd);
HAL_StatusTypeDef SSD1680_StartAddress(SSD1680_HandleTypeDef *hepd, const uint8_t x, const uint16_t y);
// High level functions
HAL_StatusTypeDef SSD1680_GetRegion(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const uint8_t width, const uint16_t height, uint8_t *data_k, uint8_t *data_r);
HAL_StatusTypeDef SSD1680_SetRegion(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const uint8_t width, const uint16_t height, const uint8_t *data_k, const uint8_t *data_r);
HAL_StatusTypeDef SSD1680_Text(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const char *string, const SSD1680_FontTypeDef *font);
HAL_StatusTypeDef SSD1680_VerticalText(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const char *string, const SSD1680_FontTypeDef *font);
#endif // INC_SSD1680_H_
