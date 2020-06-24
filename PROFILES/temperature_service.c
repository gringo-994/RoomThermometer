#include <string.h>

//#define xdc_runtime_Log_DISABLE_ALL 1  // Add to disable logs from this file
#include <xdc/runtime/Diags.h>
#include <uartlog/UartLog.h>

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "temperature_service.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* GLOBAL VARIABLES
*/

// Temperature_Service Service UUID
CONST uint8_t TemperatureServiceUUID[ATT_UUID_SIZE] =
{
  TEMPERATURE_SERVICE_SERV_UUID_BASE128(TEMPERATURE_SERVICE_SERV_UUID)
};

// TEMPERATURE UUID
CONST uint8_t ts_TEMPERATUREUUID[ATT_UUID_SIZE] =
{
 TS_TEMPERATURE_UUID_BASE128(TS_TEMPERATURE_UUID)
};

// TEMPERATURE TAU UUID
CONST uint8_t ts_TEMPERATURETAUUUID[ATT_UUID_SIZE] =
{
 TS_TEMPERATURE_TAU_UUID_BASE128(TS_TEMPERATURE_TAU_UUID)
};


/*********************************************************************
 * LOCAL VARIABLES
 */

static TemperatureServiceCBs_t *pAppCBs = NULL;
static uint8_t bs_icall_rsp_task_id = INVALID_TASK_ID;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t TemperatureServiceDecl = { ATT_UUID_SIZE, TemperatureServiceUUID };

// Characteristic "Temperature" Properties (for declaration)
static uint8_t ts_TemperatureProps = GATT_PROP_NOTIFY | GATT_PROP_READ;

// Characteristic "temperature" Value variable
static uint8_t ts_TemperatureVal[TS_TEMPERATURE_LEN] = {0};

// Length of data in characteristic "temperature" Value variable, initialized to minimal size.
static uint16_t ts_TemperatureValLen = TS_TEMPERATURE_LEN_MIN;

// Characteristic "Temperature" Client Characteristic Configuration Descriptor
static gattCharCfg_t *ts_TemperatureConfig;



// Characteristic "Temperature" Properties (for declaration)
static uint8_t ts_TemperatureTauProps = GATT_PROP_WRITE | GATT_PROP_READ;

// Characteristic "Temperature" Value variable
static uint8_t ts_TemperatureTauVal[TS_TEMPERATURE_TAU_LEN] = {0};

// Length of data in characteristic "Temperature" Value variable, initialized to minimal size.
static uint16_t ts_TemperatureTauValLen = TS_TEMPERATURE_TAU_LEN_MIN;



/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t Temperature_ServiceAttrTbl[] =
{
  // Temperature_Service Service Declaration
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID },
    GATT_PERMIT_READ,
    0,
    (uint8_t *)&TemperatureServiceDecl
  },
    // Temperature Characteristic Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &ts_TemperatureProps
    },
      // Temperature Characteristic Value
      {
        { ATT_UUID_SIZE, ts_TEMPERATUREUUID },
        GATT_PERMIT_READ,
        0,
        ts_TemperatureVal
      },
      // Temperature CCCD
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)&ts_TemperatureConfig
      },
      {
       // Temperature  TAU Characteristic Declaration
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &ts_TemperatureTauProps
      },
        // Temperature  TAU Characteristic Value
        {
          { ATT_UUID_SIZE, ts_TEMPERATURETAUUUID },
          GATT_PERMIT_READ | GATT_PERMIT_WRITE,
          0,
          ts_TemperatureTauVal
        },
};

static bStatus_t Temperature_Service_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                           uint16_t maxLen, uint8_t method );
static bStatus_t Temperature_Service_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                            uint8_t *pValue, uint16_t len, uint16_t offset,
                                            uint8_t method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t Temperature_ServiceCBs =
{
 Temperature_Service_ReadAttrCB,  // Read callback function pointer
 Temperature_Service_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

extern bStatus_t TemperatureService_AddService( uint8_t rspTaskId )
{
  uint8_t status;

  // Allocate Client Characteristic Configuration table
  ts_TemperatureConfig = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * linkDBNumConns );
  if ( ts_TemperatureConfig == NULL )
  {
    return ( bleMemAllocError );
  }
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, ts_TemperatureConfig );
  // Register GATT attribute list and CBs with GATT Server App
  status = GATTServApp_RegisterService( Temperature_ServiceAttrTbl,
                                        GATT_NUM_ATTRS( Temperature_ServiceAttrTbl ),
                                        GATT_MAX_ENCRYPT_KEY_SIZE,
                                        &Temperature_ServiceCBs );
  Log_info1("Registered service, %d attributes", (IArg)GATT_NUM_ATTRS( Temperature_ServiceAttrTbl ));
  bs_icall_rsp_task_id = rspTaskId;

  return ( status );
}

