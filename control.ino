#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

#define LED_BOARD 2
#define ServoPin 14 

const char *ssid = "esp8266";
const char *password = "";

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

Servo myservo;

String getContentType(String filename);
bool handleFileRead(String path);

void handleServo(){
  String POS = server.arg("servoPOS");
  int pos = POS.toInt();
  myservo.write(pos);  
  
  Serial.print("Servo Angle:");
  Serial.println(pos);

  server.send(200, "text/plane","");
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void connectToWifi() {
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid,password) ? "Ready" : "Failed!");
  
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}


void setup() {
  Serial.begin(115200);
  SPIFFS.begin();

  pinMode(LED_BOARD, OUTPUT);
  myservo.write(90);  
  myservo.attach(ServoPin, 544, 2400);

  digitalWrite(LED_BOARD, ON);

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });

  server.on("/setPOS", handleServo);
  server.serveStatic("/", SPIFFS, "/", "max-age=86400");

  connectToWifi();
  server.begin();
}

void loop() {
  server.handleClient();
}
