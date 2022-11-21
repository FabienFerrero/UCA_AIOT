/*
 *     __          ____        _____                       __    _ __  
 *    / /   ____  / __ \____ _/ ___/____  ____ _________  / /   (_) /_ 
 *   / /   / __ \/ /_/ / __ `/\__ \/ __ \/ __ `/ ___/ _ \/ /   / / __ \
 *  / /___/ /_/ / _, _/ /_/ /___/ / /_/ / /_/ / /__/  __/ /___/ / /_/ /
 * /_____/\____/_/ |_|\__,_//____/ .___/\__,_/\___/\___/_____/_/_.___/ 
 *                              /_/                                    
 * Author: m1nhle, mtnguyen
 * Lib jointy developed by UCA & RFThings
 */

#include "Arduino.h"
#include "SPI.h"
#include "sx126x_hal.h"

sx126x_hal_status_t sx126x_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length ) {
    sx126x_hal_t* sx126x_hal = ( sx126x_hal_t* ) context;

    while(digitalRead(sx126x_hal->busy)) { }
    
#ifdef SPI_HAS_TRANSACTION
    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
#endif
    digitalWrite(sx126x_hal->nss, LOW);

    uint8_t buffer[255];
    memcpy(buffer, command, command_length);
    memcpy(buffer + command_length, data, data_length);

    SPI.transfer(buffer, (uint32_t)(command_length + data_length));

    digitalWrite(sx126x_hal->nss, HIGH);
#ifdef SPI_HAS_TRANSACTION
    SPI.endTransaction();
#endif

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_write_bulk( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length ) {
    sx126x_hal_t* sx126x_hal = ( sx126x_hal_t* ) context;

    while(digitalRead(sx126x_hal->busy)) { }
    
#ifdef SPI_HAS_TRANSACTION
    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
#endif
    digitalWrite(sx126x_hal->nss, LOW);

    SPI.transfer((uint8_t* )command, command_length);

    uint8_t buffer[32];
	uint8_t *ptr = (uint8_t*)data;
    uint16_t _data_length = data_length;
	while(_data_length) {
		uint8_t chunkSize = ((_data_length) < (sizeof buffer) ? (_data_length) : (sizeof buffer)); // MIN(size, sizeof buffer);
		memcpy(buffer, ptr, chunkSize);
		SPI.transfer(buffer, chunkSize);
		_data_length -= chunkSize;
		ptr += chunkSize;
	}

    digitalWrite(sx126x_hal->nss, HIGH);
#ifdef SPI_HAS_TRANSACTION
    SPI.endTransaction();
#endif

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length ) {
    sx126x_hal_t* sx126x_hal = ( sx126x_hal_t* ) context;

    while(digitalRead(sx126x_hal->busy)) { }

#ifdef SPI_HAS_TRANSACTION
    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
#endif
    digitalWrite(sx126x_hal->nss, LOW);

    uint8_t buffer[255];
    memcpy(buffer, command, command_length);
    memset(buffer + command_length, SX126X_NOP, data_length);

    SPI.transfer(buffer, (uint32_t)(command_length + data_length));
    memcpy(data, buffer + command_length, data_length);

    digitalWrite(sx126x_hal->nss, HIGH);
#ifdef SPI_HAS_TRANSACTION
    SPI.endTransaction();
#endif

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_reset( const void* context ) {
    sx126x_hal_t* sx126x_hal = ( sx126x_hal_t* ) context;
    
    delay(20);
    digitalWrite(sx126x_hal->reset, LOW);
    delay(50);
    digitalWrite(sx126x_hal->reset, HIGH);
    delay(50);

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup( const void* context ) {
    sx126x_hal_t* sx126x_hal = ( sx126x_hal_t* ) context;
    
#ifdef SPI_HAS_TRANSACTION
    SPI.beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
#endif
    digitalWrite(sx126x_hal->nss, LOW);

    SPI.transfer(SX126X_NOP, 1);

	digitalWrite(sx126x_hal->nss, HIGH);
#ifdef SPI_HAS_TRANSACTION
    SPI.endTransaction();
#endif

    while (digitalRead(sx126x_hal->busy)) { }
    
	return SX126X_HAL_STATUS_OK;
}