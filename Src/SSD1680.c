/*
 * SSD1680.h
 *
 *  Created on: 30 янв. 2023 г.
 *      Author: froller
 */

#include "../Inc/SSD1680.h"
#include <stdlib.h>
#include <string.h>

void SSD1680_Reset(SSD1680_HandleTypeDef *hepd) {
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(10);
}

void SSD1680_Init(SSD1680_HandleTypeDef *hepd) {
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(20);

  /*
   _START_SEQUENCE = (
      b"\x12\x80\x14"  # soft reset and wait 20ms
      b"\x11\x01\x03"  # Ram data entry mode
      b"\x3C\x01\x05"  # border color
      b"\x2c\x01\x36"  # Set vcom voltage
      b"\x03\x01\x17"  # Set gate voltage
      b"\x04\x03\x41\x00\x32"  # Set source voltage
      b"\x4e\x01\x01"  # ram x count
      b"\x4f\x02\x00\x00"  # ram y count
      b"\x01\x03\x00\x00\x00"  # set display size
      b"\x22\x01\xf4"  # display update mode
  )
  */

  /***** #2 *****/
  SSD1680_Reset(hepd);
  SSD1680_Send(hepd, SSD1680_SW_RESET, 0, 0);
  SSD1680_Wait(hepd);

  uint8_t userId[10] = { 0 };
  SSD1680_Receive(hepd, SSD1680_READ_USER_ID, userId, sizeof(userId)); // 0x2E

  /***** #3 *****/

  //const uint8_t gateVoltage[] = { 0x00 };
  //SSD1680_Send(&hepd, SSD1680_GATE_VOLTAGE, gateVoltage, sizeof(gateVoltage));  // 0x03
  //const uint8_t sourceVoltage[] = { 0x41, 0x48, 0x32 };
  //SSD1680_Send(&hepd, SSD1680_SOURCE_VOLTAGE, sourceVoltage, sizeof(sourceVoltage));  // 0x04
  //const uint8_t vcomVoltage[] = { 0x38 };
  //SSD1680_Send(&hepd, SSD1680_VCOM_VOLTAGE, vcomVoltage, sizeof(vcomVoltage));  // 0x2C
  SSD1680_GateScanRange(hepd, 0, hepd->Resolution_Y);
  SSD1680_UpdateControl1(hepd);
  SSD1680_UpdateControl2(hepd);

  /***** #4 *****/
  SSD1680_DataEntryMode(hepd, RightThenDown);

  /***** #5 *****/
  const uint8_t tempSensor[] = { 0x80 };
  SSD1680_Send(hepd, SSD1680_SELECT_TEMP_SENSOR, tempSensor, sizeof(tempSensor));    // 0x18

#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif // DEBUG
}

void SSD1680_Wait(SSD1680_HandleTypeDef *hepd) {
  while (HAL_GPIO_ReadPin(hepd->BUSY_Port, hepd->BUSY_Pin) == GPIO_PIN_SET)
    HAL_Delay(2);
}

HAL_StatusTypeDef SSD1680_Send(SSD1680_HandleTypeDef *hepd, const uint8_t addr, const uint8_t *pData, const size_t size) {
  HAL_StatusTypeDef status = HAL_OK;
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_RESET);
  if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)&addr, sizeof(addr), SSD1680_TIMEOUT))) {
    HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
    if (hepd->LED_Port)
      HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
    return status;
  }
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
  if (size > 0) {
    status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)pData, size, SSD1680_TIMEOUT);
  }
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
  return status;
}

HAL_StatusTypeDef SSD1680_Receive(SSD1680_HandleTypeDef *hepd, const uint8_t addr, uint8_t *pData, const size_t size) {
  HAL_StatusTypeDef status = HAL_OK;
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_RESET);
  if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)&addr, sizeof(addr), SSD1680_TIMEOUT))) {
    HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
    if (hepd->LED_Port)
      HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
    return status;
  }
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
  if (size > 0) {
    status = HAL_SPI_Receive(hepd->SPI_Handle, pData, size, SSD1680_TIMEOUT);
  }
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
  return status;
}

HAL_StatusTypeDef SSD1680_Clear(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Color color) {
  return SSD1680_RAMFill(hepd, PatternSolid, PatternSolid, PatternSolid, PatternSolid, color);
}

