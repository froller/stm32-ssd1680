/*
 * SSD1680.h
 *
 *  Created on: Jan 13, 2023
 *      Author: Alexander Frolov <alex.froller@gmail.com>
 */

#include "../Inc/SSD1680.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Hardware reset the display
 * @details Ties !RESET line to GND for 2 ms
 * @param[in] hepd: SSD1680 handle pointer
 */
void SSD1680_Reset(SSD1680_HandleTypeDef *hepd) {
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(2);
  HAL_GPIO_WritePin(hepd->RESET_Port, hepd->RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(10);
}

/**
 * @brief Initialize the display
 * @details
 * @li Reset the disply
 * @li Send initialization sequence
 * @li Set source and gate scan ranges to maximum resolution
 * @li Enable internal temperature sensor
 * @li Setup voltage sources
 * @li Set data entry mode to @ref RightThenDown
 * @param[in] hepd: SSD1680 handle pointer
 */
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

/**
 * @brief Wait for display ready
 * @details Waits for BUSY line to become low. Spins with 2ms delay while not yet.
 * @param[in] hepd: SSD1680 handle pointer
 */
void SSD1680_Wait(SSD1680_HandleTypeDef *hepd) {
  while (HAL_GPIO_ReadPin(hepd->BUSY_Port, hepd->BUSY_Pin) == GPIO_PIN_SET)
    HAL_Delay(2);
}

/**
 * @brief Send command and data to the display
 * @details Send 1 or more bytes to the display.
 * First bytes is sent with !DC line pulled low and interpreted as a command (or register).
 * Other bytes are sent with !DC line pushed high and interpreted as arguments (or value).
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] command: command byte
 * @param[in] pData: pointer to the command arguments
 * @param[in] size: size of command arguments
 * @return HAL status
 * @see https://v4.cecdn.yun300.cn/100001_1909185147/SSD1680.pdf page 20 for full command list.
 *
 */
HAL_StatusTypeDef SSD1680_Send(SSD1680_HandleTypeDef *hepd, const uint8_t command, const uint8_t *pData, const size_t size) {
  HAL_StatusTypeDef status = HAL_OK;
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_RESET);
  if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)&command, sizeof(command), hepd->SPI_Timeout))) {
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
    status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)pData, size, hepd->SPI_Timeout);
  }
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
  return status;
}

/**
 * @brief Send command and receive data from the display
 * @details Send 1 to the display then receive data array.
 * First bytes is sent with !DC line pulled low and interpreted as a command (or register).
 * Other bytes are received with !DC line pushed high and interpreted as return value.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] command: command byte.
 * @param[in] pData: pointer to the buffer for return value
 * @param[in] size: size of the buffer in bytes
 * @return HAL status
 * @see https://v4.cecdn.yun300.cn/100001_1909185147/SSD1680.pdf page 20 for full command list.
 */
HAL_StatusTypeDef SSD1680_Receive(SSD1680_HandleTypeDef *hepd, const uint8_t command, uint8_t *pData, const size_t size) {
  HAL_StatusTypeDef status = HAL_OK;
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_RESET);
#endif
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_RESET);
  if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, (uint8_t *)&command, sizeof(command), hepd->SPI_Timeout))) {
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
    status = HAL_SPI_Receive(hepd->SPI_Handle, pData, size, hepd->SPI_Timeout);
  }
  HAL_GPIO_WritePin(hepd->CS_Port, hepd->CS_Pin, GPIO_PIN_SET);
#if defined(DEBUG)
  if (hepd->LED_Port)
    HAL_GPIO_WritePin(hepd->LED_Port, hepd->LED_Pin, GPIO_PIN_SET);
#endif
  return status;
}

/**
 * @brief Clears the screen
 * @details Fills the screen with specified solid color
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] color: color to fill the screen
 * @return HAL status
 * @note Slow. Waits for display ready.
 */
