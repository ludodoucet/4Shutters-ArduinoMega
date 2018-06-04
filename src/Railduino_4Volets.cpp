#include <Arduino.h>


#include <Shutters.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Ethernet.h>

boolean debug = false;

// Network settings
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 100, 11);
//////
IPAddress server(192, 168, 100, 10); // Pi ip adress with mosquitto
// MQTT settings
char message_buff[100];
String TheTopicPrefix = "volets/";
char TheTopic[] = "0";
char msg[80];
char TheTopicAssemble[80];
// Shutters settings
const unsigned long upCourseTime1 = 27 * 1000;
const unsigned long upCourseTime2 = 27 * 1000;
const unsigned long upCourseTime3 = 27 * 1000;
const unsigned long upCourseTime4 = 27 * 1000;
const unsigned long downCourseTime1 = 27 * 1000;
const unsigned long downCourseTime2 = 27 * 1000;
const unsigned long downCourseTime3 = 27 * 1000;
const unsigned long downCourseTime4 = 27 * 1000;

const float calibrationRatio = 0.1;
//const byte SHUTTERS1_EEPROM_POSITION = 1;
const byte eepromOffset1 = 1;
const byte eepromOffset2 = 2;
const byte eepromOffset3 = 3;
const byte eepromOffset4 = 4;

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
int initialize  = 0; // Initialyse the commutators state when reset
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
  if (debug == true) Serial.println(TheMessage);
  return TheMessage;
}
//////// MQTTout
void MQTTout(int ShuttersNumber, int ShuttersState) {sendMQTTPayload(CreeTheTopic(ShuttersNumber), CreateTheMessage(ShuttersState));}

//////// Shutters
// Come from Shutter.h' s example file
//
//1
void shuttersOperationHandler1(Shutters* s, ShuttersOperation operation) {
  switch (operation) {
    case ShuttersOperation::UP:
      
      if (debug == true) Serial.println("shutters1 going up.");
      up(1);
      break;
    case ShuttersOperation::DOWN:
      
      if (debug == true) Serial.println("shutters1 going down.");
      down(1);
      break;
    case ShuttersOperation::HALT:
      
      if (debug == true) { Serial.println("shutters1 halted.");}
      stp(1);
      break;
  }
}
void readInEeprom1(char* dest, byte length) {
  for (byte i = 0; i < length; i++) {
    dest[i] = EEPROM.read(eepromOffset1 + i);
  }
}
void shuttersWriteStateHandler1(Shutters* shutters, const char* state, byte length) {
  for (byte i = 0; i < length; i++) {
    EEPROM.write(eepromOffset1 + i, state[i]);
  }
  String states = String(state);
  if (debug == true) {Serial.print("shutters1 at ");
  Serial.print(states);
  Serial.println("% write");}
}
void onShuttersLevelReached1(Shutters* shutters, byte level) {
  if (debug == true) {Serial.print("shutters1 at ");
  Serial.print(level);
  Serial.println("%");}
  if (level != 255)  {  MQTTout(1, level);}
}

Shutters shutters1;
//2
void shuttersOperationHandler2(Shutters* s, ShuttersOperation operation) {
  switch (operation) {
    case ShuttersOperation::UP:
      
      if (debug == true) Serial.println("shutters2 going up.");
      up(2);
      break;
    case ShuttersOperation::DOWN:
      
      if (debug == true) Serial.println("shutters2 going down.");
      down(2);
      break;
    case ShuttersOperation::HALT:
      
      if (debug == true) { Serial.println("shutters2 halted.");}
      stp(2);
      break;
  }
}
void readInEeprom2(char* dest, byte length) {
  for (byte i = 0; i < length; i++) {
    dest[i] = EEPROM.read(eepromOffset2 + i);
  }
}
void shuttersWriteStateHandler2(Shutters* shutters, const char* state, byte length) {
  for (byte i = 0; i < length; i++) {
    EEPROM.write(eepromOffset2 + i, state[i]);
    #ifdef ESP8266
    EEPROM.commit();
    if (state != 255)  {  MQTTout(2, state);}
    #endif
  }
}
void onShuttersLevelReached2(Shutters* shutters, byte level) {
  if (debug == true) {Serial.print("shutters2 at ");
  Serial.print(level);
  Serial.println("%");}
}

Shutters shutters2;
//3
void shuttersOperationHandler3(Shutters* s, ShuttersOperation operation) {
  switch (operation) {
    case ShuttersOperation::UP:
      
      if (debug == true) Serial.println("shutters3 going up.");
      up(3);
      break;
    case ShuttersOperation::DOWN:
      
      if (debug == true) Serial.println("shutters3 going down.");
      down(3);
      break;
    case ShuttersOperation::HALT:
      
      if (debug == true) { Serial.println("shutters3 halted.");}
      stp(3);
      break;
  }
}
void readInEeprom3(char* dest, byte length) {
  for (byte i = 0; i < length; i++) {
    dest[i] = EEPROM.read(eepromOffset3 + i);
  }
}
void shuttersWriteStateHandler3(Shutters* shutters, const char* state, byte length) {
  for (byte i = 0; i < length; i++) {
    EEPROM.write(eepromOffset3 + i, state[i]);
    #ifdef ESP8366
    EEPROM.commit();
    if (state != 255)  {  MQTTout(3, state);}
    #endif
  }
}
void onShuttersLevelReached3(Shutters* shutters, byte level) {
  if (debug == true) {Serial.print("shutters3 at ");
  Serial.print(level);
  Serial.println("%");}
}

