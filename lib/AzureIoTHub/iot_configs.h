/*****************************************************************************************************************************
**********************************    Author  : Ehab Magdy Abdullah                      *************************************
**********************************    Linkedin: https://www.linkedin.com/in/ehabmagdyy/  *************************************
**********************************    Youtube : https://www.youtube.com/@EhabMagdyy      *************************************
******************************************************************************************************************************/

// Enter Your Wifi and Password
#define IOT_CONFIG_WIFI_SSID "PerryZone"
#define IOT_CONFIG_WIFI_PASSWORD "agenteP123"


// Azure IoT Hub
#define IOT_CONFIG_IOTHUB_FQDN "SAAD-Broker.azure-devices.net"
// Azure IoT Hub Device ID
#define IOT_CONFIG_DEVICE_ID "esp32-SAAD"
// Azure IoT Hub Device Key (primary key)
#define IOT_CONFIG_DEVICE_KEY "Cq3hhvpmOwSG1LM+JA7CbFuOc+TVcmrbGSmJMpfxC9w="

// Publish 1 message every 5 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 5000