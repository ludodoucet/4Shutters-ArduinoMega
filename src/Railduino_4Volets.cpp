#include <Arduino.h>
 //test

#include <Shutters.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>

boolean debug = true;
// Network settings
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 10, 11);
//////
IPAddress server(192, 168, 10, 10); // Pi ip adress with mosquitto
// MQTT settings
char message_buff[100];
String TheTopicPrefix = "volets/";
char TheTopic[] = "0";
char msg[80];
char TheTopicAssemble[80];
// Shutters settings
const unsigned long courseTime1 = 27 * 1000;
const unsigned long courseTime2 = 27 * 1000;
const unsigned long courseTime3 = 27 * 1000;
const unsigned long courseTime4 = 27 * 1000;
const float calibrationRatio = 0.1;
const byte SHUTTERS1_EEPROM_POSITION = 1;
const byte SHUTTERS2_EEPROM_POSITION = 2;
const byte SHUTTERS3_EEPROM_POSITION = 3;
const byte SHUTTERS4_EEPROM_POSITION = 4;
// ######### warning ######
// Input output settings, ex: RelayPinsUp[] = { <arduino output for the first shutter going up, <arduino output for the second...
// same for RelayPinsDown[]
const int numOfRelays = 12;
const int RelayPins[] = {39, 41, 43, 45, 47, 49, 23, 25, 27, 29, 31, 33};
const int RelayPinsUp[] = { 39, 43, 47, 31 };
const int RelayPinsDown[] = { 41, 45, 49, 33 };
const int InputPins[] = {36, 34, 48, 46, 69, 68, 67, 66, 44, 42, 40, 38, 6, 5, 3, 2, 14, 15, 16, 17, 24, 26, 28, 30};
const int numOfDigInputs = 24;
// Variable settings
int InputState[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int lastInputState[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long previousMillis = 0;
int laststatus1 = 0;
int laststatus2 = 0;
int laststatus3 = 0;
int laststatus4 = 0;
int status1 = 0;
int status2 = 0;
int status3 = 0;
int status4 = 0;
//
// auto reconnect to mosquitto server
//
EthernetClient ethClient;
PubSubClient client(ethClient);

long lastReconnectAttempt = 0;

boolean reconnect() {

  if (client.connect("arduinoClient")) {
    // Once connected, publish an announcement...
    if (debug == true) {client.publish("outTopic","hello world");}
    // ... and resubscribe
    client.subscribe("volets/in/+");
  }
  return client.connected();
}
//
// Relay command: up, down, stop ...
//
void up(int ShuttersNumber) {
  digitalWrite(RelayPinsUp[ShuttersNumber - 1], HIGH);
  digitalWrite(RelayPinsDown[ShuttersNumber - 1], LOW);
}
void down(int ShuttersNumber) {
  digitalWrite(RelayPinsUp[ShuttersNumber - 1], LOW);
  digitalWrite(RelayPinsDown[ShuttersNumber - 1], HIGH);
}
void stp(int ShuttersNumber) {
  digitalWrite(RelayPinsUp[ShuttersNumber - 1], LOW);
  digitalWrite(RelayPinsDown[ShuttersNumber - 1], LOW);
}
//////// sendMQTTPayload
// Create the topic ex: volets/out/1, volets/out/2...
//
String CreeTheTopic(int ShuttersNumber) {
  String TheTopic = TheTopicPrefix;
  TheTopic.concat("out/");
  TheTopic.concat(ShuttersNumber);
  if (debug == true) {Serial.print("TheTopic = ");
  Serial.println(TheTopic);}
  return TheTopic;
}
//
// Publish the payload
//
void sendMQTTPayload(String topic, String payload) {
  payload.toCharArray(msg, payload.length()+1);
  topic.toCharArray(TheTopic, topic.length()+1);
  client.publish(TheTopic , msg);
}
//
// Create the payload ex: UP, DOWN, 50...
//
String CreateTheMessage(int ShuttersState) {
  String TheMessage = String(ShuttersState);
  Serial.println(TheMessage);
  return TheMessage;
}
//////// MQTTout
void MQTTout(int ShuttersNumber, int ShuttersState) {sendMQTTPayload(CreeTheTopic(ShuttersNumber), CreateTheMessage(ShuttersState));}

//////// Shutters
// Come from Shutter.h' s example file
//
void shutters1Up() {
  Serial.println("shutters1 going up.");
  // TODO: Implement the code for the shutters1 to go up
  up(1);
}
void shutters1Down() {
  Serial.println("shutters1 going down.");
  // TODO: Implement the code for the shutters1 to go down
  down(1);
}
void shutters1Halt() {
  if (debug == true) { Serial.println("shutters1 halted.");}
  // TODO: Implement the code for the shutters1 to halt
  stp(1);
}
uint8_t shutters1GetState() {
  return EEPROM.read(SHUTTERS1_EEPROM_POSITION);
}
void shutters1SetState(uint8_t state) {
  if (debug == true) {  Serial.print("Saving state ");
    Serial.print(state);
    Serial.println(".");}

  EEPROM.write(SHUTTERS1_EEPROM_POSITION, state);
  if (state != 255)  {  MQTTout(1, state);}
}
void onshutters1LevelReached(uint8_t level) {
  if (debug == true) {Serial.print("shutters1 at ");}
  Serial.print(level);
  Serial.println("%");
}
Shutters shutters1(courseTime1, shutters1Up, shutters1Down, shutters1Halt, shutters1GetState, shutters1SetState, calibrationRatio, onshutters1LevelReached);
// Pour chaque volet...2
void shutters2Up() {
  Serial.println("shutters2 going up.");
  // TODO: Implement the code for the shutters2 to go up
  up(2);
}
void shutters2Down() {
  Serial.println("shutters2 going down.");
  // TODO: Implement the code for the shutters2 to go down
  down(2);
}
void shutters2Halt() {
  Serial.println("shutters2 halted.");
  // TODO: Implement the code for the shutters2 to halt
  stp(2);
}
uint8_t shutters2GetState() {
  return EEPROM.read(SHUTTERS2_EEPROM_POSITION);
}
void shutters2SetState(uint8_t state) {
  Serial.print("Saving state ");
  Serial.print(state);
  Serial.println(".");
  EEPROM.write(SHUTTERS2_EEPROM_POSITION, state);
  if (state != 255)  {  MQTTout(2, state);}
}
void onshutters2LevelReached(uint8_t level) {
  Serial.print("shutters2 at ");
  Serial.print(level);
  Serial.println("%");
}
Shutters shutters2(courseTime2, shutters2Up, shutters2Down, shutters2Halt, shutters2GetState, shutters2SetState, calibrationRatio, onshutters2LevelReached);
// ...3...
void shutters3Up() {
  Serial.println("shutters3 going up.");
  // TODO: Implement the code for the shutters3 to go up
  up(3);
}
void shutters3Down() {
  Serial.println("shutters3 going down.");
  // TODO: Implement the code for the shutters3 to go down
  down(3);
}
void shutters3Halt() {
  Serial.println("shutters3 halted.");
  // TODO: Implement the code for the shutters3 to halt
  stp(3);
}
uint8_t shutters3GetState() {
  return EEPROM.read(SHUTTERS3_EEPROM_POSITION);
}
void shutters3SetState(uint8_t state) {
  Serial.print("Saving state ");
  Serial.print(state);
  Serial.println(".");
  EEPROM.write(SHUTTERS3_EEPROM_POSITION, state);
  if (state != 255)  {  MQTTout(3, state);}
}
void onshutters3LevelReached(uint8_t level) {
  Serial.print("shutters3 at ");
  Serial.print(level);
  Serial.println("%");
}
Shutters shutters3(courseTime3, shutters3Up, shutters3Down, shutters3Halt, shutters3GetState, shutters3SetState, calibrationRatio, onshutters3LevelReached);
// ...4.
void shutters4Up() {
  Serial.println("shutters4 going up.");
  // TODO: Implement the code for the shutters4 to go up
  up(4);
}
void shutters4Down() {
  Serial.println("shutters4 going down.");
  // TODO: Implement the code for the shutters4 to go down
  down(4);
}
void shutters4Halt() {
  Serial.println("shutters4 halted.");
  // TODO: Implement the code for the shutters4 to halt
  stp(4);
}
uint8_t shutters4GetState() {
  return EEPROM.read(SHUTTERS4_EEPROM_POSITION);
}
void shutters4SetState(uint8_t state) {
  Serial.print("Saving state ");
  Serial.print(state);
  Serial.println(".");
  EEPROM.write(SHUTTERS4_EEPROM_POSITION, state);
  if (state != 255)  {  MQTTout(1, state);}
}
void onshutters4LevelReached(uint8_t level) {
  Serial.print("shutters4 at ");
  Serial.print(level);
  Serial.println("%");
}
Shutters shutters4(courseTime4, shutters4Up, shutters4Down, shutters4Halt, shutters4GetState, shutters4SetState, calibrationRatio, onshutters4LevelReached);


/////// Receve a MQQT
// Fonction pour definir les actions suite � un message re�u
void MQTTcommande(int ShuttersNumber, String valeur) {
  Serial.print(ShuttersNumber);
  Serial.println(valeur);
  if (valeur == "UP") {
    if (ShuttersNumber == 1) {shutters1.setLevel(0);}
    if (ShuttersNumber == 2) {shutters2.setLevel(0);}
    if (ShuttersNumber == 3) {shutters3.setLevel(0);}
    if (ShuttersNumber == 4) {shutters4.setLevel(0);}
  }
  else if (valeur == "DOWN") {
    if (ShuttersNumber == 1) {shutters1.setLevel(100);}
    if (ShuttersNumber == 2) {shutters2.setLevel(100);}
    if (ShuttersNumber == 3) {shutters3.setLevel(100);}
    if (ShuttersNumber == 4) {shutters4.setLevel(100);}
  }
  else if (valeur == "STOP") {
    if (ShuttersNumber == 1) {shutters1.stop();}
    if (ShuttersNumber == 2) {shutters2.stop();}
    if (ShuttersNumber == 3) {shutters3.stop();}
    if (ShuttersNumber == 4) {shutters4.stop();}
  }
  else {
    int level = valeur.toInt();
    if (ShuttersNumber == 1) {shutters1.setLevel(level);}
    if (ShuttersNumber == 2) {shutters2.setLevel(level);}
    if (ShuttersNumber == 3) {shutters3.setLevel(level);}
    if (ShuttersNumber == 4) {shutters4.setLevel(level);}
  }
}

//
//
//
// Fonction pour extraire les infos des messages
void TraductionTheMessage(char* topic, char* valeur) {
  char delimiter[] = "/";
  char *ptr;
  int ii=0;
  int ShuttersNumber = 0;
  // d�coupage du message
  ptr = strtok(topic, delimiter);
  while(ptr != NULL) {
    if (ii==2) {
      if (strcmp(ptr,"1") == 0) { ShuttersNumber = 1; }
    else if (strcmp(ptr,"2") == 0) { ShuttersNumber = 2; }
    else if (strcmp(ptr,"3") == 0) { ShuttersNumber = 3; }
      else if (strcmp(ptr,"4") == 0) { ShuttersNumber = 4; }
    }
    ptr = strtok(NULL, delimiter);
    ii++;
  }
  Serial.print(ShuttersNumber);
  Serial.println(valeur);
  MQTTcommande(ShuttersNumber, valeur);
}
//
// Fonction de r�cup�ration des messages MQTT
//
void callback(char* topic, byte* payload, unsigned int length) {
  // convert byte array into char array
  char paylC [length+1];
  for (unsigned int i = 0; i < length; i++) {
   paylC[i] = (char)payload[i];
  }
  paylC[length] = '\0';
  TraductionTheMessage(topic, paylC);
}

////////
//
void StateChangeDetectionInit() {

    for (int i = 0; i < numOfDigInputs; i++) {
     InputState[i] = digitalRead(InputPins[i]);

	  Serial.println("assigne la bonne valeur � laststatus1, 2...");
	  status1 = InputState[1 - 1]*10 + InputState[2 - 1];
	  status2 = InputState[3 - 1]*10 + InputState[4 - 1];
	  status3 = InputState[5 - 1]*10 + InputState[6 - 1];
	  status4 = InputState[11 - 1]*10 + InputState[12 - 1];
	  laststatus1 = status1;
	  laststatus2 = status2;
	  laststatus3 = status3;
	  laststatus4 = status4;
	  break;
     }

}
// Fonction pour interpreter l'�tat des inters apr�s un changement d'�tat
//
void switching(int ShuttersNumber, int statusX) {
  Serial.print("ShuttersNumber = ");
  Serial.print(ShuttersNumber);
  Serial.print(", statusX = ");
  Serial.println(statusX);
  // statusX, 10 descent, 1 monte, 11 stop, 0 bug
  switch (statusX) {
    case 10:
     if (ShuttersNumber == 1) {shutters1.setLevel(100);}
     if (ShuttersNumber == 2) {shutters2.setLevel(100);}
     if (ShuttersNumber == 3) {shutters3.setLevel(100);}
     if (ShuttersNumber == 4) {shutters4.setLevel(100);}
     break;
    case 1:
     if (ShuttersNumber == 1) {shutters1.setLevel(0);}
     if (ShuttersNumber == 2) {shutters2.setLevel(0);}
     if (ShuttersNumber == 3) {shutters3.setLevel(0);}
     if (ShuttersNumber == 4) {shutters4.setLevel(0);}
     break;
    case 11:
     if (ShuttersNumber == 1) {shutters1.stop();}
     if (ShuttersNumber == 2) {shutters2.stop();}
     if (ShuttersNumber == 3) {shutters3.stop();}
     if (ShuttersNumber == 4) {shutters4.stop();}
     break;
    case 0:
     Serial.println("ERREUR");
     break;
  }
}
//
// Fonction pour surveiller le changement d'�tat des inters (commutateur filaire pour volets) exemple Arduino
//
void StateChangeDetection() {
   if ((unsigned long)(millis() - previousMillis) >= 50) {
   // save the last time you blinked the LED
   previousMillis = millis();
    for (int i = 0; i < numOfDigInputs; i++) {
     InputState[i] = digitalRead(InputPins[i]);
     if (InputState[i] != lastInputState[i]) {
      if (InputState[i] == HIGH) {
	  Serial.print("InputState[i], i = ");
      Serial.print(i + 1);
      Serial.println(" LOW");
      int status1 = InputState[1 - 1]*10 + InputState[2 - 1];
      int status2 = InputState[3 - 1]*10 + InputState[4 - 1];
      int status3 = InputState[5 - 1]*10 + InputState[6 - 1];
      int status4 = InputState[11 - 1]*10 + InputState[12 - 1];
      if (status1 != laststatus1){ switching(1 , status1); }
      laststatus1 = status1;
      if (status2 != laststatus2){ switching(2 , status2); }
      laststatus2 = status2;
      if (status3 != laststatus3){ switching(3 , status3); }
      laststatus3 = status3;
      if (status4 != laststatus4){ switching(4 , status4); }
      laststatus4 = status4;
     }
      else {
	  Serial.print("InputState[i], i = ");
      Serial.print(i + 1);
      Serial.println(" HIGH");
      int status1 = InputState[1 - 1]*10 + InputState[2 - 1];
      int status2 = InputState[3 - 1]*10 + InputState[4 - 1];
      int status3 = InputState[5 - 1]*10 + InputState[6 - 1];
      int status4 = InputState[11 - 1]*10 + InputState[12 - 1];
      if (status1 != laststatus1){ switching(1 , status1); }
      laststatus1 = status1;
      if (status2 != laststatus2){ switching(2 , status2); }
      laststatus2 = status2;
      if (status3 != laststatus3){ switching(3 , status3); }
      laststatus3 = status3;
      if (status4 != laststatus4){ switching(4 , status4); }
      laststatus4 = status4;
    }
     }
     lastInputState[i] = InputState[i];
    }
   }
}
void setup() {
  Serial.begin(9600);
  /// stay at the start of the setup for initialise the commutators state
  for (int i = 1; i < numOfRelays; i = i + 2) {
        pinMode(RelayPins[i], OUTPUT);
  }
  for (int i = 0; i < numOfDigInputs; i++) {
        pinMode(InputPins[i], INPUT_PULLUP);
  }

  StateChangeDetectionInit();
  ///////////////////////
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  delay(1500);
  lastReconnectAttempt = 0;



  shutters1.begin();
  shutters2.begin();
  shutters3.begin();
  shutters4.begin();

}

void loop() {

  if (!client.connected()) {

    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
  client.loop();
  }


  StateChangeDetection();
  shutters1.loop();
  shutters2.loop();
  shutters3.loop();
  shutters4.loop();
}