HAL_StatusTypeDef SSD1680_Clear(SSD1680_HandleTypeDef *hepd, const enum SSD1680_Color color) {
  return SSD1680_RAMFill(hepd, PatternSolid, PatternSolid, PatternSolid, PatternSolid, color);
}

/**
 * @brief Shows checker pattern
 * @details Fills the screen with checker pattern 16x16 for primary (black) color and 8x8 for secondary (red) color.
 * @param[in] hepd: SSD1680 handle pointer
 * @return HAL status
 * @note Slow. Waits for display ready.
 * @see SSD1680_RAMFill for custom patterns
 */
HAL_StatusTypeDef SSD1680_Checker(SSD1680_HandleTypeDef *hepd) {
  return SSD1680_RAMFill(hepd, Pattern16, Pattern16, Pattern8, Pattern8, ColorAnotherRed);
}

/**
 * @brief Shows checker pattern
 * @details Fills the screen with checker pattern with specified strides for primary and secondary colors.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] kx: horizontal stride for primary color
 * @param[in] ky: vertical stride for primary color
 * @param[in] rx: horizontal stride for secondary color
 * @param[in] ry: vertical stride for secondary color
 * @param[in] color: color of the top-left pixel
 * @return HAL status
 * @note Slow. Waits for display ready.
 * @see SSD1680_Checker for common test pattern
 */
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

/**
 * @brief Update the display
 * @details Start update sequence to show internal memory content on the display.
 * @param[in] hepd: SSD1680 handle pointer
 * @return HAL status
 * @note Slow. Takes half to several seconds depending on display model.
 */
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

/**
 * @brief Set data entry mode
 * @details Set behavior of address pointer on bulk data transfer into or out of RAM
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] mode: Data entry mode
 * @return HAL status
 */
HAL_StatusTypeDef SSD1680_DataEntryMode(SSD1680_HandleTypeDef *hepd, const enum SSD1680_DataEntryMode mode) {
  return SSD1680_Send(hepd, SSD1680_DATA_ENTRY_MODE, &mode, sizeof(mode));   // 0x11
}

/**
 * @brief Get gate (row) scan range for refresh operation
 * @details Set the range of rows to be updated on next refresh
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] top: topmost row to be updated
 * @param[in] height: number of rows to be updated
 * @return HAL status
 */
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

/**
 * @brief Set horizontal RAM range
 * @details Set the range outside of which the X (horizontal) address counter will wrap around.
 * Useful when sending a sprite to avoid issuing extra address setting commands.
 * The Y address counter will also increment or decrement depending on data entry mode.
 * Intended to use together with SSD1680_DataEntryMode and SSD1680_RAMYRange.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] left: start of the range. Must be multiple of 8.
 * @param[in] width: width of the range. Must be multiple of 8.
 * @return HAL status
 * @see SSD1680_SSD1680_DataEntryMode
 * @see SSD1680_RAMYRange
 */
HAL_StatusTypeDef SSD1680_RAMXRange(SSD1680_HandleTypeDef *hepd, const uint8_t left, const uint8_t width) {
  if (left % 8 + width % 8)
    return HAL_ERROR;
  const uint8_t ramXRange[] = { left / 8, (left + width) / 8 - 1};
  return SSD1680_Send(hepd, SSD1680_RAM_X_RANGE, ramXRange, sizeof(ramXRange));   // 0x44
}

/**
 * @brief Set vertical RAM range
 * @details Set the range outside of which the Y (vertical) address counter will wrap around.
 * Useful when sending a sprite to avoid issuing extra address setting commands.
 * The X address counter will also increment or decrement depending on data entry mode.
 * Intended to use together with SSD1680_DataEntryMode and SSD1680_RAMXRange.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] top: start of the range.
 * @param[in] height: height of the range.
 * @return HAL status
 * @see SSD1680_SSD1680_DataEntryMode
 * @see SSD1680_RAMXRange
 */