Shutters shutters3;
//4
void shuttersOperationHandler4(Shutters* s, ShuttersOperation operation) {
  switch (operation) {
    case ShuttersOperation::UP:
      
      if (debug == true) Serial.println("shutters4 going up.");
      up(4);
      break;
    case ShuttersOperation::DOWN:
      
      if (debug == true) Serial.println("shutters4 going down.");
      down(4);
      break;
    case ShuttersOperation::HALT:
      
      if (debug == true) { Serial.println("shutters4 halted.");}
      stp(4);
      break;
  }
}
void readInEeprom4(char* dest, byte length) {
  for (byte i = 0; i < length; i++) {
    dest[i] = EEPROM.read(eepromOffset4 + i);
  }
}
void shuttersWriteStateHandler4(Shutters* shutters, const char* state, byte length) {
  for (byte i = 0; i < length; i++) {
    EEPROM.write(eepromOffset4 + i, state[i]);
    #ifdef ESP8466
    EEPROM.commit();
    if (state != 255)  {  MQTTout(4, state);}
    #endif
  }
}
void onShuttersLevelReached4(Shutters* shutters, byte level) {
  if (debug == true) {Serial.print("shutters4 at ");
  Serial.print(level);
  Serial.println("%");}
}

Shutters shutters4;
/////// Receve a MQQT
// Fonction pour definir les actions suite � un message re�u
void MQTTcommande(int ShuttersNumber, String valeur) {
  if (debug == true) {Serial.print(ShuttersNumber);
  Serial.println(valeur);}
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
  if (debug == true) {Serial.print(ShuttersNumber);
  Serial.println(valeur);}
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

	  if (debug == true) Serial.println("assigne la bonne valeur � laststatus1, 2...");
	  status1 = InputState[1 - 1]*10 + InputState[2 - 1];
	  status2 = InputState[3 - 1]*10 + InputState[4 - 1];
	  status3 = InputState[5 - 1]*10 + InputState[6 - 1];
	  status4 = InputState[11 - 1]*10 + InputState[12 - 1];
	  laststatus1 = status1;
	  laststatus2 = status2;
	  laststatus3 = status3;
	  laststatus4 = status4;
	  initialize = 1;
	  break;
     }

}
// Fonction pour interpreter l'�tat des inters apr�s un changement d'�tat
//
void switching(int ShuttersNumber, int statusX) {
  if (debug == true) {Serial.print("ShuttersNumber = ");
  Serial.print(ShuttersNumber);
  Serial.print(", statusX = ");
  Serial.println(statusX);}
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
	if (initialize == 0 and debug == true) {Serial.println("inialized");}
	else {
		if ((unsigned long)(millis() - previousMillis) >= 50) {
	   // save the last time you blinked the LED
	   previousMillis = millis();
		for (int i = 0; i < numOfDigInputs; i++) {
		 InputState[i] = digitalRead(InputPins[i]);
		 if (InputState[i] != lastInputState[i]) {
		  if (InputState[i] == HIGH) {
		  if (debug == true) {Serial.print("InputState[i], i = ");
		  Serial.print(i + 1);
		  Serial.println(" LOW");}
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
		  if (debug == true) {Serial.print("InputState[i], i = ");
		  Serial.print(i + 1);
		  Serial.println(" HIGH");}
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
}
void setup() {
  Serial.begin(9600);
  /// stay at the start of the setup for initialize the commutators state
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


  //1
  char storedShuttersState1[shutters1.getStateLength()];
  readInEeprom1(storedShuttersState1, shutters1.getStateLength());
  shutters1
    .setOperationHandler(shuttersOperationHandler1)
    .setWriteStateHandler(shuttersWriteStateHandler1)
    .restoreState(storedShuttersState1)
    .setCourseTime(upCourseTime1, downCourseTime1)
    .onLevelReached(onShuttersLevelReached1)
    .begin();
  //2
  char storedShuttersState2[shutters2.getStateLength()];
  readInEeprom2(storedShuttersState2, shutters2.getStateLength());
  shutters2
    .setOperationHandler(shuttersOperationHandler2)
    .setWriteStateHandler(shuttersWriteStateHandler2)
    .restoreState(storedShuttersState2)
    .setCourseTime(upCourseTime2, downCourseTime2)
    .onLevelReached(onShuttersLevelReached2)
    .begin();    
  //3
  char storedShuttersState3[shutters3.getStateLength()];
  readInEeprom3(storedShuttersState3, shutters3.getStateLength());
  shutters3
    .setOperationHandler(shuttersOperationHandler3)
    .setWriteStateHandler(shuttersWriteStateHandler3)
    .restoreState(storedShuttersState3)
    .setCourseTime(upCourseTime3, downCourseTime3)
    .onLevelReached(onShuttersLevelReached3)
    .begin();
  //4
  char storedShuttersState4[shutters4.getStateLength()];
  readInEeprom4(storedShuttersState4, shutters4.getStateLength());
  shutters4
    .setOperationHandler(shuttersOperationHandler4)
    .setWriteStateHandler(shuttersWriteStateHandler4)
    .restoreState(storedShuttersState4)
    .setCourseTime(upCourseTime4, downCourseTime4)
    .onLevelReached(onShuttersLevelReached4)
    .begin();

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
