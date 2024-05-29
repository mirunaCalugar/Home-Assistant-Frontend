#include <EEPROM.h>
#include <Ethernet.h> // Biblioteca Ethernet pentru a trimite e-mailuri

#define LED_PIN 13
#define TEMPERATURE_SENSOR_PIN A0
#define HUMIDITY_SENSOR_PIN A1
#define WATER_LEVEL_SENSOR_PIN A2

#define EVENT_START_ADDR 0        // Adresa de început a EEPROM pentru evenimente
#define EVENT_MAX_LENGTH 50       // Lungimea maximă a fiecărui eveniment (inclusiv data și ora)
#define MAX_EVENTS 10             // Numărul maxim de evenimente de stocat

#define MESSAGE_START_ADDR 320    // Adresa de început a EEPROM pentru mesaje
#define MESSAGE_MAX_LENGTH 32     // Lungimea maximă a fiecărui mesaj
#define MAX_MESSAGES 10           // Numărul maxim de mesaje de stocat

#define WATER_LEVEL_THRESHOLD 30

EthernetClient client;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600); // Inițializarea comunicării seriale la o rată de 9600 de biți pe secundă
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();

    switch (command) {
      case 'A':
        digitalWrite(LED_PIN, HIGH);
        break;
      case 'S':
        digitalWrite(LED_PIN, LOW);
        break;
      case 'R': {
        int tempSensorValue = analogRead(TEMPERATURE_SENSOR_PIN);
        float tempVoltage = tempSensorValue * (5.0 / 1023.0);
        float temperature = (tempVoltage - 0.5) * 100;

        int humiditySensorValue = analogRead(HUMIDITY_SENSOR_PIN);
        float simulatedHumidity = map(humiditySensorValue, 0, 1023, 0, 100);

        int waterLevelSensorValue = analogRead(WATER_LEVEL_SENSOR_PIN);
        float waterLevel = map(waterLevelSensorValue, 0, 1023, 0, 100);
         Serial.print(temperature);
        Serial.print(",");
        Serial.print(simulatedHumidity);
        Serial.print(",");
        Serial.println(waterLevel);
        

        // Preia data și ora curentă
        unsigned long currentMillis = millis();
        unsigned long secondsSinceEpoch = currentMillis / 1000;
        unsigned long minutesSinceEpoch = secondsSinceEpoch / 60;
        unsigned long hoursSinceEpoch = minutesSinceEpoch / 60;
        unsigned long daysSinceEpoch = hoursSinceEpoch / 24;
        unsigned long currentSeconds = secondsSinceEpoch % 60;
        unsigned long currentMinutes = minutesSinceEpoch % 60;
        unsigned long currentHours = hoursSinceEpoch % 24;

        String currentTime = String(daysSinceEpoch) + "-" + String(currentHours) + ":" + String(currentMinutes) + ":" + String(currentSeconds);

        // Verificarea condiției de inundație
        if (waterLevel > WATER_LEVEL_THRESHOLD) {
          String event = currentTime + "," + String(temperature) + "," + String(simulatedHumidity) + "," + String(waterLevel);
          storeEvent(event);
          //Serial.println("Flood Detected");
          //sendEmailNotification(temperature, simulatedHumidity, waterLevel); // Trimiteți notificare prin e-mail
        }
        break;
      }
      case 'E': {
        String event = Serial.readStringUntil('\n');
        storeEvent(event);
        break;
      }
      case 'M': {
        // Citiți evenimentele stocate din EEPROM
        readStoredEvents();
        break;
      }
      case 'U': {
        String message = Serial.readStringUntil('\n');
        storeMessage(message);
        break;
      }
      case 'G': {
        // Citiți mesajele stocate din EEPROM
        readStoredMessages();
        break;
      }
      case 'D': {
        int index = Serial.parseInt();
        deleteEvent(index);
        break;
      }
    }
  }
  delay(1000);
}

void storeEvent(String event) {
  static int eventIndex = 0;
  int startAddr = EVENT_START_ADDR + eventIndex * EVENT_MAX_LENGTH;
  eventIndex = (eventIndex + 1) % MAX_EVENTS; // Mențineți indexul în intervalul 0-9

  for (int i = 0; i < EVENT_MAX_LENGTH; i++) {
    if (i < event.length()) {
      EEPROM.write(startAddr + i, event[i]);
    } else {
      EEPROM.write(startAddr + i, 0); // Completați spațiile rămase cu caractere nule
    }
  }
}

void storeMessage(String message) {
  static int messageIndex = 0;
  int startAddr = MESSAGE_START_ADDR + messageIndex * MESSAGE_MAX_LENGTH;
  messageIndex = (messageIndex + 1) % MAX_MESSAGES; // Mențineți indexul în intervalul 0-9

  for (int i = 0; i < MESSAGE_MAX_LENGTH; i++) {
    if (i < message.length()) {
      EEPROM.write(startAddr + i, message[i]);
    } else {
      EEPROM.write(startAddr + i, 0); // Completați spațiile rămase cu caractere nule
    }
  }
}

void readStoredEvents() {
  for (int i = 0; i < MAX_EVENTS; i++) {
    int startAddr = EVENT_START_ADDR + i * EVENT_MAX_LENGTH;
    String event = "";
    for (int j = 0; j < EVENT_MAX_LENGTH; j++) {
      char c = EEPROM.read(startAddr + j);
      if (c == 0
) {
        break;
      }
      event += c;
    }
    if (event.length() > 0) {
      Serial.println(event);
    }
  }
}

void readStoredMessages() {
  for (int i = 0; i < MAX_MESSAGES; i++) {
    int startAddr = MESSAGE_START_ADDR + i * MESSAGE_MAX_LENGTH;
    String message = "";
    for (int j = 0; j < MESSAGE_MAX_LENGTH; j++) {
      char c = EEPROM.read(startAddr + j);
      if (c == 0) break;
      message += c;
    }
    if (message.length() > 0) {
      Serial.println(message);
    }
  }
}

void deleteEvent(int index) {
  if (index >= 0 && index < MAX_EVENTS) {
    int startAddr = EVENT_START_ADDR + index * EVENT_MAX_LENGTH;
    for (int i = 0; i < EVENT_MAX_LENGTH; i++) {
      EEPROM.write(startAddr + i, 0);
    }
  }
}
