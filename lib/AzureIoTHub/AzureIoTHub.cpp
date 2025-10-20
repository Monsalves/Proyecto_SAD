#include "AzureIoTHub.h"

#include <Arduino.h>
#include <WiFi.h>
#include <az_core.h>
#include <az_result.h>
#include <az_iot.h>
#include <azure_ca.h>
#include <mqtt_client.h>
#include <cstdio>
#include <cstring>

#include "AzIoTSasToken.h"
#include "SerialLogger.h"
#include "iot_configs.h"
#include <SensorDHT22.h>

#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp32)"
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_QOS1 1
#define DO_NOT_RETAIN_MSG 0
#define SAS_TOKEN_DURATION_IN_MINUTES 60
#define UNIX_TIME_NOV_13_2017 1510592825

#define PST_TIME_ZONE -8
#define PST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF 1
#define GMT_OFFSET_SECS (PST_TIME_ZONE * 3600)
#define GMT_OFFSET_SECS_DST ((PST_TIME_ZONE + PST_TIME_ZONE_DAYLIGHT_SAVINGS_DIFF) * 3600)

#ifndef TELEMETRY_FREQUENCY_MILLISECS
#define TELEMETRY_FREQUENCY_MILLISECS 5000
#endif

#define LED_PIN 2
#define POTENTIOMETER_PIN 32

static const char* ssid = IOT_CONFIG_WIFI_SSID;
static const char* password = IOT_CONFIG_WIFI_PASSWORD;
static const char* host = IOT_CONFIG_IOTHUB_FQDN;
static const char* mqtt_broker_uri = "mqtts://" IOT_CONFIG_IOTHUB_FQDN;
static const char* device_id = IOT_CONFIG_DEVICE_ID;
static const int mqtt_port = AZ_IOT_DEFAULT_MQTT_CONNECT_PORT;

static esp_mqtt_client_handle_t mqtt_client = nullptr;
static az_iot_hub_client client;

static char mqtt_client_id[128];
static char mqtt_username[128];
static char mqtt_password[200];
static uint8_t sas_signature_buffer[256];
static char telemetry_topic[128];
static String telemetry_payload;
static unsigned long next_telemetry_send_time_ms = 0;
static uint16_t potentiometer = 0;

#define INCOMING_DATA_BUFFER_SIZE 128
static char incoming_data[INCOMING_DATA_BUFFER_SIZE];

#ifndef IOT_CONFIG_USE_X509_CERT
static AzIoTSasToken sasToken(
    &client,
    AZ_SPAN_FROM_STR(IOT_CONFIG_DEVICE_KEY),
    AZ_SPAN_FROM_BUFFER(sas_signature_buffer),
    AZ_SPAN_FROM_BUFFER(mqtt_password));
#endif

static void connectToWiFi()
{
  Logger.Info("Connecting to WIFI SSID " + String(ssid));

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  Serial.println();
  Logger.Info("WiFi connected, IP address: " + WiFi.localIP().toString());
}