bStatus_t TemperatureService_RegisterAppCBs( TemperatureServiceCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    pAppCBs = appCallbacks;
    Log_info1("Registered callbacks to application. Struct %p", (IArg)appCallbacks);
    return ( SUCCESS );
  }
  else
  {
    Log_warning0("Null pointer given for app callbacks.");
    return ( FAILURE );
  }
}

static uint8_t Temperature_Service_findCharParamId(gattAttribute_t *pAttr)
{
  // Is this a Client Characteristic Configuration Descriptor?
  if (ATT_BT_UUID_SIZE == pAttr->type.len && GATT_CLIENT_CHAR_CFG_UUID == *(uint16_t *)pAttr->type.uuid)
    return Temperature_Service_findCharParamId(pAttr - 1); // Assume the value attribute precedes CCCD and recurse

  // Is this attribute in "Temperature"?
  else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, ts_TEMPERATUREUUID, pAttr->type.len))
    return TS_TEMPERATURE_ID;

  // Is this attribute in "Temperature"?
  else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, ts_TEMPERATURETAUUUID, pAttr->type.len))
    return TS_TEMPERATURE_TAU_ID;

  else
    return 0xFF; // Not found. Return invalid.
}

bStatus_t TemperatureService_SetParameter( uint8_t param, uint16_t len, void *value )
{
  bStatus_t ret = SUCCESS;
  uint8_t  *pAttrVal;
  uint16_t *pValLen;
  uint16_t valMinLen;
  uint16_t valMaxLen;
  uint8_t sendNotiInd = FALSE;
  gattCharCfg_t *attrConfig;
  uint8_t needAuth;

  switch ( param )
  {
    case TS_TEMPERATURE_ID:
      pAttrVal  =  ts_TemperatureVal;
      pValLen   = &ts_TemperatureValLen;
      valMinLen =  TS_TEMPERATURE_LEN_MIN;
      valMaxLen =  TS_TEMPERATURE_LEN;
      sendNotiInd = TRUE;
      attrConfig  = ts_TemperatureConfig;
      needAuth    = FALSE; // Change if authenticated link is required for sending.
      Log_info2("SetParameter : %s len: %d", (IArg)"TEMPERATURE", (IArg)len);
      break;

    case TS_TEMPERATURE_TAU_ID:
        pAttrVal  =  ts_TemperatureTauVal;
        pValLen   = &ts_TemperatureTauValLen;
        valMinLen =  TS_TEMPERATURE_TAU_LEN_MIN;
        valMaxLen =  TS_TEMPERATURE_TAU_LEN;
        sendNotiInd = FALSE;
        needAuth    = FALSE; // Change if authenticated link is required for sending.
        Log_info2("SetParameter : %s len: %d", (IArg)"TEMPERATURE TAU", (IArg)len);
      break;

    default:
      Log_error1("SetParameter: Parameter #%d not valid.", (IArg)param);
      return INVALIDPARAMETER;
  }

  // Check bounds, update value and send notification or indication if possible.
  if ( len <= valMaxLen && len >= valMinLen )
  {
    memcpy(pAttrVal, value, len);
    *pValLen = len; // Update length for read and get.

    if (sendNotiInd)
    {
      Log_info2("Trying to send noti/ind: connHandle %x, %s",
                (IArg)attrConfig[0].connHandle,
                (IArg)((attrConfig[0].value==0)?"\x1b[33mNoti/ind disabled\x1b[0m" :
                       (attrConfig[0].value==1)?"Notification enabled" :
                                                "Indication enabled"));
      // Try to send notification.
      GATTServApp_ProcessCharCfg( attrConfig, pAttrVal, needAuth,
                                  Temperature_ServiceAttrTbl, GATT_NUM_ATTRS( Temperature_ServiceAttrTbl ),
                                  bs_icall_rsp_task_id,  Temperature_Service_ReadAttrCB);
    }
  }
  else
  {
    Log_error3("Length outside bounds: Len: %d MinLen: %d MaxLen: %d.", (IArg)len, (IArg)valMinLen, (IArg)valMaxLen);
    ret = bleInvalidRange;
  }

  return ret;
}

bStatus_t TemperatureService_GetParameter( uint8_t param, uint16_t *len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case TS_TEMPERATURE_ID:
      *len = MIN(*len, ts_TemperatureValLen);
      memcpy(value, ts_TemperatureVal, *len);
      break;

    case TS_TEMPERATURE_TAU_ID:
      *len = MIN(*len, ts_TemperatureTauValLen);
      memcpy(value, ts_TemperatureTauVal, *len);
      break;

    default:
      Log_error1("GetParameter: Parameter #%d not valid.", (IArg)param);
      ret = INVALIDPARAMETER;
      break;
  }
  return ret;
}

