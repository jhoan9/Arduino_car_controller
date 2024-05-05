/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleWrite.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <ESP32Servo.h>
#include <dht11.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define enableA 4
#define motorA1 5
#define motorA2 18

#define enableB 2
#define motorB1 19
#define motorB2 22

int LedTrasera = 35;

//variable para el uso del servomotor
int pos = 0;
int i = 0;
int pinServo = 15;

int TR = 21;
int ECHO = 23;
int DISTANCIA;
int DURACION;

Servo servo_15;

/*
  importación de libreria de temperatura y
  pin que usara el sensor
*/
dht11 DHT11;
#define DHT11PIN 13

//variable global que contiene el valor de las peticiones
char req = '0';
char result;

//variable int que definira a donde girara el carro
int mover = 1;
int turnTime = 2000;

/*
  variable que almacenará el valor de la temperatura
*/
float temperatura = 0.0;
/*
  se crea esta clase para escuchar las peticiones que realiza el usuario
  mediante la conexion bluetooth
*/
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    //se captura el valor que envio el usuario
    std::string value = pCharacteristic->getValue();

    //se valida que tenga valores el dato enviado
    if (value.length() > 0) {

      // el valor llega en formato array, y se extrae la posicion 0
      req = value[0];

      //se habilitan los motores
      digitalWrite(enableA, HIGH);
      digitalWrite(enableB, HIGH);

      int chk = 0;
      std::string response = "";
      //se realizan validaciones para la acción que va ejecutar
      switch (req) {
        case '1':
          mover(1);
          break;
        case '2':
          mover(2);
          delay(400);
          stop();
          break;
        case '3':
          mover(3);
          delay(400);
          stop();
          break;
        case '4':
          mover(4);
          break;
        case '0':
          stop();
          break;
        case '8':
          //se habilita modo automatico
          result = 'enabled';
          break;
        case '9':
          //se habilita modo manual
          result = 'disabled';
          break;
        case '5':
          Serial.println("Iniciando escaneo de tempeartura ");
          Serial.print(req);

          //se habilita modo senso de temperatura

          //se lee el valor de temperatura y se realizan validaciones
          //para identificar que sucedió con la lectura
          delay(1000);
          chk = DHT11.read(DHT11PIN);
          switch (chk) {
            case DHTLIB_OK:
              Serial.println("Correcto");
              response = "Correcto";
              temperature(pCharacteristic, response);
              break;
            case DHTLIB_ERROR_CHECKSUM:
              Serial.println("Error de datos");
              response = "Error de datos";
              temperatureFailure(pCharacteristic, response);
              break;
            case DHTLIB_ERROR_TIMEOUT:
              Serial.println("Error de tiempo de espera");
              response = "Error de tiempo de espera";
              temperatureFailure(pCharacteristic, response);
              break;
            default:
              Serial.println("Error desconocido");
              response = "Error desconocido";
              temperatureFailure(pCharacteristic, response);
              break;
          }
          break;
        default:
          req = 'go';
          stop();
          break;
      }
    }
  }

  void temperature(BLECharacteristic *pCharacteristic, std::string response) {
    Serial.print("Humedad (%): ");
    Serial.println((float)DHT11.humidity, 1);

    Serial.print("Temperatura (C): ");
    Serial.println((float)DHT11.temperature, 1);
    temperatura = (float)DHT11.temperature;

    //se convierte a string el valor float
    char buffer[10];
    itoa(temperatura, buffer, 10);
    response = buffer;
    temperatura = 0;

    Serial.print("*** ");
    Serial.println(response.c_str());

    //se envia el valor que se valido
    pCharacteristic->setValue(response);
    //pCharacteristic->notify();  // Notifica al cliente que hay nuevos datos disponibles
  }

  void temperatureFailure(BLECharacteristic *pCharacteristic, std::string res) {
    pCharacteristic->setValue(res);
    //pCharacteristic->notify();  // Notifica al cliente que hay nuevos datos disponibles
  }

  //función que valida que acción tomar
  void mover(int direccion) {
    switch (direccion) {

      case 1:  // Hacia delante
        Serial.println("Hacia delante el motor");
        digitalWrite(motorA1, LOW);
        digitalWrite(motorA2, HIGH);
        digitalWrite(motorB1, LOW);
        digitalWrite(motorB2, HIGH);
        digitalWrite(LedTrasera, LOW);
        break;

      case 2:  // Hacia la derecha
        Serial.println("Hacia DERECHA el motor");
        digitalWrite(motorA1, HIGH);
        digitalWrite(motorA2, LOW);
        digitalWrite(motorB1, LOW);
        digitalWrite(motorB2, HIGH);
        digitalWrite(LedTrasera, LOW);
        break;

      case 3:  // Hacia la izquierda
        Serial.println("Hacia IZQUIERDA el motor");
        digitalWrite(motorA1, LOW);
        digitalWrite(motorA2, HIGH);
        digitalWrite(motorB1, HIGH);
        digitalWrite(motorB2, LOW);
        digitalWrite(LedTrasera, LOW);
        break;

      case 4:  // Hacia atrás
        Serial.println("Hacia atrás el motor");
        digitalWrite(motorA1, HIGH);
        digitalWrite(motorA2, LOW);
        digitalWrite(motorB1, HIGH);
        digitalWrite(motorB2, LOW);
        delay(1000);
        digitalWrite(LedTrasera, HIGH);
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
    digitalWrite(LedTrasera, LOW);
  }
};