static void initializeTime()
{
  Logger.Info("Setting time using SNTP");

  configTime(GMT_OFFSET_SECS, GMT_OFFSET_SECS_DST, NTP_SERVERS);
  time_t now = time(nullptr);
  while (now < UNIX_TIME_NOV_13_2017)
  {
    delay(500);
    Serial.print('.');
    now = time(nullptr);
  }

  Serial.println();
  Logger.Info("Time initialized!");
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
  (void)handler_args;
  (void)base;
  (void)event_id;
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
#else
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
#endif
  switch (event->event_id)
  {
    case MQTT_EVENT_ERROR:
      Logger.Info("MQTT event MQTT_EVENT_ERROR");
      break;

    case MQTT_EVENT_CONNECTED:
    {
      Logger.Info("MQTT event MQTT_EVENT_CONNECTED");

      int msg_id = esp_mqtt_client_subscribe(mqtt_client, AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC, 1);
      if (msg_id == -1)
      {
        Logger.Error("Could not subscribe for cloud-to-device messages.");
      }
      else
      {
        Logger.Info("Subscribed for cloud-to-device messages; message id:" + String(msg_id));
      }

      msg_id = esp_mqtt_client_subscribe(mqtt_client, "$iothub/methods/POST/#", 1);
      if (msg_id == -1)
      {
        Logger.Error("Could not subscribe for direct methods.");
      }
      else
      {
        Logger.Info("Subscribed for direct methods; message id:" + String(msg_id));
      }
      break;
    }

    case MQTT_EVENT_DISCONNECTED:
      Logger.Info("MQTT event MQTT_EVENT_DISCONNECTED");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      Logger.Info("MQTT event MQTT_EVENT_SUBSCRIBED");
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      Logger.Info("MQTT event MQTT_EVENT_UNSUBSCRIBED");
      break;

    case MQTT_EVENT_PUBLISHED:
      Logger.Info("MQTT event MQTT_EVENT_PUBLISHED");
      break;

    case MQTT_EVENT_DATA:
    {
      Logger.Info("MQTT event MQTT_EVENT_DATA");

      const int topic_len = min(event->topic_len, INCOMING_DATA_BUFFER_SIZE - 1);
      strncpy(incoming_data, event->topic, topic_len);
      incoming_data[topic_len] = '\0';

      char payload[INCOMING_DATA_BUFFER_SIZE];
      const int payload_len = min(event->data_len, INCOMING_DATA_BUFFER_SIZE - 1);
      strncpy(payload, event->data, payload_len);
      payload[payload_len] = '\0';

      Logger.Info("Topic: " + String(incoming_data));
      Logger.Info("Payload: " + String(payload));

      if (strstr(incoming_data, "$iothub/methods/POST/") != nullptr)
      {
        char method_name[64];
        if (sscanf(incoming_data, "$iothub/methods/POST/%63[^/]/", method_name) == 1)
        {
          bool led_state = false;
          if (strcmp(method_name, "led_on") == 0)
          {
            digitalWrite(LED_PIN, HIGH);
            led_state = true;
            Logger.Info("LED turned ON");
          }
          else if (strcmp(method_name, "led_off") == 0)
          {
            digitalWrite(LED_PIN, LOW);
            led_state = false;
            Logger.Info("LED turned OFF");
          }
          else
          {
            Logger.Error("Unknown method: " + String(method_name));
          }

          char request_id[32];
          if (sscanf(incoming_data, "$iothub/methods/POST/%*[^/]/?$rid=%31s", request_id) == 1)
          {
            char response_topic[128];
            snprintf(response_topic, sizeof(response_topic), "$iothub/methods/res/200/?$rid=%s", request_id);
            const String response_payload = String("{\"led\":") + (led_state ? "true" : "false") + "}";
            if (esp_mqtt_client_publish(mqtt_client, response_topic, response_payload.c_str(), response_payload.length(), MQTT_QOS1, DO_NOT_RETAIN_MSG) == 0)
            {
              Logger.Error("Failed to publish direct method response");
            }
            else
            {
              Logger.Info("Direct method response sent successfully");
            }
          }
        }
      }
      break;
    }

    case MQTT_EVENT_BEFORE_CONNECT:
      Logger.Info("MQTT event MQTT_EVENT_BEFORE_CONNECT");
      break;

    default:
      Logger.Error("MQTT event UNKNOWN");
      break;
  }

#if !(defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3)
  return ESP_OK;
#endif
}

static az_result generateTelemetryPayload()
{
  telemetry_payload = medirHumedadTemperatura();
  return AZ_OK;
}

static az_result azurePublishTelemetry()
{
  Logger.Info("Sending telemetry ...");

  az_result result = az_iot_hub_client_telemetry_get_publish_topic(
      &client, nullptr, telemetry_topic, sizeof(telemetry_topic), nullptr);
  if (az_result_failed(result))
  {
    Logger.Error("Failed to compose telemetry topic");
    return result;
  }

  result = generateTelemetryPayload();
  if (az_result_failed(result))
  {
    Logger.Error("Failed to build telemetry payload");
    return result;
  }

  if (esp_mqtt_client_publish(
          mqtt_client,
          telemetry_topic,
          telemetry_payload.c_str(),
          telemetry_payload.length(),
          MQTT_QOS1,
          DO_NOT_RETAIN_MSG)
      == 0)
  {
    Logger.Error("Failed publishing telemetry");
    return AZ_ERROR_UNEXPECTED_END;
  }

  Logger.Info("Telemetry published successfully");
  return AZ_OK;
}