static bStatus_t Temperature_Service_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                       uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                       uint16_t maxLen, uint8_t method )
{
  bStatus_t status = SUCCESS;
  uint16_t valueLen;
  uint8_t paramID = 0xFF;

  // Find settings for the characteristic to be read.
  paramID = Temperature_Service_findCharParamId( pAttr );
  switch ( paramID )
  {
    case TS_TEMPERATURE_ID:
      valueLen = ts_TemperatureValLen;

      Log_info4("ReadAttrCB : %s connHandle: %d offset: %d method: 0x%02x",
                 (IArg)"temperature",
                 (IArg)connHandle,
                 (IArg)offset,
                 (IArg)method);
      break;

    case TS_TEMPERATURE_TAU_ID:
      valueLen = ts_TemperatureTauValLen;

      Log_info4("ReadAttrCB : %s connHandle: %d offset: %d method: 0x%02x",
                 (IArg)"temperature tau",
                 (IArg)connHandle,
                 (IArg)offset,
                 (IArg)method);
      break;

    default:
      Log_error0("Attribute was not found.");
      return ATT_ERR_ATTR_NOT_FOUND;
  }
  // Check bounds and return the value
  if ( offset > valueLen )  // Prevent malicious ATT ReadBlob offsets.
  {
    Log_error0("An invalid offset was requested.");
    status = ATT_ERR_INVALID_OFFSET;
  }
  else
  {
    *pLen = MIN(maxLen, valueLen - offset);  // Transmit as much as possible
    memcpy(pValue, pAttr->pValue + offset, *pLen);
  }

  return status;
}

static bStatus_t Temperature_Service_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t len, uint16_t offset,
                                        uint8_t method )
{
  bStatus_t status  = SUCCESS;
  uint8_t   paramID = 0xFF;
  uint8_t   changeParamID = 0xFF;
  uint16_t writeLenMin;
  uint16_t writeLenMax;
  uint16_t *pValueLenVar;

  // See if request is regarding a Client Characterisic Configuration
  if (ATT_BT_UUID_SIZE == pAttr->type.len && GATT_CLIENT_CHAR_CFG_UUID == *(uint16_t *)pAttr->type.uuid)
  {
    Log_info3("WriteAttrCB (CCCD): param: %d connHandle: %d %s",
              (IArg)Temperature_Service_findCharParamId(pAttr),
              (IArg)connHandle,
              (IArg)(method == GATT_LOCAL_WRITE?"- restoring bonded state":"- OTA write"));

    // Allow notification and indication, but do not check if really allowed per CCCD.
    status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                             offset, GATT_CLIENT_CFG_NOTIFY |
                                                     GATT_CLIENT_CFG_INDICATE );
    if (SUCCESS == status && pAppCBs && pAppCBs->pfnCfgChangeCb)
       pAppCBs->pfnCfgChangeCb( connHandle, TEMPERATURE_SERVICE_SERV_UUID,
                                Temperature_Service_findCharParamId(pAttr), pValue, len );

     return status;
  }

  // Find settings for the characteristic to be written.
  paramID = Temperature_Service_findCharParamId( pAttr );
  switch ( paramID )
  {
      case TS_TEMPERATURE_TAU_ID:
          writeLenMin  = TS_TEMPERATURE_TAU_LEN_MIN;
          writeLenMax  = TS_TEMPERATURE_TAU_LEN;
          pValueLenVar = &ts_TemperatureTauValLen;
          Log_info5("WriteAttrCB : %s connHandle(%d) len(%d) offset(%d) method(0x%02x)",
                               (IArg)"Temperature TAU",
                               (IArg)connHandle,
                               (IArg)len,
                               (IArg)offset,
                               (IArg)method);
          break;
      default:
      Log_error0("Attribute was not found.");
      return ATT_ERR_ATTR_NOT_FOUND;
  }
  if ( offset >= writeLenMax )
    {
      Log_error0("An invalid offset was requested.");
      status = ATT_ERR_INVALID_OFFSET;
    }
  else if ( offset + len > writeLenMax )
    {
      Log_error0("Invalid value length was received.");
      status = ATT_ERR_INVALID_VALUE_SIZE;
    }
  else if ( offset + len < writeLenMin && ( method == ATT_EXECUTE_WRITE_REQ || method == ATT_WRITE_REQ ) )
    {
      // Refuse writes that are lower than minimum.
      // Note: Cannot determine if a Reliable Write (to several chars) is finished, so those will
      //       only be refused if this attribute is the last in the queue (method is execute).
      //       Otherwise, reliable writes are accepted and parsed piecemeal.
      Log_error0("Invalid value length was received.");
      status = ATT_ERR_INVALID_VALUE_SIZE;
    }
  else
    {
      memcpy(pAttr->pValue + offset, pValue, len);

      if ( offset + len >= writeLenMin )
      {
        changeParamID = paramID;
        *pValueLenVar = offset + len; // Update data length.
      }
    }

    // Let the application know something changed (if it did) by using the
    // callback it registered earlier (if it did).
    if (changeParamID != 0xFF)
      if ( pAppCBs && pAppCBs->pfnChangeCb )
          pAppCBs->pfnChangeCb( connHandle, TEMPERATURE_SERVICE_SERV_UUID, paramID, pValue, len+offset ); // Call app function from stack task context.

    return status;
  }