void setup() {
  Serial.begin(115200);

  Serial.print('Iniciando.....');

  int chk = DHT11.read(DHT11PIN);

  servo_15.attach(pinServo, 500, 25000);

  //se definen los pines de salida y entrada
  pinMode(TR, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(enableA, OUTPUT);
  pinMode(motorA1, OUTPUT);
  pinMode(motorA2, OUTPUT);
  pinMode(enableB, OUTPUT);
  pinMode(motorB1, OUTPUT);
  pinMode(motorB2, OUTPUT);
  pinMode(LedTrasera, OUTPUT);


  //se asigna el nombre del dispositivo
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

  // si el valor es 8, el carro es autonomo
  if (req == '8') {
    bool move = rideCar();
    if (!move) {
      int gradosPos = 180;
      int newDistancia = 0;
      int grado = 0;

      for (pos = 0; pos <= gradosPos; pos += 2) {
        // tell servo to go to position in variable 'pos'
        //por cada posición se realiza la medición de distancia
        servo_15.write(pos);
        digitalWrite(TR, HIGH);
        delay(15);
        digitalWrite(TR, LOW);
        DURACION = pulseIn(ECHO, HIGH);
        DISTANCIA = DURACION / 58.2;
        Serial.println(DISTANCIA);
        delay(100);

        if (DISTANCIA >= 70) {
          if (DISTANCIA >= newDistancia) {
            newDistancia = DISTANCIA;
            grado = pos;
          }
        }
      }
      for (pos = 180; pos >= 90; pos -= 1) {
        //decirle al servo que vaya a la posición en la variable 'pos'
        servo_15.write(pos);
        if (req == '9') {
          break;
        }
        // wait 15 ms for servo to reach the position
        delay(15);  // Wait for 15 millisecond(s)
      }
      //retroceder(1000);
      delay(100);
      girar(grado);
    }
  } else if (result == 'disabled') {
    stop();
  }
}

bool rideCar() {
  //se lee la distancia a la que se encuentra los objetos
  bool run = false;
  digitalWrite(TR, HIGH);
  delay(100);
  digitalWrite(TR, LOW);
  DURACION = pulseIn(ECHO, HIGH);
  DISTANCIA = DURACION / 58.2;
  //Asignar rango de medicion;
  if (DISTANCIA <= 1260 && DISTANCIA >= 55) {
    Serial.println("Hacia delante el motor");
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, HIGH);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, HIGH);
    run = true;
  } else {
    digitalWrite(enableA, LOW);
    digitalWrite(enableB, LOW);
    digitalWrite(motorA1, LOW);
    digitalWrite(motorA2, LOW);
    digitalWrite(motorB1, LOW);
    digitalWrite(motorB2, LOW);
  }
  return run;
}

void girar(int grado) {
  int wait = 0;
  if (grado == 0) {
    retroceder(500);
    delay(1000);
    girarIzquierda(135);
    delay(1000);
  } else {
    if (grado <= 90) {
      wait = girarDerecha(grado);
    } else {
      wait = girarIzquierda(grado);
    }
  }
  delay(wait);
  stop();
}

int girarDerecha(int grado) {
  int wait = 0;
  Serial.println("Hacia DERECHA el motor");
  digitalWrite(motorA1, HIGH);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, HIGH);
  wait = ((90 - grado) * turnTime) / 90;
  return wait;
}

int girarIzquierda(int grado) {
  int wait = 0;
  Serial.println("Hacia IZQUIERDA el motor");
  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, HIGH);
  digitalWrite(motorB1, HIGH);
  digitalWrite(motorB2, LOW);
  wait = ((grado - 90) * turnTime) / 180;
  return wait;
}

void retroceder(int wait) {
  Serial.println("Hacia atrás el motor");
  digitalWrite(motorA1, HIGH);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, HIGH);
  digitalWrite(motorB2, LOW);
  delay(wait);
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
