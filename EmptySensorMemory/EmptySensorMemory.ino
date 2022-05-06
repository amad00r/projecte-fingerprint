/**
 * @file Fingerprint.ino
 * @author Amadeu Moya (amadeumosar@gmail.com) i Jordi Granja
 * @brief Petit script per a esborrar ràpidament la memòria del sensor.
 * @version 0.1
 * @date 2022-05-06
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

void setup() {
    Serial.begin(9600);
    while(!Serial);
    delay(100);

    Serial.println("\n\nBuscant el sensor...\n\n");

    finger.begin(57600);

    bool verify = finger.verifyPassword();
    while(verify != true) {
        Serial.println("No s'ha trobat el sensor :(");
        delay(1000);
    }
    Serial.println("S'ha trobat el sensor!\n\n");

    int empty_verify = finger.emptyDatabase();
    while(empty_verify != FINGERPRINT_OK) {
        Serial.println("No s'ha pogut esborrar la memòria :(");
        delay(1000);
    }
    Serial.println("S'han esborrat les empremtes guardades!\n\n");
}

void loop() {} //Res