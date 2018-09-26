/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * Description: functions for I2C connected external EEPROM.
 * Not platform dependent.
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(I2C_EEPROM)

// --------------------------------------------------------------------------
// Includes
// --------------------------------------------------------------------------

#include HAL_PATH(.., HAL.h)
#include <Wire.h>

// --------------------------------------------------------------------------
// Externals
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Local defines
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Types
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Public Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Private Variables
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Private functions
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// Public functions
// --------------------------------------------------------------------------

static uint8_t eeprom_device_address = 0x50;

static void eeprom_init(void) {
  static bool eeprom_initialized = false;
  if (!eeprom_initialized) {
    Wire1.begin();
    eeprom_initialized = true;
  }
}

void eeprom_write_byte(unsigned char *pos, unsigned char value) {
  unsigned eeprom_address = (unsigned) pos;

  eeprom_init();

  Wire1.beginTransmission(eeprom_device_address);
  Wire1.write((int)(eeprom_address >> 8));   // MSB
  Wire1.write((int)(eeprom_address & 0xFF)); // LSB
  Wire1.write(value);
  Wire1.endTransmission();

  // wait for write cycle to complete
  // this could be done more efficiently with "acknowledge polling"
  delay(5);
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
void eeprom_update_block(const void *pos, void* eeprom_address, size_t n) {
  eeprom_init();

  Wire1.beginTransmission(eeprom_device_address);
  Wire1.write((int)((unsigned)eeprom_address >> 8));   // MSB
  Wire1.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
  Wire1.endTransmission();

  uint8_t *ptr = (uint8_t*)pos;
  uint8_t flag = 0;
  Wire1.requestFrom(eeprom_device_address, (byte)n);
  for (byte c = 0; c < n && Wire1.available(); c++)
    flag |= Wire1.read() ^ ptr[c];

  if (flag) {
    Wire1.beginTransmission(eeprom_device_address);
    Wire1.write((int)((unsigned)eeprom_address >> 8));   // MSB
    Wire1.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
    Wire1.write((uint8_t*)pos, n);
    Wire1.endTransmission();

    // wait for write cycle to complete
    // this could be done more efficiently with "acknowledge polling"
    delay(5);
  }
}


unsigned char eeprom_read_byte(unsigned char *pos) {
  byte data = 0xFF;
  unsigned eeprom_address = (unsigned)pos;

  eeprom_init();

  Wire1.beginTransmission(eeprom_device_address);
  Wire1.write((int)(eeprom_address >> 8));   // MSB
  Wire1.write((int)(eeprom_address & 0xFF)); // LSB
  Wire1.endTransmission();
  Wire1.requestFrom(eeprom_device_address, (byte)1);
  return Wire1.available() ? Wire1.read() : 0xFF;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void eeprom_read_block(void* pos, const void* eeprom_address, size_t n) {
  eeprom_init();

  Wire1.beginTransmission(eeprom_device_address);
  Wire1.write((int)((unsigned)eeprom_address >> 8));   // MSB
  Wire1.write((int)((unsigned)eeprom_address & 0xFF)); // LSB
  Wire1.endTransmission();
  Wire1.requestFrom(eeprom_device_address, (byte)n);
  for (byte c = 0; c < n; c++ )
    if (Wire1.available()) *((uint8_t*)pos + c) = Wire1.read();
}


#endif // ENABLED(I2C_EEPROM)
