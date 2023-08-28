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

void setup() {
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

    while (!finger.verifyPassword()) {
        Serial.println("Did not find fingerprint sensor :(");
        delay(1000)
    }
    Serial.println("Found fingerprint sensor!");

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x"));       Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x"));       Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: "));       Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: "));     Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: "));      Serial.println(finger.baud_rate);
}


uint8_t getFingerprintEnroll() {
    Serial.println("Ready to enroll a fingerprint!");
    Serial.print("Enrolling ID #");
    Serial.println(id);

    Serial.print("Waiting for valid finger to enroll as #"); 
    Serial.println(id);
    
    int p;

    while ((p = finger.getImage()) != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_NOFINGER)         Serial.println(".");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_IMAGEFAIL)        Serial.println("Imaging error");
        else                                        Serial.println("Unknown error");

        if (digitalRead(7) == LOW) return false;
    }
    Serial.println("Image taken");

    p = finger.image2Tz(1);
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_IMAGEMESS)        Serial.println("Image too messy");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_FEATUREFAIL)      Serial.println("Could not find fingerprint features");
        else if (p == FINGERPRINT_INVALIDIMAGE)     Serial.println("Could not find fingerprint features");
        else if (p != FINGERPRINT_OK)               Serial.println("Unknown error");
        return false;
    }
    Serial.println("Image converted\nRemove finger");

    delay(2000);

    while (finger.getImage() != FINGERPRINT_NOFINGER)
        if (digitalRead(7) == LOW) return false;


    Serial.print("ID "); Serial.println(id);
    Serial.println("Place same finger again");

    while ((p = finger.getImage()) != FINGERPRINT_OK) {
        if (p == FINGERPRINT_NOFINGER) Serial.print(".");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_IMAGEFAIL) Serial.println("Imaging error");
        else Serial.println("Unknown error");
        
        if (digitalRead(7) == LOW) return false;
    }
    Serial.println("Image taken");


    p = finger.image2Tz(2);
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_IMAGEMESS)        Serial.println("Image too messy");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_FEATUREFAIL)      Serial.println("Could not find fingerprint features");
        else if (p == FINGERPRINT_INVALIDIMAGE)     Serial.println("Could not find fingerprint features");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Image converted");


    Serial.print("Creating model for #"); Serial.println(id);

    p = finger.createModel();
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_ENROLLMISMATCH)   Serial.println("Fingerprints did not match");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Prints matched!");


    Serial.print("ID "); Serial.println(id);

    p = finger.storeModel(id);
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_BADLOCATION)      Serial.println("Could not store in that location");
        else if (p == FINGERPRINT_FLASHERR)         Serial.println("Error writing to flash");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Stored!");

    return true;
}


uint8_t getFingerprintID() {
    int p = finger.getImage();
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_NOFINGER)         Serial.println("No finger detected");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_IMAGEFAIL)        Serial.println("Imaging error");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Image taken");

    // OK success!

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_IMAGEMESS)        Serial.println("Image too messy");
        else if (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_FEATUREFAIL)      Serial.println("Could not find fingerprint features");
        else if (p == FINGERPRINT_INVALIDIMAGE)     Serial.println("Could not find fingerprint features");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Image converted");
    

    p = finger.fingerSearch();
    if (p != FINGERPRINT_OK) {
        if      (p == FINGERPRINT_PACKETRECIEVEERR) Serial.println("Communication error");
        else if (p == FINGERPRINT_NOTFOUND)         Serial.println("Did not find a match");
        else                                        Serial.println("Unknown error");
        return false;
    }
    Serial.println("Found a print match!");
  

    Serial.print("Found ID #"); Serial.print(finger.fingerID); Serial.print(" with confidence of "); Serial.println(finger.confidence);
    return true;
}

void loop() {
    Serial.println("S'ha canviat el mode a detectar empremtes");
    digitalWrite(9, HIGH);
    digitalWrite(10, LOW);
    
    while (digitalRead(7) == LOW) {
        if (getFingerprintID()) {
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
        if (getFingerprintEnroll()) {
            Serial.println("S'ha emmagatzemat una empremta!");
            ++id;

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