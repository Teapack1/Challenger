#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// Define pins
#define STEERING_SERVO_PIN 18
#define ESC_PIN 19

Servo steeringServo;
Servo esc;

typedef struct struct_message {
  int joy1_x;
  int joy1_y;
} struct_message;

struct_message receivedData;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
}

int mapSteeringPosition(int joy_value) {
  return map(joy_value, -255, 255, 0, 180);
}

int mapThrottlePosition(int joy_value) {
  return map(joy_value, -255, 255, 0, 180);
}

void setup() {
  Serial.begin(115200);

  steeringServo.attach(STEERING_SERVO_PIN);
  esc.attach(ESC_PIN);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  int steeringPosition = mapSteeringPosition(receivedData.joy1_x);
  int throttlePosition = mapThrottlePosition(receivedData.joy1_y);

  steeringServo.write(steeringPosition);
  esc.write(throttlePosition);

  Serial.print("Steering: ");
  Serial.print(receivedData.joy1_x);
  Serial.print(" -> ");
  Serial.println(steeringPosition);

  Serial.print("Throttle: ");
  Serial.print(receivedData.joy1_y);
  Serial.print(" -> ");
  Serial.println(throttlePosition);

  delay(50);
}
