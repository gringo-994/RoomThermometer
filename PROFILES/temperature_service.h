/*
 * temperature_service.h
 *
 *  Created on: 02 giu 2020
 *      Author: raffaele-bithiatec
 */

#ifndef PROFILES_TEMPERATURE_SERVICE_H_
#define PROFILES_TEMPERATURE_SERVICE_H_


#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <bcomdef.h>

/*********************************************************************
 * CONSTANTS
 */
// Service UUID
#define TEMPERATURE_SERVICE_SERV_UUID 0x1140
#define TEMPERATURE_SERVICE_SERV_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), \
    0x00, 0xF0

// TEMPERATURE Characteristic defines
#define TS_TEMPERATURE_ID                 0
#define TS_TEMPERATURE_UUID               0x1141
#define TS_TEMPERATURE_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, 0xF0
#define TS_TEMPERATURE_LEN                1
#define TS_TEMPERATURE_LEN_MIN            1
// TEMPERATURE Tau Characteristic defines
#define TS_TEMPERATURE_TAU_ID               1
#define TS_TEMPERATURE_TAU_UUID             0x1142
#define TS_TEMPERATURE_TAU_UUID_BASE128(uuid) 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0xB0, 0x00, 0x40, 0x51, 0x04, LO_UINT16(uuid), HI_UINT16(uuid), 0x00, 0xF0
#define TS_TEMPERATURE_TAU_LEN                1
#define TS_TEMPERATURE_TAU_LEN_MIN            1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*TemperatureServiceChange_t)( uint16_t connHandle, uint16_t svcUuid, uint8_t paramID, uint8_t *pValue, uint16_t len );

typedef struct
{
    TemperatureServiceChange_t pfnChangeCb;          // Called when characteristic value changes
    TemperatureServiceChange_t pfnCfgChangeCb;       // Called when characteristic CCCD changes
} TemperatureServiceCBs_t;

/*********************************************************************
 * API FUNCTIONS
 */

/*
 * TemperatureService_AddService- Initializes the TemperatureService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t TemperatureService_AddService(uint8_t rspTaskId);

/*
 * TemperatureService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t TemperatureService_RegisterAppCBs(TemperatureServiceCBs_t *appCallbacks);

/*
 * TemperatureService_SetParameter - Set a TemperatureService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t TemperatureService_SetParameter(uint8_t param,
                                            uint16_t len,
                                            void *value);

/*
 * ButtonService_GetParameter - Get a TemperatureService parameter.
 *
 *    param - Profile parameter ID
 *    len   - pointer to a variable that contains the maximum length that can be written to *value.
              After the call, this value will contain the actual returned length.
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t TemperatureService_GetParameter(uint8_t param,
                                            uint16_t *len,
                                            void *value);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* PROFILES_TEMPERATURE_SERVICE_H_ */
