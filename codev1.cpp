/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

String outputState = "apagado";

#define enableA 4
#define motorA1 5
#define motorA2 18

#define enableB 2
#define motorB1 19
#define motorB2 22

char terminated;

int pos = 0;
int pinServo = 15;

int TR = 21;
int ECHO = 23;
int DISTANCIA;
int DURACION;

Servo servo_15;


int mover = 1;

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0) {
      char req = value[0];
      Serial.println(req);
      /*********/
      terminated = 'false';
      digitalWrite(enableA, HIGH);
      digitalWrite(enableB, HIGH);
      //se realizan validaciones para la acción que va ejecutar
      //cuando es 1 avanzara
      if (req == '1') {
        mover(1);
        terminated = 'true';
      } else if (req == '2') {
        mover(2);
        delay(300);
        stop();
        terminated = 'true';
      } else if (req == '3') {
        mover(3);
        delay(300);
        stop();
        terminated = 'true';
      } else if (req == '4') {
        mover(4);
        terminated = 'true';
      } else if (req == '0') {
        stop();
        terminated = 'true';
      } else {
        stop();
        terminated = 'true';
      }
    }
  }

  void mover(int direccion) {
    switch (direccion) {

      case 1:  // Hacia delante
        Serial.println("Hacia delante el motor");
        digitalWrite(motorA1, LOW);
        digitalWrite(motorA2, HIGH);
        digitalWrite(motorB1, LOW);
        digitalWrite(motorB2, HIGH);
        break;

      case 2:  // Hacia la derecha
        Serial.println("Hacia DERECHA el motor");
        digitalWrite(motorA1, HIGH);
        digitalWrite(motorA2, LOW);
        digitalWrite(motorB1, LOW);
        digitalWrite(motorB2, HIGH);
        break;

      case 3:  // Hacia la izquierda
        Serial.println("Hacia IZQUIERDA el motor");
        digitalWrite(motorA1, LOW);
        digitalWrite(motorA2, HIGH);
        digitalWrite(motorB1, HIGH);
        digitalWrite(motorB2, LOW);
        break;

      case 4:  // Hacia atrás
        Serial.println("Hacia atrás el motor");
        digitalWrite(motorA1, HIGH);
        digitalWrite(motorA2, LOW);
        digitalWrite(motorB1, HIGH);
        digitalWrite(motorB2, LOW);
        break;
    }
  }

  void stop() {
    Serial.println(" PARAMOS motor");
    digitalWrite(enableA, LOW);
    digitalWrite(enableB, LOW);
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, LOW);
  }
};


void setup() {
  Serial.begin(115200);

  terminated = 'true';

  servo_15.attach(pinServo, 500, 25000);

  pinMode(TR, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(enableA, OUTPUT);
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(enableB, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);

  BLEDevice::init("MyCarController");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  // put your main code here, to run repeatedly:
  // sweep the servo from 0 to 180 degrees in steps
  // of 1 degrees

  digitalWrite(TR, HIGH);
  delay(1000);
  digitalWrite(TR, LOW);
  DURACION = pulseIn(ECHO, HIGH);
  DISTANCIA = DURACION / 58.2;
  Serial.println(DISTANCIA);
  delay(100);
  //Asignar rango de medicion;
  if (DISTANCIA <= 1260 && DISTANCIA >= 35) {
    Serial.println("Hacia delante el motor");
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, HIGH);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, HIGH);

  } else {
    //delay(DISTANCIA * 10);
    //----- apagamos
    digitalWrite(enableA, LOW);
    digitalWrite(enableB, LOW);
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, LOW);
  }
  Serial.println(terminated);
  if (3 > 6) {
    for (pos = 0; pos <= 180; pos++) {
      // tell servo to go to position in variable 'pos'
      servo_15.write(pos);
      // wait 15 ms for servo to reach the position
      delay(15);  // Wait for 15 millisecond(s)
    }
    for (pos = 180; pos >= 0; pos -= 1) {
      // tell servo to go to position in variable 'pos'
      servo_15.write(pos);
      // wait 15 ms for servo to reach the position
      delay(15);  // Wait for 15 millisecond(s)
    }
  }
}