HAL_StatusTypeDef SSD1680_Checker(SSD1680_HandleTypeDef *hepd) {
  return SSD1680_RAMFill(hepd, Pattern16, Pattern16, Pattern8, Pattern8, ColorAnotherRed);
}

HAL_StatusTypeDef SSD1680_RAMFill(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Pattern kx, const enum SSD1680_Pattern ky, const enum SSD1680_Pattern rx, const enum SSD1680_Pattern ry, const enum SSD1680_Color color) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_RAMXRange(hepd, 0, hepd->Resolution_X)))
    return status;
  if ((status = SSD1680_RAMYRange(hepd, 0, hepd->Resolution_Y)))
    return status;
  uint8_t pattern = (ky << 4) | kx | ((color & 1) << 7);
  if ((status = SSD1680_Send(hepd, SSD1680_PATTERN_BLACK, &pattern, sizeof(pattern))))  // 0x47
    return status;
  SSD1680_Wait(hepd);
  pattern = (ry << 4) | rx | ((color & 2) << 6);
  if ((status = SSD1680_Send(hepd, SSD1680_PATTERN_RED, &pattern, sizeof(pattern))))  // 0x46
    return status;
  SSD1680_Wait(hepd);
  return status;
}

HAL_StatusTypeDef SSD1680_Refresh(SSD1680_HandleTypeDef *hepd) {
  HAL_StatusTypeDef status = HAL_OK;
  const uint8_t boosterSoftStart[] = { 0x80, 0x90, 0x90, 0x00 };
  if ((status = SSD1680_Send(hepd, SSD1680_BOOSTER_SOFT_START, boosterSoftStart, sizeof(boosterSoftStart))))    // 0x0C
    return status;
  if ((status = SSD1680_UpdateControl2(hepd))) // 0x22
    return status;
  if ((status = SSD1680_Send(hepd, SSD1680_MASTER_ACTIVATION, 0, 0)))   // 0x20
    return status;
  SSD1680_Wait(hepd);
  return status;
}

HAL_StatusTypeDef SSD1680_DataEntryMode(SSD1680_HandleTypeDef *hepd, const enum SSD1680_DataEntryMode mode) {
  return SSD1680_Send(hepd, SSD1680_DATA_ENTRY_MODE, &mode, sizeof(mode));   // 0x11
}

HAL_StatusTypeDef SSD1680_GateScanRange(SSD1680_HandleTypeDef *hepd, const uint16_t top, const uint16_t height) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_Send(hepd, SSD1680_GATE_SCAN_START, (uint8_t *)&top, sizeof(top))))  // 0x0F
    return status;
#pragma pack(push, 1)
  const struct {
    uint16_t height;
    uint8_t order;
  } gateScan = { height, 0 };
#pragma pack(pop)
  return SSD1680_Send(hepd, SSD1680_GATE_SCAN, (uint8_t *)&gateScan, sizeof(gateScan));     // 0x01
}

HAL_StatusTypeDef SSD1680_RAMXRange(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint8_t width) {
  if (left % 8 + width % 8)
    return HAL_ERROR;
  const uint8_t ramXRange[] = { left / 8, (left + width) / 8 - 1};
  return SSD1680_Send(hepd, SSD1680_RAM_X_RANGE, ramXRange, sizeof(ramXRange));   // 0x44
}

HAL_StatusTypeDef SSD1680_RAMYRange(SSD1680_HandleTypeDef *hepd, const uint16_t top, const uint16_t height) {
  const uint16_t ramYRange[] = { top, (top + height) - 1};
  return SSD1680_Send(hepd, SSD1680_RAM_Y_RANGE, (uint8_t *)ramYRange, sizeof(ramYRange));   // 0x45
}

HAL_StatusTypeDef SSD1680_UpdateControl1(SSD1680_HandleTypeDef *hepd) {
  const uint8_t inverseR = 0;
  const uint8_t bypassR = hepd->Color_Depth & 0x01;
  const uint8_t inverseK = 0;
  const uint8_t bypassK = 0;
#pragma pack(push, 1)
  const struct {
    // LSB
    uint8_t reserved1:2;
    uint8_t bypassK:1;
    uint8_t inverseK:1;
    uint8_t reserved2:2;
    uint8_t bypassR:1;
    uint8_t inverseR:1;
    // MSB
    uint8_t reserved0:7;
    uint8_t sourceMode:1;
  } updateControl1 = { 0, bypassK, inverseK, 0, bypassR, inverseR, 0, hepd->Scan_Mode };
#pragma pack(pop)
  return SSD1680_Send(hepd, SSD1680_UPDATE_CONTROL_1, (uint8_t *)&updateControl1, sizeof(updateControl1));  // 0x21
}