static void initializeIoTHubClient()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.user_agent = AZ_SPAN_FROM_STR(AZURE_SDK_CLIENT_USER_AGENT);

  if (az_result_failed(az_iot_hub_client_init(
          &client,
          az_span_create((uint8_t*)host, strlen(host)),
          az_span_create((uint8_t*)device_id, strlen(device_id)),
          &options)))
  {
    Logger.Error("Failed initializing Azure IoT Hub client");
    return;
  }

  size_t client_id_length = 0;
  if (az_result_failed(az_iot_hub_client_get_client_id(&client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Logger.Error("Failed getting client id");
    return;
  }
  mqtt_client_id[client_id_length] = '\0';

  if (az_result_failed(az_iot_hub_client_get_user_name(&client, mqtt_username, sizeofarray(mqtt_username), nullptr)))
  {
    Logger.Error("Failed getting MQTT username");
  }
}

static int initializeMqttClient()
{
#ifndef IOT_CONFIG_USE_X509_CERT
  if (sasToken.Generate(SAS_TOKEN_DURATION_IN_MINUTES) != 0)
  {
    Logger.Error("Failed generating SAS token");
    return 1;
  }
#endif

  esp_mqtt_client_config_t mqtt_config;
  memset(&mqtt_config, 0, sizeof(mqtt_config));

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  mqtt_config.broker.address.uri = mqtt_broker_uri;
  mqtt_config.broker.address.port = mqtt_port;
  mqtt_config.credentials.client_id = mqtt_client_id;
  mqtt_config.credentials.username = mqtt_username;

#ifdef IOT_CONFIG_USE_X509_CERT
  mqtt_config.credentials.authentication.certificate = IOT_CONFIG_DEVICE_CERT;
  mqtt_config.credentials.authentication.certificate_len = sizeof(IOT_CONFIG_DEVICE_CERT);
  mqtt_config.credentials.authentication.key = IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY;
  mqtt_config.credentials.authentication.key_len = sizeof(IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY);
#else
  mqtt_config.credentials.authentication.password = (const char*)az_span_ptr(sasToken.Get());
#endif

  mqtt_config.session.keepalive = 30;
  mqtt_config.network.disable_auto_reconnect = false;
  mqtt_config.session.disable_clean_session = 0;
  mqtt_config.broker.verification.certificate = (const char*)ca_pem;
  mqtt_config.broker.verification.certificate_len = ca_pem_len;
#else
  mqtt_config.uri = mqtt_broker_uri;
  mqtt_config.port = mqtt_port;
  mqtt_config.client_id = mqtt_client_id;
  mqtt_config.username = mqtt_username;
#ifdef IOT_CONFIG_USE_X509_CERT
  mqtt_config.client_cert_pem = IOT_CONFIG_DEVICE_CERT;
  mqtt_config.client_key_pem = IOT_CONFIG_DEVICE_CERT_PRIVATE_KEY;
#else
  mqtt_config.password = (const char*)az_span_ptr(sasToken.Get());
#endif
  mqtt_config.keepalive = 30;
  mqtt_config.disable_auto_reconnect = false;
  mqtt_config.disable_clean_session = 0;
  mqtt_config.event_handle = mqtt_event_handler;
  mqtt_config.user_context = nullptr;
  mqtt_config.cert_pem = (const char*)ca_pem;
#endif

  mqtt_client = esp_mqtt_client_init(&mqtt_config);
  if (mqtt_client == nullptr)
  {
    Logger.Error("Failed creating mqtt client");
    return 1;
  }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, nullptr);
#endif

  if (esp_mqtt_client_start(mqtt_client) != ESP_OK)
  {
    Logger.Error("Could not start mqtt client");
    return 1;
  }

  Logger.Info("MQTT client started");
  return 0;
}

static void establishConnection()
{
  connectToWiFi();
  initializeTime();
  initializeIoTHubClient();
  (void)initializeMqttClient();
}

void iniciarAzureIoTHub()
{
  establishConnection();
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void azureIoTHubLoop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWiFi();
  }
}

void azureSendTelemetry()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectToWiFi();
    return;
  }

  const unsigned long now = millis();
  if (now < next_telemetry_send_time_ms)
  {
    return;
  }

  potentiometer = map(analogRead(POTENTIOMETER_PIN), 0, 4095, 0, 100);

  if (az_result_failed(azurePublishTelemetry()))
  {
    Logger.Error("Telemetry send failed");
  }

  next_telemetry_send_time_ms = now + TELEMETRY_FREQUENCY_MILLISECS;
}
