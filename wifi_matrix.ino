// Program to exercise the MD_MAX72XX library
//
// Test the library transformation functions

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>

const char* ssid     = "Tenda_EA0800";
const char* password = "";

// Use a button to transfer between transformations or just do it on a timer basis
// #define USE_SWITCH_INPUT  1

//#define SWITCH_PIN  8 // switch pin if enabled - active LOW

// We always wait a bit between updates of the display
#define  DELAYTIME  500  // in milliseconds

// Number of times to repeat the transformation animations
#define REPEATS_PRESET  16

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES   1
#define WRAPAROUND_MODE MD_MAX72XX::OFF

#define DATA_PIN  13  // or MOSI
#define CS_PIN    15  // or SS
#define CLK_PIN   14  // or SCK

#define DEBUG false

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

WiFiServer server(65432);

IPAddress ip(192, 168, 0, 110); 

char delimiters[] = " ";

String input;

void setup()
{
  Serial.begin(115200);
  mx.begin();
  // use wraparound mode
  mx.control(MD_MAX72XX::WRAPAROUND, WRAPAROUND_MODE);

#if USE_SWITCH_INPUT
  pinMode(SWITCH_PIN, INPUT_PULLUP);
#endif

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.print("WiFi connected. ");
  Serial.println(WiFi.macAddress());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  for(int i = 0; i < mx.getColumnCount(); i++){
    mx.setColumn(i, 0xFF);
    delay(100);
  }

  mx.clear();
}

void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    byte input_hash = 0;
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Receiving...");
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        if(DEBUG){ Serial.write(c); }
        input += c;
        input_hash += (byte)c;
      }
    }

    Serial.println();

    client.write(input_hash);
    
    Serial.println(input);
    client.println(input);
    
    if(input.startsWith("pset ")){
      int x, y, index = 0, nextIndex;
      bool state;

      index = input.indexOf(' ', 0);
      nextIndex = input.indexOf(' ', index + 1);
      if (nextIndex != -1){
        x = input.substring(index, nextIndex).toInt() - 1;
      }
      
      index = nextIndex;
      nextIndex = input.indexOf(' ', index + 1);
      if (nextIndex != -1){
        y = input.substring(index, nextIndex).toInt() - 1;

        index = nextIndex;

        state = (bool)(input.substring(index, input.length()).toInt());

      }else{
        y = input.substring(index, input.length()).toInt() - 1;
        state = !mx.getPoint(x, y);
      }

      Serial.println(x);
      Serial.println(y);
      Serial.println(state);
      
      mx.setPoint(x, y, state);

      Serial.print("mx.setPoint("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", "); Serial.print(state); Serial.println(")");
      client.print("mx.setPoint("); client.print(x); client.print(", "); client.print(y); client.print(", "); client.print(state); client.println(")");
    }else
    if (input.startsWith("invv")){
      mx.transform(MD_MAX72XX::TINV);
    }

    // Clear the header variable
    input = "";
    input_hash = 0;
    
    // Close the connection
    client.stop();
    Serial.println();Serial.println("Client disconnected.");Serial.println();
  }
}
