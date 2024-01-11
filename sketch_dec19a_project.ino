#include <WiFi.h>
#include <ArduinoMqttClient.h>

//Constants won't change. They're used here to set pin numbers:
const int buttonPin = 35;   //The number of the pushbutton pin
const int ledPin = 33;    //The number of the LED pin
char mqttSsid[] = "IoTatelierF2122";    //Wifi SSID (22 to 44)
char mqttPass[] = "IoTatelier";   //Wifi password
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.122.1";    //MQTT broker IP address
const int port = 1883;      //MQTT broker port
const char publishTopic[] = "mariekristkoiz/light";   //Topic to publish light data
const char subscribeTopic[] = "Toufik/plantvochtigheid";    //Topic to subscribe for messages
long count = 0;
const long interval = 1000;   //Analog read interval
unsigned long previousMillis = 0;
const int lightPin = ledPin;    //Pin for the light
const int analogPin = 34;   //Pin for analog sensor input
byte lastButtonState =  LOW;    //Variable storing the last button state 
byte ledState = LOW;    //Variable storing the LED state
bool dark = false;

// MQTT code:
void setupForMQTT() {
  Serial.println("Wifi connecting");

  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(mqttSsid, mqttPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("MQTT connecting");
  bool MQTTconnected = false;

  while (!MQTTconnected) {
    if (!mqttClient.connect(broker, port))    //Connecting to MQTT broker
      delay(1000);
    else
      MQTTconnected = true;
  }

  mqttClient.onMessage(onMqttMessage);    //Set up of the function handeling incoming MQTT messages
  mqttClient.subscribe(subscribeTopic);   //Subscribes to specific MQTT topic
  Serial.println("Set up complete");
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message with topic '");
  Serial.println(mqttClient.messageTopic());

  String message = ".";

  while (mqttClient.available()) {
    message.concat((char)mqttClient.read());
  }

  Serial.println(message);
  
  if (message == "on")
    digitalWrite(lightPin, HIGH);
  else
    digitalWrite(lightPin, LOW);
}

void loopForMQTT() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    int value = analogRead(analogPin);
    
    Serial.print("Sending message to topic: ");
    Serial.println(publishTopic);
    Serial.println(value);
    
    mqttClient.beginMessage(publishTopic,true,0);   //Starts publishing a message to an MQTT topic
    mqttClient.print(value);    //Prints the sensor value to the MQTT message
    mqttClient.endMessage();    //Finishes the MQTT message 
  }

  delay(1);
}

void setup() {
  pinMode(ledPin, OUTPUT);      //Initialises the LED pin as an output
  pinMode(buttonPin, INPUT);    //Initialises the pushbutton pin as an input

  //MQTT setup:
  Serial.begin (115200);
  pinMode(lightPin, OUTPUT);
  
  setupForMQTT();   //Set up for MQTT connection making

}

void loop() {
//Button loop:
  byte buttonState = digitalRead(buttonPin);    //Reads the state of the pushbutton value

  if (buttonState != lastButtonState) {   //Reads button state and stores it
    lastButtonState = buttonState;    //If correct we store current state in last state variable 

    if (buttonState == LOW) {   //Check if current button's state is LOW
      ledState = (ledState == HIGH) ? LOW: HIGH;    //Toggles LED
      digitalWrite(ledPin, ledState);
    }
  }

//MQTT loop: 
  mqttClient.poll();    //Continuously polling for incoming MQTT messages

  loopForMQTT();    //Performs actions related to MQTT
}