HAL_StatusTypeDef SSD1680_RAMYRange(SSD1680_HandleTypeDef *hepd, const uint16_t top, const uint16_t height) {
  const uint16_t ramYRange[] = { top, (top + height) - 1};
  return SSD1680_Send(hepd, SSD1680_RAM_Y_RANGE, (uint8_t *)ramYRange, sizeof(ramYRange));   // 0x45
}

/**
 * @brief Send update control sequence 1.
 * @details Not intended to be used outside of SSD1680_Refresh
 * @param[in] hepd: SSD1680 handle pointer
 * @return HAL status
 * @see SSD1680_Refresh
 */
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

/**
 * @brief Send update control sequence 2.
 * @details Not intended to be used outside of SSD1680_Refresh
 * @param[in] hepd: SSD1680 handle pointer
 * @return HAL status
 * @see SSD1680_Refresh
 */
HAL_StatusTypeDef SSD1680_UpdateControl2(SSD1680_HandleTypeDef *hepd) {
  const uint8_t updateControl2[] = { 0xF7 }; /* See datasheet page 26 */
  return SSD1680_Send(hepd, SSD1680_UPDATE_CONTROL_2, updateControl2, sizeof(updateControl2));  // 0x22
}

/**
 * @brief Set border color
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] color: border color
 * @return HAL status
 */
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

/**
 * @brief Set RAM bank for read
 * @details Sets the RAM bank for the next RAM read operation.
 * Not intended to use outside of SSD1680_GetRegion.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] ram: RAM bank
 * @return HAL status
 */
HAL_StatusTypeDef SSD1680_RAMReadOption(SSD1680_HandleTypeDef *hepd, const enum SSD1680_RAMBank ram) {
  return SSD1680_Send(hepd, SSD1680_RAM_READ_OPT, (uint8_t *)&ram, 1);
}

/**
 * @brief Read temperature
 * @details Reads temperature from internal sensor.
 * @param[in] hepd: SSD1680 handle pointer
 * @return Temperature
 * @note Return value is not surrounding air temperature but the temperature of display itself.
 * Pretty inaccurate relative to dedicated sensor chips.
 */
uint16_t SSD1680_ReadTemp(SSD1680_HandleTypeDef *hepd) {
  const uint8_t tempSensor[] = { 0x80 };
  SSD1680_Send(hepd, SSD1680_SELECT_TEMP_SENSOR, tempSensor, sizeof(tempSensor));   // 0x18
  uint16_t temp;
  SSD1680_Receive(hepd, SSD1680_READ_TEMP, (uint8_t *)&temp, sizeof(temp));         // 0x1b
  return temp >> 4;
}


/**
 * @brief Reset both horizontal and vertical RAM ranges
 * @details Sets both horizontal and vertical RAM ranges to the actual display size.
 * Useful when updating whole screen after sending sprite with SSD1680_SetRegion
 * @param[in] hepd: SSD1680 handle pointer
 * @return HAL status
 * @see SSD1680_RAMXRange
 * @see SSD1680_RAMYRange
 */
HAL_StatusTypeDef SSD1680_ResetRange(SSD1680_HandleTypeDef *hepd) {
  HAL_StatusTypeDef status = HAL_OK;
  if ((status = SSD1680_RAMXRange(hepd, 0, hepd->Resolution_X / 8)))
    return status;
  if ((status = SSD1680_RAMYRange(hepd, 0, hepd->Resolution_Y)))
    return status;
  return status;
}

/**
 * @brief Set RAM start address for bulk data transfer
 * @details Sets both horizontal and vertical address counters before bult data transfer.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] x: column. Must be multiple of 8.
 * @param[in] y: row
 * @return HAL status
 * @see SSD1680_GetRegion
 * @see SSD1680_SetRegion
 */
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

