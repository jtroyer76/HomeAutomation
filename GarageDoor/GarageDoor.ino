#include <Homie.h>
#include <NewPing.h>

#define DEBOUNCE_DELAY 200
#define MAX_DISTANCE 244
#define TRIGGER_DISTANCE 50
#define GARAGE_TRIGGER_DURATION 500

#define firmwareVersion "1.0.0"

//////////////////
// Door 1
/////////////////
#define DOOR1_BUTTON 5
#define TRIGGER1_PIN 0
#define ECHO1_PIN 4

unsigned long garage1SwitchRelease;
int switch1State = 0;
int door1State = 0;
int lastDoor1State = 0;
unsigned long lastDebounce1Time = 0;

HomieNode light1Node("lightone", "LightOne", "switch");
HomieNode door1Node("door_one", "Door_One", "door");
NewPing sonar1(TRIGGER1_PIN, ECHO1_PIN, MAX_DISTANCE); 

//////////////////
// Door 2
/////////////////
#define DOOR2_BUTTON 14
#define TRIGGER2_PIN 12
#define ECHO2_PIN 13

unsigned long garage2SwitchRelease;
int switch2State = 0;
int door2State = 0;
int lastDoor2State = 0;
unsigned long lastDebounce2Time = 0;

HomieNode light2Node("lighttwo", "LightTwo", "switch");
HomieNode door2Node("door_two", "Door_Two", "door");
NewPing sonar2(TRIGGER2_PIN, ECHO2_PIN, MAX_DISTANCE); 

bool light1OnHandler(const HomieRange& range, const String& value) {
  if (value != "true") return false;

  digitalWrite(DOOR1_BUTTON, HIGH);
  light1Node.setProperty("on").send("true");

  switch1State = true;
  garage1SwitchRelease = millis() + GARAGE_TRIGGER_DURATION;
  
  Homie.getLogger() << "Opener one pressed" << endl;

  return true;
}

bool light2OnHandler(const HomieRange& range, const String& value) {
  if (value != "true") return false;

  digitalWrite(DOOR2_BUTTON, HIGH);
  light2Node.setProperty("on").send("true");

  switch2State = true;
  garage2SwitchRelease = millis() + GARAGE_TRIGGER_DURATION;
  
  Homie.getLogger() << "Opener two pressed" << endl;

  return true;
}

void loopHandler(){
  //////////////////
  // Door 1
  /////////////////
  unsigned int distance1 = sonar1.ping_cm();
  bool door1Sensor = (distance1 != 0) && (distance1 < TRIGGER_DISTANCE);

  if (door1Sensor != lastDoor1State) {
    lastDebounce1Time = millis();
  }

  if((millis() - lastDebounce1Time) > DEBOUNCE_DELAY) {
    // We are now debounced
    if (door1State != lastDoor1State) {
      door1State = door1Sensor;

      Homie.getLogger() << "Door one is now " << (door1State ? "open" : "close") << endl;
      door1Node.setProperty("open").send(door1State ? "true" : "false");            
    }
  }

  lastDoor1State = door1Sensor;

  // Un toggle switch
  if((millis() > garage1SwitchRelease) && switch1State) {
    digitalWrite(DOOR1_BUTTON, LOW);
    switch1State = false;
    light1Node.setProperty("on").send("false");  
  }  

  //////////////////
  // Door 2
  /////////////////
  unsigned int distance2 = sonar2.ping_cm();
  int door2Sensor = (distance2 != 0) && (distance2 < TRIGGER_DISTANCE);

  if (door2Sensor != lastDoor2State) {
    lastDebounce2Time = millis();
  }

  if((millis() - lastDebounce2Time) > DEBOUNCE_DELAY) {
    // We are now debounced
    if (door2State != lastDoor2State) {
      door2State = door2Sensor;

      Homie.getLogger() << "Door two is now " << (door2State ? "open" : "close") << endl;
      door2Node.setProperty("open").send(door2State ? "true" : "false");            
    }
  }

  lastDoor2State = door2Sensor;

  // Un toggle switch
  if((millis() > garage2SwitchRelease) && switch2State) {
    digitalWrite(DOOR2_BUTTON, LOW);
    switch2State = false;
    light2Node.setProperty("on").send("false");  
  }  
}

void setup() {
  Serial.begin(115200);

  pinMode(DOOR1_BUTTON, OUTPUT);
  digitalWrite(DOOR1_BUTTON, LOW);

  pinMode(DOOR2_BUTTON, OUTPUT);
  digitalWrite(DOOR2_BUTTON, LOW);

  Homie_setFirmware("garage-system", firmwareVersion);
  Homie.setLoopFunction(loopHandler);

  light1Node.advertise("on").setName("On").setDatatype("boolean").settable(light1OnHandler);
  door1Node.advertise("open").setName("Open").setDatatype("boolean");
  
  light2Node.advertise("on").setName("On").setDatatype("boolean").settable(light2OnHandler);
  door2Node.advertise("open").setName("Open").setDatatype("boolean");

  Homie.setup();
}

void loop() {
  Homie.loop();
}
