#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "elegantota.h"

// Variables for 74HC595 code
#define SER_Pin 14         // Serial Input pin on 74HC595 No 1 (74HC595 Pin 14). Otherwise known as DS
#define RCLK_Pin 12        // Shift Register Clock Pin on both 74HC595s (74HC595 Pin 12). Otherwise known as ST_CP
#define SRCLK_Pin 13       // Storage Register Clock Pin on both 74HC595s (74HC595 Pin 11). Otherwise known as SH_CP
#define outout_enablePin 5 // Output Enable pin on both 74HC595s (74HC595 Pin 13). Must be pulled LOW to enable the outputs

typedef struct struct_message
{
  int a;
  bool b;
} struct_message;

struct_message myData;
boolean registers[16];                                             // Zero-indexed array (0-15) which holds the state of the 16 relays
uint8_t broadcastAddress[] = {0x98, 0x88, 0xe0, 0x04, 0xe2, 0x48}; // 98:88:e0:04:e2:48  экран
esp_now_peer_info_t peerInfo;
uint32_t ping_timer;
extern boolean update;

void send_command(int relay, bool state)
{
  struct_message msg;
  msg.a = relay;
  msg.b = state;
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&msg, sizeof(msg));

  if (result != ESP_OK)
  {
    Serial.println("Error sending the data");
  }
}

void OnDataSent(const esp_now_recv_info_t *info, esp_now_send_status_t status)
{
  if (status == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("ok");
  }
  else
  {
    Serial.println(" - не выполнено (сброс выходов)");
    // Если связь потеряна, сбрасываем всё для безопасности
    digitalWrite(RCLK_Pin, LOW);
    for (int i = 15; i >= 0; i--)
    {
      registers[i] = LOW; // ОБЯЗАТЕЛЬНО обновляем состояние в памяти
      digitalWrite(SRCLK_Pin, LOW);
      digitalWrite(SER_Pin, LOW);
      digitalWrite(SRCLK_Pin, HIGH);
    }
    digitalWrite(RCLK_Pin, HIGH);
  }
}

// callback function that will be executed when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
{
  memcpy(&myData, incomingData, sizeof(myData));
  
  // ИСПРАВЛЕНО: используем && и проверяем ID реле
  if (myData.a >= 0 && myData.a <= 15)
  {
    Serial.printf("Зона: %d, Состояние: %s\n", myData.a, myData.b ? "вкл" : "выкл");
    
    digitalWrite(RCLK_Pin, LOW);
    registers[myData.a] = myData.b;
    for (int i = 15; i >= 0; i--)
    {
      digitalWrite(SRCLK_Pin, LOW);
      digitalWrite(SER_Pin, registers[i]);
      digitalWrite(SRCLK_Pin, HIGH);
    }
    digitalWrite(RCLK_Pin, HIGH);
  }
  else if (myData.a == 255)
  {
    update = true;
    Serial.println("OTA mode requested");
  }
}

void setup()
{
  for (int i = 15; i >= 0; i--)
  {
    registers[i] = LOW;
  }
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  pinMode(outout_enablePin, OUTPUT);
  digitalWrite(outout_enablePin, LOW);
  // Initialize Serial Monitor
  Serial.begin(115200);
  // delay (10000);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.enableAP(false);
  delay(1000);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  if (millis() - ping_timer > 1000)
  {
    send_command(0, false);
    ping_timer = millis();
  }
  ota_loop();
}
