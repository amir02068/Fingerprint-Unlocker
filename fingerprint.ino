#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <BleKeyboard.h>

HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

struct User {
  uint8_t id;
  const char* name;
  const char* password;
};

const User users[] = {
  {1, "Admin", "12345678"}, //you can write your password here
  {2, "User", "11111111"},
  {3, "Guest", "00000000"}
};
const int userCount = sizeof(users) / sizeof(User);

BleKeyboard bleKeyboard("ESP32-FingerKey", "ESP32", 100);

void setup() {
  Serial.begin(115200);
  fingerSerial.begin(57600, SERIAL_8N1, 16, 17);  // RX = GPIO16, TX = GPIO17
  bleKeyboard.begin();

  finger.begin(57600);
  finger.getTemplateCount();
  Serial.print("Registered users: ");
  Serial.println(finger.templateCount);
}

void loop() {
  static uint32_t lastSendTime = 0;
  
  if (millis() - lastSendTime > 100) {
    uint8_t p = finger.getImage();
    if (p == FINGERPRINT_OK) {
      p = finger.image2Tz();
      if (p == FINGERPRINT_OK) {
        p = finger.fingerFastSearch();
        if (p == FINGERPRINT_OK) {
          sendPasswordWithEnter(finger.fingerID);
          lastSendTime = millis();
        }
      }
    }
  }
  delay(10);
}

void sendPasswordWithEnter(uint8_t fingerID) {
  if (!bleKeyboard.isConnected()) {
    Serial.println("Bluetooth not connected");
    return;
  }

  for (int i = 0; i < userCount; i++) {
    if (users[i].id == fingerID) {
      Serial.print("Sending for: ");
      Serial.println(users[i].name);
      
      bleKeyboard.releaseAll();
      bleKeyboard.write(KEY_RETURN);
      delay(400);
      
      bleKeyboard.print(users[i].password);
      bleKeyboard.write(KEY_RETURN);

      Serial.println("Password sent ✔ ");
      return;
    }
  }
  Serial.println("User not found ❌ ");
}
