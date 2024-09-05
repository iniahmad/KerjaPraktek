#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>

// WiFi credentials
const char* ssid = "add ssid here";
const char* password = "add password here";

// UDP settings
const int udpPort = 12345;
WiFiUDP udp;

// I2S speaker pins
#define I2S_WS 25
#define I2S_SD 22
#define I2S_SCK 26

// I2S configuration
#define I2S_PORT I2S_NUM_1
#define SAMPLE_RATE 44100
#define BUFFER_LEN 512

int16_t rBuffer[BUFFER_LEN];
IPAddress local_IP(192, 168, 174, 30);

// Timeout settings
unsigned long lastPacketTime = 0;
const unsigned long timeoutDuration = 100; // 0.1 seconds

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.config(local_IP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set up I2S
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_LEN,
    .use_apll = false
  };

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_SD,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

  udp.begin(udpPort);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read((char*)rBuffer, BUFFER_LEN * sizeof(int16_t));
    if (len > 0) {
      udp.flush();
      size_t bytes_written;
      i2s_write(I2S_PORT, rBuffer, len, &bytes_written, portMAX_DELAY);
      lastPacketTime = millis(); // Update the last packet time
    }
  }
  // else {

  //   // Clear the buffer or fill with silence
  //   memset(rBuffer, 0, sizeof(rBuffer));
  //   size_t bytes_written;
  //   i2s_write(I2S_PORT, rBuffer, sizeof(rBuffer), &bytes_written, portMAX_DELAY);
  // }

  // Check for timeout
  if (millis() - lastPacketTime > timeoutDuration) {
    // Clear the buffer or fill with silence
    memset(rBuffer, 0, sizeof(rBuffer));
    size_t bytes_written;
    i2s_write(I2S_PORT, rBuffer, sizeof(rBuffer), &bytes_written, portMAX_DELAY);
  }
}