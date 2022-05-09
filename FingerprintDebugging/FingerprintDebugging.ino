/**
 * @file Fingerprint_debug.ino
 * @author Amadeu Moya (amadeumosar@gmail.com) i Jordi Granja
 * @brief Codi per a la depuració i el control d'errors del sistema.
 * @version 0.1
 * @date 2022-05-09
 * 
 * https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
 */


#include <Adafruit_Fingerprint.h> 

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 


int id;

void setup()
{
  pinMode(5, OUTPUT);
  pinMode(7, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  finger.begin(57600);

  finger.getTemplateCount();
  id = finger.templateCount + 1;

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}


uint8_t getFingerprintEnroll() {

  int p = -1;

  Serial.println("Ready to enroll a fingerprint!");
  Serial.print("Enrolling ID #");
  Serial.println(id);

  Serial.print("Waiting for valid finger to enroll as #"); 
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
    if (digitalRead(7) == LOW) {
      return false;
    }
  }

  if (digitalRead(7) == HIGH) {
    p = finger.image2Tz(1);
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println("Image too messy");
        return p;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println("Could not find fingerprint features");
        return p;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println("Could not find fingerprint features");
        return p;
      default:
        Serial.println("Unknown error");
        return p;
    }

    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FINGERPRINT_NOFINGER) {
      p = finger.getImage();
      if (digitalRead(7) == LOW) {
        return false;
      }
    }
    if (digitalRead(7) == HIGH) {
      Serial.print("ID "); Serial.println(id);
      p = -1;
      Serial.println("Place same finger again");
      while (p != FINGERPRINT_OK) {
        p = finger.getImage();
        switch (p) {
          case FINGERPRINT_OK:
            Serial.println("Image taken");
            break;
          case FINGERPRINT_NOFINGER:
            Serial.print(".");
            break;
          case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            break;
          case FINGERPRINT_IMAGEFAIL:
            Serial.println("Imaging error");
            break;
          default:
            Serial.println("Unknown error");
            break;
        }
        if (digitalRead(7) == LOW) {
          return false;
        }
      }

      if (digitalRead(7) == HIGH) {
        p = finger.image2Tz(2);
        switch (p) {
          case FINGERPRINT_OK:
            Serial.println("Image converted");
            break;
          case FINGERPRINT_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
          case FINGERPRINT_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
          case FINGERPRINT_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
          case FINGERPRINT_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
          default:
            Serial.println("Unknown error");
            return p;
        }

        Serial.print("Creating model for #");  Serial.println(id);

        p = finger.createModel();
        if (p == FINGERPRINT_OK) {
          Serial.println("Prints matched!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
          Serial.println("Communication error");
          return p;
        } else if (p == FINGERPRINT_ENROLLMISMATCH) {
          Serial.println("Fingerprints did not match");
          return p;
        } else {
          Serial.println("Unknown error");
          return p;
        }

        Serial.print("ID "); Serial.println(id);
        p = finger.storeModel(id);
        if (p == FINGERPRINT_OK) {
          Serial.println("Stored!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
          Serial.println("Communication error");
          return p;
        } else if (p == FINGERPRINT_BADLOCATION) {
          Serial.println("Could not store in that location");
          return p;
        } else if (p == FINGERPRINT_FLASHERR) {
          Serial.println("Error writing to flash");
          return p;
        } else {
          Serial.println("Unknown error");
          return p;
        }

        return true;
      }
    }
  }
}


// Esto es del fingerprint
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return true;
}

void loop() {

  Serial.println("S'ha canviat el mode a detectar empremtes");
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
 
  while (digitalRead(7) == LOW) {
    if (getFingerprintID() == true) {
      Serial.println("S'ha detectat una empremta!");
      digitalWrite(5, HIGH);
      delay(2500);
      digitalWrite(5, LOW);

      digitalWrite(9, LOW);
      delay(250);            
      digitalWrite(9, HIGH);  
      delay(250);
      digitalWrite(9, LOW);
      delay(250);            
      digitalWrite(9, HIGH);
    }
    delay(100);
  }

  Serial.println("S'ha canviat el mode a guardar empremtes");
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);

  while (digitalRead(7) == HIGH) {
    if (getFingerprintEnroll() == true) {
      Serial.println("S'ha emmagatzemat una empremta!");
      id += 1;

      digitalWrite(10, LOW);
      delay(250);            
      digitalWrite(10, HIGH);  
      delay(250);
      digitalWrite(10, LOW);
      delay(250);            
      digitalWrite(10, HIGH);
    }
  }
}