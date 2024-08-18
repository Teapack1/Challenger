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
  int joy2_x;
  int joy2_y;
} struct_message;

struct_message receivedData;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
}

int mapSteeringPosition(int joy_value) {
  // Reversed steering, offset to the left, and increased range
  int reversedValue = -joy_value;
  int offsetValue = reversedValue - 30; // Adjust this value to fine-tune the left offset
  return map(offsetValue, -255, 255, 30, 150); // Increased range and offset
}

int mapThrottlePosition(int joy_value) {
  if (joy_value < -40) {
    // Map reverse throttle, full reverse at joy_value = -300, neutral at joy_value = -40
    return map(joy_value, -300, -40, 0, 90); // Reverse throttle
  } else if (joy_value > 40) {
    // Map forward throttle, neutral at joy_value = 40, full forward at joy_value = 205
    return map(joy_value, 40, 205, 90, 180); // Forward throttle
  } else {
    // Within -40 to 40 range, the throttle should stay at neutral
    return 90; // Neutral throttle
  }
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
  int throttlePosition = mapThrottlePosition(receivedData.joy2_y);

  steeringServo.write(steeringPosition);
  esc.write(throttlePosition);

  Serial.print("Steering: ");
  Serial.print(receivedData.joy1_x);
  Serial.print(" -> ");
  Serial.println(steeringPosition);

  Serial.print("Throttle: ");
  Serial.print(receivedData.joy2_y);
  Serial.print(" -> ");
  Serial.println(throttlePosition);

  delay(20);
}