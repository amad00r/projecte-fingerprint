/**
 * @file Fingerprint.ino
 * @author Amadeu Moya (amadeumosar@gmail.com) i Jordi Granja
 * @brief Projecte de tecnologia de 2n bat amb Arduino per a l'obertura de la porta del laboratori Edison amb un sensor d'empremtes dactilars.
 * @version 0.1
 * @date 2022-05-06
 * 
 * https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
 */


//Importem el codi que ens proporciona el fabricant per a treballar amb el seu sensor. Cal tenir-lo descarregat.
#include <Adafruit_Fingerprint.h> 

//Es defineix el SoftwareSerial o HardwareSerial depenent del nostre model d'Arduino.
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif

//Objecte instanciat a partir de la classe Adafruit_Fingerprint.
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); 


//Emmagatzema un nombre sencer que indica quina ID assignar a la propera empremta que vulguem guardar en la memòria del sensor.
int id;

void setup() {
  /*
   * Definim el pin 5 com a sortida que donarà senyal al relé, que deixarà passar CC 12V a l'electroimant que obre la porta.
   * Definim el pin 7 com a entrada de senyal connectat al commutador per a determinar si el sistema es troba en mode de detectar o guardar empremtes.
   * Definim el pin 9 com a sortida de senyal que encen un díode LED.
   * Definim el pin 10 com a sortida de senyal que encen l'altre díode LED.
   */
  pinMode(5, OUTPUT);
  pinMode(7, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  finger.begin(57600);

  /**
   * Compta el nombre d'empremtes emmagatzemades al sensor i el guarda a la variable templateCount de l'objecte finger.
   * Amb aquest valor, calculem l'ID de la propera empremta i el guardem a la variable global id.
   */
  finger.getTemplateCount();
  id = finger.templateCount + 1;
}

/**
 * @brief Revisa si el valor de p és FINGERPRINT_OK, o no.
 *
 * @param p Variable que conté una senyal de confirmació o error retornada per alguna funció de l'objecte finger.
 * @return true si p conté FINGERPRINT_OK, i false si conté qualsevol altra senyal.
 */
bool check_p(uint8_t p) { 
  if (p == FINGERPRINT_OK) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Revisa el valor del pin físic 7 de la placa Arduino.
 *
 * @return true si digitalRead(7) == HIGH, i false si digitalRead(7) == LOW.
 */
bool check_pin7() {
  if (digitalRead(7) == HIGH) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Realitza tot el procés d'inscriure una nova empremta al sensor.
 *
 * @return true si ha tingut éxit, i false si no n'ha tingut o ha succeït algun error.
 */
uint8_t fingerprint_enroll() {
  /**
   * Variable privada de la funció que emmagatzema les senyals de confirmació i error de les 
   * diferents funcions de l'objecte finger que s'executaran.
   */
  int p = -1;

  /**
   * Es repeteix el bucle fins que el valor de la variable p sigui una senyal de confirmació de que ha detectat una empremta.
   * Després, converteix l'imatge presa i la converteix en un mapa de característiques de l'empremta. 
   * Aquesta és la primera mostra de dues -> (ho hem d'indicar al paràmetre de la funció image2Tz(slot)) 
   * que necessitem per a crear el model de l'empremta que volem introduir a la memòria del sensor.
   */
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (check_pin7() == false) { return false; }
  }
  p = finger.image2Tz(1);
  if (check_p(p) == false) { return false; }

  /**
   * Quan s'ha registrat una primera vegada l'empremta hem de treure el dit per a registrar-la una segona vegada.
   * Aquest bucle no acaba fins que deixa de detectar un dit al sensor.
   */
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
    if (check_pin7() == false) { return false; }
  }

  /**
   * De la mateixa manera que abans, esperem fins que el sensor detecta una dit i posteriorment convertim la seva imatge
   * en un mapa de característiques, que en aquest cas serà el segon.
   */
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (check_pin7() == false) { return false; }
  }
  p = finger.image2Tz(2);
  if (check_p(p) == false) { return false; }

  /**
   * Ara que ja tenim els dos mapes de característiques, creem el model de l'empremta i el guardem en la memòria del 
   * sensor amb l'ID que hem calculat anteriorment passant-lo com a paràmetre a la funció storeModel(id).
   */
  p = finger.createModel();
  if (check_p(p) == false) { return false; }
  p = finger.storeModel(id);
  if (check_p(p) == false) { return false; }

  return true;
}

/**
 * @brief Llegeix el sensor i comprova si les dades coincideixen amb les d'alguna empremta registrada a la seva memòria.
 *
 * @return true si l'empremta llegida existeix a la memòria, i false si no està registrada o ha succeït alguna mena d'error.
 */
uint8_t fingerprint_check() {
  //Llegeix les dades del sensor, i, si detecta alguna empremta, en crea un mapa de característiques.
  uint8_t p = finger.getImage();
  if (check_p(p) == false) { return false; }
  p = finger.image2Tz();
  if (check_p(p) == false) { return false; }

  /**
   * Comparem el mapa de característiques que acabem de crear i el comparem amb els dels models emmagatzemats a la 
   * memòria del sensor per a trobar alguna coincidència.
   */
  p = finger.fingerSearch();
  if (check_p(p) == false) { return false; }

  return true;
}


void loop() {
  //El LED amb pin 9 s'encén i el de pin 10 s'apaga.
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);

  /**
   * Quan el pin 7 detecti entrada de senyal, sortirem d'aquest bucle i passarem d'executar 
   * fingerprint_check() a executar fingerprint_enroll().
   */
  while (check_pin7() == false) {
    /**
     * Si fingerprint_check() detecta alguna coincidència, dona senyal al relé durant 2.5 segons i,
     * després, fa pampallugar el LED amb pin 9 dues vegades en intervals de 0.25 segons.
     */
    if (fingerprint_check() == true) {
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

  //El LED amb pin 9 s'apaga i el de pin 10 s'encén.
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);

  /**
   * Quan el pin 7 deixi de detectar entrada de senyal, sortirem d'aquest bucle i passarem d'executar 
   * fingerprint_enroll() a executar fingerprint_check().
   */
  while (check_pin7() == true) {
    /**
     * Si fingerprint_check() emmagatzema amb éxit una empremta, incrementa ID en 1 unitat per a la propera empremta, 
     * i, després, fa pampallugar el LED amb pin 10 dues vegades en intervals de 0.25 segons.
     */
    if (fingerprint_enroll() == true) {
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