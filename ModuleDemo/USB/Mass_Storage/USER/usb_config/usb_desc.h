

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define MASS_SIZ_DEVICE_DESC              18
#define MASS_SIZ_CONFIG_DESC              32

#define MASS_SIZ_STRING_LANGID            4
#define MASS_SIZ_STRING_VENDOR            14
#define MASS_SIZ_STRING_PRODUCT           38
#define MASS_SIZ_STRING_SERIAL            26
#define MASS_SIZ_STRING_INTERFACE         18

/* Exported functions ------------------------------------------------------- */
extern const uint8_t MASS_DeviceDescriptor[MASS_SIZ_DEVICE_DESC];
extern const uint8_t MASS_ConfigDescriptor[MASS_SIZ_CONFIG_DESC];

extern const uint8_t MASS_StringLangID[MASS_SIZ_STRING_LANGID];
extern const uint8_t MASS_StringVendor[MASS_SIZ_STRING_VENDOR];
extern const uint8_t MASS_StringProduct[MASS_SIZ_STRING_PRODUCT];
extern uint8_t MASS_StringSerial[MASS_SIZ_STRING_SERIAL];
extern const uint8_t MASS_StringInterface[MASS_SIZ_STRING_INTERFACE];

#endif /* __USB_DESC_H */