HAL_StatusTypeDef SSD1680_UpdateControl2(SSD1680_HandleTypeDef *hepd) {
  const uint8_t updateControl2[] = { 0xF7 }; // See datasheet page 26
  return SSD1680_Send(hepd, SSD1680_UPDATE_CONTROL_2, updateControl2, sizeof(updateControl2));  // 0x22
}

HAL_StatusTypeDef SSD1680_Border(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Color color) {
#pragma pack(push, 1)
  const struct {
    uint8_t LUT:2;
    uint8_t transition:1;
    uint8_t reserved:1;
    uint8_t level:2;
    uint8_t mode:2;
  } border = { color, 1, 0, 0, 0 };
#pragma pack(pop)
  return SSD1680_Send(hepd, SSD1680_BORDER, (uint8_t *)&border, sizeof(border));
}

HAL_StatusTypeDef SSD1680_RAMReadOption(SSD1680_HandleTypeDef *hepd, const enum SSD1680_RAMBank ram) {
  return SSD1680_Send(hepd, SSD1680_RAM_READ_OPT, (uint8_t *)&ram, 1);
}

uint16_t SSD1680_ReadTemp(SSD1680_HandleTypeDef *hepd) {
  const uint8_t tempSensor[] = { 0x80 };
  SSD1680_Send(hepd, SSD1680_SELECT_TEMP_SENSOR, tempSensor, sizeof(tempSensor));   // 0x18
  uint16_t temp;
  SSD1680_Receive(hepd, SSD1680_READ_TEMP, (uint8_t *)&temp, sizeof(temp));         // 0x1b
  return temp >> 4;
}

HAL_StatusTypeDef SSD1680_ResetRange(SSD1680_HandleTypeDef *hepd) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_RAMXRange(hepd, 0, hepd->Resolution_X / 8)))
    return status;
  if ((status = SSD1680_RAMYRange(hepd, 0, hepd->Resolution_Y)))
    return status;
  return status;
}

HAL_StatusTypeDef SSD1680_StartAddress(SSD1680_HandleTypeDef *hepd, const uint8_t x, const uint16_t y) {
  if (x % 8)
    return HAL_ERROR;
  HAL_StatusTypeDef status = HAL_OK;
  const uint8_t xaddr = x / 8;
  if ((status = SSD1680_Send(hepd, SSD1680_RAM_X, &xaddr, sizeof(xaddr))))   // 0x4E
    return status;
  if ((status = SSD1680_Send(hepd, SSD1680_RAM_Y, (uint8_t *)&y, sizeof(y))))   // 0x4F
    return status;
  return status;
}

HAL_StatusTypeDef SSD1680_GetRegion(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const uint8_t width, const uint16_t height, uint8_t *data_k, uint8_t *data_r) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_RAMXRange(hepd, left, width)))
    return status;
  if ((status = SSD1680_RAMYRange(hepd, top, height)))
    return status;

  if (data_k) {
    if ((status = SSD1680_RAMReadOption(hepd, RAMBlack)))
      return status;
    if ((status = SSD1680_StartAddress(hepd, left, top)))
      return status;
    // Sending Read RAM command
    HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_RESET);
    uint8_t cmd = SSD1680_READ;
#if defined(DEBUG)
    if (hepd->LED_Port)
      HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_RESET);
#endif // DEBUG
    if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, &cmd, 1, SSD1680_TIMEOUT))) {
      HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
      goto exit_GetRegion;
    }
    HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
    // Reading data
    uint8_t *pData = data_k;
#define SSD1680_DUMMY_BYTES 2
#if SSD1680_DUMMY_BYTES
      // Reading dummy bytes
      if ((status = HAL_SPI_Receive(hepd->SPI_Handle, pData, SSD1680_DUMMY_BYTES, SSD1680_TIMEOUT)))
        goto exit_GetRegion;
#endif // SSD1680_DUMMY_BYTES
      if ((status = HAL_SPI_Receive(hepd->SPI_Handle, pData, width / 8 * height, SSD1680_TIMEOUT)))
        goto exit_GetRegion;
  }
