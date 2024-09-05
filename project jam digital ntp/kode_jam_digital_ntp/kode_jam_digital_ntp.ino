#include <SPI.h>
#include <UIPEthernet.h>
#include <NTPClient.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// Network settings
byte mac[] = { 0xDE, 0xAD, 0xB4, 0xEF, 0xFE, 0xAF }; // MAC address for Ethernet shield
EthernetUDP udp;

// pilih server
// IPAddress timeServer(10,111,221,1); // NTP server airnav hlp
IPAddress timeServer(182,16,248,57); // NTP server bmkg

NTPClient timeClient(udp, timeServer, 0, 60000); // NTP server and offset

#define clk_reg 8
#define data_reg 6
#define storage_reg 7 
#define IndicatorPin 4
#define reset_enc 10

// urutan digit
#define digitA 4 //
  #define digitB 3 //
  #define digitC 2 //
  #define digitD 1 //
  #define digitE 5 //
  #define digitF 6 //
  #define digitG 7 //
#define dp 8 //dp

#define FetchInterval 100 // Interval pengambilan data dari ntp (ms)

unsigned long jam = 88, menit = 88, detik = 88, lastdetik = 0;

int flag_ethernet = 0; // 1 berarti nyambung
int flag_last_ethernet = 0;
int tahun, bulan, tanggal;
DateTime now;

void setup() {
  // define output pin
  pinMode(clk_reg, OUTPUT);
  pinMode(data_reg, OUTPUT);
  pinMode(storage_reg, OUTPUT);
  pinMode(IndicatorPin, OUTPUT);
  pinMode(reset_enc, OUTPUT);

  // inisiasi serial dan 7 segmen
  Serial.begin(9600);
  print7seg();

  // reset ethernet
  digitalWrite(reset_enc, LOW);
  delay(100);
  digitalWrite(reset_enc, HIGH);
  delay(100);
  Ethernet.init(3);
  
  // Start Ethernet and DHCP
  while((Ethernet.hardwareStatus() != 10)){
    Serial.println((Ethernet.hardwareStatus() == 10) ? "hardware ok!" : "hardware not ok. try restarting");
     digitalWrite(IndicatorPin, HIGH);
      delay(500);

  }
  digitalWrite(IndicatorPin, LOW);

  Serial.println((Ethernet.hardwareStatus() == 10) ? "hardware ok!" : "hardware not ok. try restarting");
  // Serial.println(Ethernet.hardwareStatus());
  Serial.println("Starting Ethernet connection...");
  while (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP. Try again...");
    // No point in carrying on, so do nothing forevermore:
    for(int i = 0; i < 10; i++) {
      digitalWrite(IndicatorPin, HIGH);
      delay(500);
      digitalWrite(IndicatorPin, LOW);
      delay(500);
    }
  }

  // Give the Ethernet shield a second to initialize:
  // delay(1000);
  
  Serial.println("Ethernet connection successful");
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());

  // Initialize NTP client
  timeClient.begin();

  rtc.begin();
}

void loop() {
  // Update NTP client
  if (Ethernet.linkStatus() == LinkON) {
    flag_ethernet = 1;
    timeClient.update();
    // Get the current time
    unsigned long epochTime = timeClient.getEpochTime();

    // Convert epoch time to human-readable format
    jam = (epochTime % 86400L) / 3600;
    menit = (epochTime % 3600) / 60;
    detik = epochTime % 60;

    // Update RTC only at the start of each hour to reduce power consumption
    if (menit == 0 && detik == 0 && lastdetik != detik) {
    // if (lastdetik != detik) {
      rtc.adjust(DateTime(1, 1, 1, jam, menit, detik));
    // }
    }
  } else {
    flag_ethernet = 0;
    // Get time from RTC if failed to get from NTP
    updateTimeFromRTC();
  }

  if(flag_ethernet == 1 && flag_last_ethernet == 0) {
    digitalWrite(IndicatorPin, LOW); //nyala pake ethernet
    flag_last_ethernet = 1;
  }
  else if (flag_ethernet == 0 && flag_last_ethernet == 1) {
    digitalWrite(IndicatorPin, HIGH); //ngga pake ethernet
    Serial.println("Ethernet not connected, using RTC instead");
    flag_last_ethernet = 0;
  }

  // Update the 7-segment display when a change in seconds is detected
  if (detik != lastdetik) {
    print7seg();
    lastdetik = detik;
    // Print the time to the serial monitor
    Serial.print("Current time: ");
    Serial.print(jam);
    Serial.print(":");
    Serial.print(menit);
    Serial.print(":");
    Serial.println(detik);
  }
    

  // Wait for a while before querying again
  delay(FetchInterval);
}

void updateTimeFromRTC() {
  now = rtc.now();
  jam = now.hour();
  menit = now.minute();
  detik = now.second();
}

void print7seg() {
  // Print digits to 7-segment display
  // part detik
  printDigit(int(detik % 10));
  printDigit(int(detik / 10));

  // part menit
  printDigit(int(menit % 10));
  printDigit(int(menit / 10));

  // part jam
  printDigit(int(jam % 10));
  printDigit(int(jam / 10));

  digitalWrite(storage_reg, HIGH);
  digitalWrite(storage_reg, LOW);
}

void printDigit(int angka) {
  for (int i = 1; i <= 8; i++) {
    digitalWrite(data_reg, !isOn(angka, i));
    digitalWrite(clk_reg, HIGH);
    digitalWrite(clk_reg, LOW);
  }
}

int isOn(int angka, int digit) {
  // Handle the decimal point case
  if (digit == dp) {
    return 0;
  }
  
  // Determine which segments should be on based on the digit
  switch (angka) {
    case 0:
      return (digit == digitD) ? 0 : 1;
    case 1:
      return (digit == digitA || digit == digitE) ? 1 : 0;
    case 2:
      return (digit == digitC || digit == digitE) ? 0 : 1;
    case 3:
      return (digit == digitC || digit == digitG) ? 0 : 1;
    case 4:
      return (digit == digitB || digit == digitF || digit == digitG) ? 0 : 1;
    case 5:
      return (digit == digitA || digit == digitG) ? 0 : 1;
    case 6:
      return (digit == digitA) ? 0 : 1;
    case 7:
      return (digit == digitA || digit == digitB || digit == digitE) ? 1 : 0;
    case 8:
      return 1;
    case 9:
      return (digit == digitG) ? 0 : 1;
    default:
      return 0; // Default case to handle unexpected inputs
  }
}