/**
 * @brief Bulk read data from RAM
 * @details Reads data from RAM region with specified location and dimensions.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] left: leftmost column. Must be multiple of 8.
 * @param[in] top: topmost row
 * @param[in] width: region width. Must be multiple of 8.
 * @param[in] height: region height.
 * @param[out] data_k: pointer to buffer where to store data from primary (black) RAM bank.
 * Buffer must be at least `width / 8 * height` bytes.
 * Set to NULL to skip reading from primary RAM bank.
 * @param[out] data_r: pointer to buffer where to stora data from secondary (red) RAM bank.
 * Set to NULL to skip reading from secondary RAM bank.
 * Buffer must be at least `width / 8 * height` bytes.
 * @return HAL status
 * @see SSD1680_SetRegion
 */
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
    if ((status = HAL_SPI_Transmit(hepd->SPI_Handle, &cmd, 1, hepd->SPI_Timeout))) {
      HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
      goto exit_GetRegion;
    }
    HAL_GPIO_WritePin(hepd->DC_Port, hepd->DC_Pin, GPIO_PIN_SET);
    // Reading data
    uint8_t *pData = data_k;
#define SSD1680_DUMMY_BYTES 2
#if SSD1680_DUMMY_BYTES
      // Reading dummy bytes
      if ((status = HAL_SPI_Receive(hepd->SPI_Handle, pData, SSD1680_DUMMY_BYTES, hepd->SPI_Timeout)))
        goto exit_GetRegion;
#endif // SSD1680_DUMMY_BYTES
      if ((status = HAL_SPI_Receive(hepd->SPI_Handle, pData, width / 8 * height, hepd->SPI_Timeout)))
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

/**
 * @brief Bulk write data to RAM
 * @details Writes data to RAM region with specified location and dimensions.
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] left: leftmost column. Must be multiple of 8.
 * @param[in] top: topmost row
 * @param[in] width: region width. Must be multiple of 8.
 * @param[in] height: region height.
 * @param[in] data_k: pointer to buffer where data for primary (black) RAM bank is stored.
 * Buffer must be at least `width / 8 * height` bytes.
 * Set to NULL to skip updating primary RAM bank.
 * @param[in] data_r: pointer to buffer where data for secondary (red) RAM bank is stored.
 * Buffer must be at least `width / 8 * height` bytes.
 * Set to NULL to skip updating secondary RAM bank.
 * @return HAL status
 * @see SSD1680_GetRegion
 */
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

/**
 * @brief Put a text a screen horizontally
 * @details Prints a string at specified position with specified font.
 * Partially supports the following control characters:
 * @li `0x08` Backspace
 * @li `0x09` Tab
 * @li `0x0A` Line feed
 * @li `0x0D` Carriage return
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] left: horizontal position of a string
 * @param[in] top: vertical position of a string
 * @param[in] string: zero-terminated string to print
 * @param[in] font: pointer to font
 * @return HAL status
 * @bug Overflowing screen causes text to be highly distorted.
 */
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

/**
 * @brief Put a text a screen vertically
 * @details Prints a string at specified position with specified font.
 * Intended to be used with special fonts with glyphs turned 90 degrees clockwise (i.e. like cp866_8x8_r or cp866_8x16_r)
 * Partially supports the following control characters:
 * @li `0x08` Backspace
 * @li `0x09` Tab
 * @li `0x0A` Line feed
 * @li `0x0D` Carriage return
 * @param[in] hepd: SSD1680 handle pointer
 * @param[in] left: horizontal position of a string
 * @param[in] top: vertical position of a string
 * @param[in] string: zero-terminated string to print
 * @param[in] font: pointer to font
 * @return HAL status
 * @bug Overflowing screen causes text to be highly distorted.
 */
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

/*
void ByteGridTranspose(uint8_t *out, const uint16_t width, const uint16_t height, const uint8_t *in) {
  for (uint16_t y = 0; y < height; ++y)
    for (uint16_t x = 0; x < width; ++x) {
      uint16_t iB = (y * width + x) / 8;
      uint16_t oB = (x * height) / 8 + (height - y - 1) / 8;
      for (uint8_t b = 0; b < 8; ++b)
        out[oB] = out[oB] | (((in[iB] >> (7 - x % 8)) & 1) <<  (0 + y % 8));
    }
}
*/