exit_GetRegion:
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif // DEBUG
  return status;

}

HAL_StatusTypeDef SSD1680_SetRegion(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const uint8_t width, const uint16_t height, const uint8_t *data_k, const uint8_t *data_r) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_RAMXRange(hepd, left, width)))
    return status;
  if ((status = SSD1680_RAMYRange(hepd, top, height)))
    return status;

  if (data_k) {
    if ((status = SSD1680_StartAddress(hepd, left, top)))
      return status;
    if ((status = SSD1680_Send(hepd, SSD1680_WRITE_BLACK, data_k, width / 8 * height))) // 0x24
      return status;
  }
  if (data_r) {
    if ((status = SSD1680_StartAddress(hepd, left, top)))
      return status;
    if ((status = SSD1680_Send(hepd, SSD1680_WRITE_RED, data_r, width / 8 * height))) // 0x26
      return status;
  }
  return status;
}

HAL_StatusTypeDef SSD1680_Text(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const char *string, const SSD1680_FontTypeDef *font) {
  const uint8_t tab_width = 4;
  const uint8_t glyphSize = font->width / 8 * font->height;
  const size_t len = strlen(string);
  uint8_t pos_x = 0;
  uint8_t pos_y = 0;
  for (size_t i = 0; i < len; ++i) {
    switch (string[i]) {
    case 0x08:    // backspace
      if (pos_x)
        --pos_x;
      break;
    case 0x09:    // tab
      pos_x = (pos_x / tab_width + 1) * tab_width;
      break;
    case 0x0A:    // line feed
      ++pos_y;
      pos_x = 0;
      break;
    case 0x0D:    // carriage return
      pos_x = 0;
      break;
    default:
      {
        HAL_StatusTypeDef status = HAL_OK;
        uint8_t buffer[glyphSize];
        memcpy(buffer, font->data + ((unsigned char)string[i] * glyphSize), sizeof(buffer));
        for (uint8_t j = 0; j < sizeof(buffer); ++j)
          buffer[j] = ~buffer[j];
        if ((status = SSD1680_SetRegion(hepd, left + font->width * pos_x, top + font->height * pos_y, font->width, font->height, buffer, NULL)))
          return status;
        ++pos_x;
      }
    }
  }
  return HAL_OK;
}

HAL_StatusTypeDef SSD1680_VerticalText(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint16_t top, const char *string, const SSD1680_FontTypeDef *font) {
  const uint8_t tab_width = 4;
  const uint8_t glyphSize = font->width / 8 * font->height;
  const size_t len = strlen(string);
  uint8_t pos_x = 0;
  uint8_t pos_y = 0;
  for (size_t i = 0; i < len; ++i) {
    switch (string[i]) {
    case 0x08:    // backspace
      if (pos_y)
        --pos_y;
      break;
    case 0x09:    // tab
      pos_y = (pos_y / tab_width + 1) * tab_width;
      break;
    case 0x0A:    // line feed
      --pos_x;
      pos_y = 0;
      break;
    case 0x0D:    // carriage return
      pos_y = 0;
      break;
    default:
      {
        HAL_StatusTypeDef status = HAL_OK;
        uint8_t buffer[glyphSize];
        memcpy(buffer, font->data + ((unsigned char)string[i] * glyphSize), sizeof(buffer));
        for (uint8_t j = 0; j < sizeof(buffer); ++j)
          buffer[j] = ~buffer[j];
        if ((status = SSD1680_SetRegion(hepd, left + font->width * pos_x, top + font->height * pos_y, font->width, font->height, buffer, NULL)))
          return status;
        ++pos_y;
      }
    }
  }
  return HAL_OK;
}

void ByteGridTranspose(uint8_t *out, const uint16_t width, const uint16_t height, const uint8_t *in) {
  for (uint16_t y = 0; y < height; ++y)
    for (uint16_t x = 0; x < width; ++x) {
      uint16_t iB = (y * width + x) / 8;
      uint16_t oB = (x * height) / 8 + (height - y - 1) / 8;
      for (uint8_t b = 0; b < 8; ++b)
        out[oB] = out[oB] | (((in[iB] >> (7 - x % 8)) & 1) <<  (0 + y % 8));
    }
}
