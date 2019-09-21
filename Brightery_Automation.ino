/*--------------------------------------------------------------
  Program:      Arduino Powered Home Automation

  Description:  Arduino web server that displays and controls relays,
                using buttons. The web page is stored on the micro SD card.

  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                micro SD card formatted FAT16 or FAT32.
                Pins 6 to 9 as outputs to 4 channel relay circuit.

  Software:     Developed using Arduino 1.8.10 software
                Should be compatible with Arduino 1.0 +
                SD card contains web page called index.htm

  References:   - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - SD Card library documentation:
                  http://arduino.cc/en/Reference/SD

  Date:         20 Sept 2019

  Author:       Brightery <info@brightery.com>
  --------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>


// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAD };
// IP address, may need to change depending on network
IPAddress ip(192, 168, 1, 177);
// create a server at port 80
EthernetServer server(80);
// the web page file on the SD card
File webFile;
// buffered HTTP request stored 
String HTTP_REQUEST;
boolean pins_state[10];
void setup() {
  // for debugging
  Serial.begin(9600);
  // Initialize ethernet
  Ethernet.init(10);

  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  if (!SD.exists("index.htm")) {
    Serial.println("ERROR - Can't find index.htm file!");
    return;  // can't find index file
  }
  
  Ethernet.begin(mac, ip);  // initialize Ethernet device

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  // Check network cable
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

// Switches

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  
  pins_state[6] = false;
  pins_state[7] = false;
  pins_state[8] = false;
  pins_state[9] = false;
  
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);
  
  // start to listen for clients
  server.begin();
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        HTTP_REQUEST += c;
        if (c == '\n') { // END OF FIRST LINE OF REQURST
          Serial.println(HTTP_REQUEST);
          if (HTTP_REQUEST.indexOf("GET / ") != -1) {
              client.println("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: keep-alive\n");
              webFile = SD.open("index.htm");  // open web page file
              if (webFile) {
                while (webFile.available()) {
                  client.write(webFile.read()); // send web page to client
                }
                webFile.close();
              }
              
          } else if (HTTP_REQUEST.indexOf("GET /update") != -1) {  // UPDATE
            int pin = HTTP_REQUEST.substring(14,15).toInt();
            if(HTTP_REQUEST.substring(18,19) == "f"){ // TURN THE REPLAY HIGH
                pins_state[pin] = false;
                digitalWrite(pin, HIGH);
            }
            else
            {
              pins_state[pin] = true;
              digitalWrite(pin, LOW);
            }
            RenderStatus(client);
          } else if (HTTP_REQUEST.indexOf("GET /get ")  != -1) {  // GET STATUS
            RenderStatus(client); 
          }
          HTTP_REQUEST = "";  // Reset result variable & get ready for a new request
          break;
        }
      }
    }
    
    delay(1); // give the web browser time to receive the data
    client.stop(); // close the connection:
  }
}


void RenderStatus(EthernetClient client){
    client.println("HTTP/1.1 200 OK\nContent-Type: application/json\nConnection: keep-alive\n"); // send rest of HTTP header
    client.print("[");
    for(int PIN_NUM = 0; PIN_NUM < 10; PIN_NUM++) {
        if (pins_state[PIN_NUM]) {
            client.print("1");
        }else {
            client.print("0");
        }
        if(PIN_NUM < 10-1)
        client.print(",");
    }
    client.print("]");
} 
