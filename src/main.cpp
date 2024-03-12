/* ESP32 - DVM7135

   Heavily based on:

   ESP32 DEVKIT - ARDUINO IDE 1.8.5 - TLC7135 - PCF8574
   Gustavo Murta 29/03/2018
   Use I2C pullup Resistors 3K3 ohms
   https://www.elektormagazine.com/labs/esp32-digital-voltmeter

*/

#include "main.h"

#define SDApin 21    // PCF8574 SDA pin15       => GPIO_21
#define SCLpin 22    // PCF8574 SCL pin14       => GPIO_22
#define PCF8574 0x20 // PCF8574 Address (original)

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";
const char *PARAM_INPUT_3 = "ip";
const char *PARAM_INPUT_4 = "gateway";

bool ledState;

void i2cScan()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning..."); /*ESP32 starts scanning available I2C devices*/
  nDevices = 0;
  for (address = 1; address < 127; address++)
  { /*for loop to check number of devices on 127 address*/
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {                                                 /*if I2C device found*/
      Serial.print("I2C device found at address 0x"); /*print this line if I2C device found*/
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX); /*prints the HEX value of I2C address*/
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("No I2C devices found\n"); /*If no I2C device attached print this message*/
  }
  else
  {
    Serial.println("done\n");
  }
  delay(5000); /*Delay given for checking I2C bus every 5 sec*/
}

void notifyClients()
{
  ws.textAll(String(ledState));
}

void SendWsClients(String txstring)
{
  ws.textAll(txstring);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0)
    {
      ledState = !ledState;
      notifyClients();
      Serial.write("Toggle received !\n");
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void sendRoot(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, sizeof(index_html_gz));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void InitWebserver()
{
  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "---Test ok... ---"); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) // send gzipped index (gzip)
            {
              AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, sizeof(index_html_gz));
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });

   server.on("/home", HTTP_GET, sendRoot);

  server.onNotFound(notFound);

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.begin();
}

void setup()
{
  // serial
  Serial.begin(115200);

  // Wifi:

  WiFi.mode(WIFI_STA);
  WiFi.begin("_THOES_", "0598613193");
  Serial.println("Connecting to WiFi...");

  unsigned long previousMillis = 0;
  const long interval = 10000; // interval to wait for Wi-Fi connection (milliseconds)
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Failed to connect.");
      delay(10000);
      ESP.restart();
    }
  }

  Serial.println("Wifi connected...");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  InitWebserver();

  // we actually not use this...
  Wire.begin(SDApin, SCLpin); // sda= GPIO_21 /scl= GPIO_22
  i2cScan();

  Serial.println("Welcome to the DVM32....");

  initTCL7135(MeasurementReady);

  delay(1000);
}

void MeasurementReady(int16_t value)
{ // Ment to act as callback from measurements
  char buffer[100];

  sprintf(buffer, "{\"value\":%d}", value);

  SendWsClients(buffer);
}

unsigned long nextTime;

void loop()
{
  if (millis() > nextTime)
  {
    nextTime = millis() + 100; // beware of overflow
    ws.cleanupClients();

    // doTCL7135();
  }

  doTCL7135();

  delay(10);
}