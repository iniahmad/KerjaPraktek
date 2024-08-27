/// setup sensor
#include <PZEM004Tv30.h>

#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#define PZEM_SERIAL Serial2
#define delay_sampling 30000 // delay for sampling voltage and current

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);



/// setup wifi dan send
#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid     = "add ssid here";
const char* password = "add password here";

// REPLACE with your server address and path to receive_data.php
const char* serverName = "http://10.111.221.94/receive_data.php";


// Variables for storing sensor data
float voltage = 3.3;  // Example voltage value
float current = 1.2;  // Example current value

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi network");
}

void loop() {
  float voltage = pzem.voltage();
  float current = pzem.current();

  if (isnan(voltage)){
    Serial.println("Error reading voltage. Possibly caused by not connecting the power");
  } else if (isnan(current)) {
    Serial.println("Error reading current");
  } else {

      // Check WiFi connection status
      if(WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        // Your server IP address or domain name with the path to the PHP file
        http.begin(serverName);
        
        // Specify content-type header
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        
        // Prepare your HTTP POST request data
        String httpRequestData = "voltage=" + String(voltage) + "&current=" + String(current);
        Serial.print("httpRequestData: ");
        Serial.println(httpRequestData);
        
        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);
        
        // debug to check if its an unknown error
        // if (httpResponseCode == -2) {
        //   delay(500);
        //   Serial.println("Unkown error triggered! trying to send again");
        //   int httpResponseCode = http.POST(httpRequestData);

        //   if (httpResponseCode > 0) {
        //     Serial.print("HTTP Response code: ");
        //     Serial.println(httpResponseCode);
        //   } else {
        //     Serial.print("Error code: ");
        //     Serial.println(httpResponseCode);
        //   }
        // }

        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        Serial.println();
        
        // Free resources
        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }

  }

  // Send an HTTP POST request every 10 seconds
  delay(delay_sampling);  
}
