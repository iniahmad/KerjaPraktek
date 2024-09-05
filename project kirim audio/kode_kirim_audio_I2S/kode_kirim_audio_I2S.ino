/*
  ESP32 I2S Microphone Sample with WiFiUDP
  esp32-i2s-mic-wifiudp.ino
  Sample sound from I2S microphone, send over WiFi using UDP

  Requires INMP441 I2S microphone
*/

// Include necessary libraries
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "add ssid here";
const char* password = "add password here";

// UDP settings
const char* udpAddress = "192.168.174.30"; // add address of the server/receiver (printed to terminal from receiver code)
const int udpPort = 12345;
WiFiUDP udp;

// Connections to INMP441 I2S microphone
#define I2S_WS 25
#define I2S_SD 32
#define I2S_SCK 33

// Use I2S Processor 0
#define I2S_PORT I2S_NUM_0

// Define input buffer length
#define bufferLen 512
int16_t sBuffer[bufferLen];

void i2s_install() {
  // Set up I2S Processor configuration
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = i2s_bits_per_sample_t(16),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
}

void i2s_setpin() {
  // Set I2S pin configuration
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  Serial.println(" ");

  delay(1000);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up I2S
  i2s_install();
  i2s_setpin();
  i2s_start(I2S_PORT);

  // Start UDP
  udp.begin(udpPort);

  delay(500);
}

void loop() {
  // Get I2S data and place in data buffer
  size_t bytesIn = 0;
  esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen, &bytesIn, portMAX_DELAY);

  if (result == ESP_OK) {
    // Read I2S data buffer
    int16_t samples_read = bytesIn / sizeof(int16_t);

    if (samples_read > 0) {
      // Send data over UDP
      udp.beginPacket(udpAddress, udpPort);
      udp.write((uint8_t*)sBuffer, samples_read * sizeof(int16_t));
      udp.endPacket();
    }
  }
}
