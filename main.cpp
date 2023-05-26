#include <Arduino.h>
#include <SPI.h>
#include <WiFi101.h>
#include <HTTPClient.h>
#include <MFRC522.h> //RFID
#include <Servo.h>

// Informations de connexion Wi-Fi
char ssid[] = "122dumaresq";
char password[] = "dumaresq";


// Adresse IP et port du serveur
char server[] = "10.0.0.5";
int port = 80;

//Lien du serveur distant pour la réception et traitement des données

const char* serverName = "10.0.0.5/parking/script.php";

#define SS_PIN 11
#define RST_PIN 4

#define TRIGGER_PIN 6
#define ECHO_PIN 7

#define PIR_PIN_1 8 // Broche du capteur PIR pour le premier emplacement
#define PIR_PIN_2 9 // Broche du capteur PIR pour le deuxième emplacement

MFRC522 mfrc522(SS_PIN, RST_PIN); // Création d'une instance de MFRC522
Servo servoEntree;                // Définir le nom du servo
Servo servoSortie;                // Définir le nom du servo

bool isVehicleDetectedSpot1 = false; // Indicateur de détection de véhicule pour le premier emplacement
bool isVehicleDetectedSpot2 = false; // Indicateur de détection de véhicule pour le deuxième emplacement
int availableParkingSpots = 2;       // Nombre d'emplacements de parking disponibles
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
String uid;
String tagClient = "5C 56 3C 5C";

int calculateDistance(int triggerPin, int echoPin);
bool detectVehicle(int pirPin);
void sendDataToServer(String uid, bool isVehicleDetectedSpot1, bool isVehicleDetectedSpot2);

void setup()
{
  Serial.begin(9600);

  // Connexion au réseau Wi-Fi
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Connexion au réseau Wi-Fi...");
    WiFi.begin(ssid, password);
    delay(1000);
  }

  Serial.println("Connexion au réseau Wi-Fi établie !");

  SPI.begin();       // Initialisation de la communication SPI
  mfrc522.PCD_Init(); // Initialisation du lecteur RFID
  Serial.println("Approchez une carte RFID...");
  servoEntree.attach(3); // Broche du servo
  servoSortie.attach(4); // Broche du servo
  servoEntree.write(0);  // Position de départ du servo
  servoSortie.write(0);  // Position de départ du servo

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN_1, INPUT);
  pinMode(PIR_PIN_2, INPUT);

  Serial.println("Put your card to the reader...");
  Serial.println();
}

void loop()
{
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  uid = content.substring(1);
  if (uid  == tagClient) // Changez ici l'UID de la carte/carte pour autoriser l'accès
  {
    Serial.println("Authorized access");
    Serial.println();
    delay(500);

    // Vérification de la distance du capteur ultrason
    int distanceEntree = calculateDistance(TRIGGER_PIN, ECHO_PIN);
    int distanceSortie = calculateDistance(TRIGGER_PIN, ECHO_PIN);
    
    // Vérification de la présence de véhicule sur les emplacements du parking
    isVehicleDetectedSpot1 = detectVehicle(PIR_PIN_1);
    isVehicleDetectedSpot2 = detectVehicle(PIR_PIN_2);

    delay(1000);

    if (distanceEntree < 5 && (!isVehicleDetectedSpot1 || !isVehicleDetectedSpot2)) // Si la distance est inférieure à 20 cm et l'emplacement est disponible, le véhicule est détecté
    {
      

      servoEntree.write(180);
      delay(5000);
      servoEntree.write(0);

      //Attendre 2 sécondes pour permettre au véhicule de stationner à un spot
      
    
     while ((isVehicleDetectedSpot1==false) || (isVehicleDetectedSpot2==false))
     {
      delay(20000);
      isVehicleDetectedSpot1 = detectVehicle(PIR_PIN_1);
      isVehicleDetectedSpot2 = detectVehicle(PIR_PIN_2);
      
     }
     sendDataToServer(uid, isVehicleDetectedSpot1, isVehicleDetectedSpot2);
      
   
    }
    else if (distanceSortie < 5 ) // Si la distance est inférieure à 5 cm 
    {
      Serial.println("Vehicle detected à la sortie du parking");
      
      servoSortie.write(180);
      delay(5000);
      servoSortie.write(0);
       isVehicleDetectedSpot1 = detectVehicle(PIR_PIN_1);
      isVehicleDetectedSpot2 = detectVehicle(PIR_PIN_2);

      sendDataToServer(uid, isVehicleDetectedSpot1, isVehicleDetectedSpot2);
    }
    else
    {
      Serial.println("No vehicle detected or parking spot is occupied!");
    }
  }
  else
  {
    Serial.println("Access denied");
  }
}

int calculateDistance(int triggerPin, int echoPin)
{
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  return distance;
}

bool detectVehicle(int pirPin)
{
  if (digitalRead(pirPin) == HIGH)
  {
    return true; // Véhicule détecté
  }
  else
  {
    return false; // Pas de véhicule détecté
  }
}

void sendDataToServer(String uid, bool isVehicleDetectedSpot1, bool isVehicleDetectedSpot2) {
  if (WiFi.status() == WL_CONNECTED) {
    HttpClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "uid=" + uid + "&spot1=" + String(isVehicleDetectedSpot1) +
                             "&spot2=" + String(isVehicleDetectedSpot2);

    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}





