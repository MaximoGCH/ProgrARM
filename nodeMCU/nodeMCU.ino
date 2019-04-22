#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

/* This are the WiFi access point settings. Update them to your likin */
char *ssid;
char *password;

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);
File fileHtml;
int htmlStart;
int isConected;

void handleRoot() {
  server.send(200, "text/html", "<!DOCTYPE html> <html> <head> <title> Control </title> <style> body{ background-color: #171814; color: #fff; } .ld{ border: 16px solid #fff; border-radius: 50%; border-top: 16px solid #171814; width: 120px; height: 120px; -webkit-animation: sp 2s linear infinite; /* Safari */ animation: sp 2s linear infinite; } @-webkit-keyframes sp { 0% {-webkit-transform: rotate(0deg);} 100% {-webkit-transform: rotate(360deg);} } @keyframes sp { 0% {transform: rotate(0deg);} 100% {transform: rotate(360deg);} } div, h1{ text-align: center; margin: auto; margin-top: 40px; } </style> </head> <body> <h1>Cargando, espere por favor</h2> <div class=\"ld\"></div> <script> let r = \"\"; function lp() { let xh = new XMLHttpRequest(); xh.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200){ if(this.responseText[this.responseText.length-1] == \"?\" && this.responseText[this.responseText.length-2] == \"?\"){ document.write(r); }else{ r += this.responseText; lp(); } } }; xh.open(\"GET\",\"/getPage\", true); xh.send(); } lp(); </script> </body> </html>\n\n");
  //Serial.println("Archivo root mandado");
  if(htmlStart == 1){
    htmlStart = 0;
    fileHtml.close();
  }
}

void handlePage() {

  int availableN;
  char html[1000];
  
  if(htmlStart == 0){
    htmlStart = 1;
    fileHtml = SPIFFS.open("/index.html", "r");
  }
  if(!fileHtml){
    server.send(200, "text/html", "Ha ocurrido un error :(???");
    return;
  }

  availableN = fileHtml.available();
  if(availableN >= 1000){
    fileHtml.readBytes(html, 1000);
    server.send(200, "text/html", html);
    //Serial.println("Archivo mandado");
    //Serial.println("Bytes");
    //Serial.println(availableN);
  }else
  if(availableN){
    server.send(200, "text/html", fileHtml.readString());
    //Serial.println("Archivo mandado");
    //Serial.println("Bytes");
    //Serial.println(availableN);
  }else{
    server.send(200, "text/html", "???");
    //Serial.println("Archivo final mandado");
  }
}

void handleload(){
  if(!server.hasArg("name")){
    server.send(200, "text/plain", "Error, name expected");
    return;
  }
  if(!server.hasArg("body")){
    server.send(200, "text/plain", "Error, body expected");
    return;
  }
  Serial.print("Sv");
  Serial.print(server.arg("name"));
  Serial.print('\n');
  Serial.print(server.arg("body"));
  server.send(200, "text/plain", "ok");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send(404, "text/plain", message);
}

void setup() {
  int nombre;
  int longSsid;
  int longPass;
  char readChar;
  delay(1000);
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
    ; // wait for serial port to connect.
  }

  ssid = (char *)malloc(sizeof(char));
  ssid[0] = '\0';
  password = (char *)malloc(sizeof(char));;
  password[0] = '\0';
  nombre = 1;
  longSsid = 1;
  longPass = 1;
  while (Serial.available() == 0) {
    //esperando a que le pasen el nombre y contrasenna de red
    delay(100);
  }
  while (Serial.available() > 0) {
    readChar = Serial.read();
    if(readChar == '\n'){
      nombre = 0;
    }else
    if(nombre == 1){
      longSsid ++;
      ssid = (char *)realloc(ssid, longSsid*sizeof(char));
      ssid[longSsid-2] = readChar;
      ssid[longSsid-1] = '\0';
    }else{
      longPass ++;
      password = (char *)realloc(password, longPass*sizeof(char));
      password[longPass-2] = readChar;
      password[longPass-1] = '\0';
    }
  }  

  SPIFFS.begin(); 

  isConected = false;
  //set-up the custom IP address
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print(" ");
  Serial.print(ssid);
  Serial.print(" ");
  Serial.print(password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print('\n');
  Serial.print(WiFi.localIP());
  Serial.print('\n');
  MDNS.begin("esp8266");

  server.on ("/", handleRoot);
  server.on ("/getPage", handlePage);
  server.on ("/load", HTTP_POST, handleload);
  server.onNotFound (handleNotFound);
  
  server.begin();
  htmlStart = 0;
}

void loop() {
  server.handleClient();
  MDNS.update();
}
