#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

int counter = 0;
const int ledPin = 13;
const int maxCounter = 1000;
const int usLoopDelay = 50;
const int xBandPin = 8;
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  pinMode(xBandPin, INPUT);
  pinMode(ledPin, OUTPUT);  
  ss.begin(GPSBaud);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setTxPower(20);
  
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  Serial.println();
   for(int idx = 0; idx < maxCounter; ++idx)
  {
      int sensorValue = digitalRead(xBandPin);
      if(sensorValue)
      {
        delayMicroseconds(20000);
        LoRa.println("Se detecto persona en : ");
        printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
        printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
        LoRa.println("");
        LoRa.println("Enviar ayuda !!!");
        delayMicroseconds(50000);
      }
      digitalWrite(ledPin, sensorValue ? HIGH : LOW);
      delayMicroseconds(usLoopDelay);
  }
  
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    LoRa.println(F("No GPS data received: check wiring"));
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      LoRa.print('*');
      LoRa.print(' ');
  }
  else
  {
    LoRa.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      LoRa.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  LoRa.print(sz);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    LoRa.print(i<slen ? str[i] : ' ');
  smartDelay(0);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
