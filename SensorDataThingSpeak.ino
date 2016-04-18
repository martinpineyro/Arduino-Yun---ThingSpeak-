#include "DHT.h"

#ifdef SPARK
	#include "ThingSpeak/ThingSpeak.h"
#else
	#include "ThingSpeak.h"
#endif

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE); 

#ifdef ARDUINO_ARCH_AVR

  #ifdef ARDUINO_AVR_YUN
    #include "YunClient.h"
    YunClient client;
  #else

    #ifdef USE_WIFI_SHIELD
      #include <SPI.h>
      // ESP8266 USERS -- YOU MUST COMMENT OUT THE LINE BELOW.  There's a bug in the Arduino IDE that causes it to not respect #ifdef when it comes to #includes
      // If you get "multiple definition of `WiFi'" -- comment out the line below.
      #include <WiFi.h>
      char ssid[] = "<SSID>";          //  your network SSID (name) 
      char pass[] = "<WIFI PASWORD>";   // your network password
      int status = WL_IDLE_STATUS;
      WiFiClient  client;
    #else
      // Use wired ethernet shield
      #include <SPI.h>
      #include <Ethernet.h>
      byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
      EthernetClient client;
    #endif
  #endif
  // On Arduino:  0 - 1023 maps to 0 - 5 volts
  #define VOLTAGE_MAX 5.0
  #define VOLTAGE_MAXCOUNTS 1023.0
#endif

#ifdef ARDUINO_ARCH_ESP8266
  #include <ESP8266WiFi.h>
  char ssid[] = "<SSID>";          //  your network SSID (name) 
  char pass[] = "<WIFI PASSWORD>";   // your network password
  int status = WL_IDLE_STATUS;
  WiFiClient  client;
  // On ESP8266:  0 - 1023 maps to 0 - 1 volts
  #define VOLTAGE_MAX 1.0
  #define VOLTAGE_MAXCOUNTS 1023.0
#endif

#ifdef SPARK
    TCPClient client;
    #define VOLTAGE_MAX 3.3
    #define VOLTAGE_MAXCOUNTS 4095.0
#endif


unsigned long myChannelNumber = <CHANNEL NUMBER>;
const char * myWriteAPIKey = "<API KEY>";

void setup() {
  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_ESP8266)
    #ifdef ARDUINO_AVR_YUN
      Bridge.begin();
    #else
      #if defined(USE_WIFI_SHIELD) || defined(ARDUINO_ARCH_ESP8266)
        WiFi.begin(ssid, pass);
      #else
        Ethernet.begin(mac);
      #endif
    #endif
  #endif
  
  ThingSpeak.begin(client);
  
  dht.begin();
}

void loop() {
  // Read the input on each pin, convert the reading, and set each field to be sent to ThingSpeak.
  // On Arduino:  0 - 1023 maps to 0 - 5 volts
  // On ESP8266:  0 - 1023 maps to 0 - 1 volts
  // On Particle: 0 - 4095 maps to 0 - 3.3 volts
  float pinVoltage = analogRead(A0) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
  ThingSpeak.setField(1,pinVoltage);
  #ifndef ARDUINO_ARCH_ESP8266
    // The ESP8266 only has one analog input, so skip this
    pinVoltage = analogRead(A1) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
    ThingSpeak.setField(2,pinVoltage);
    
     float h = dht.readHumidity();
     // Read temperature as Celsius
     float t = dht.readTemperature();

       // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
    return;
  }
    pinVoltage = h;
    ThingSpeak.setField(3,pinVoltage);
    pinVoltage = t;
    ThingSpeak.setField(4,pinVoltage);

    /*
    pinVoltage = analogRead(A4) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
    ThingSpeak.setField(5,pinVoltage);
    pinVoltage = analogRead(A5) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
    ThingSpeak.setField(6,pinVoltage);
    pinVoltage = analogRead(A6) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
    ThingSpeak.setField(7,pinVoltage);
    pinVoltage = analogRead(A7) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
    ThingSpeak.setField(8,pinVoltage);
    */
  #endif

  // Write the fields that you've set all at once.
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  

  delay(20000); // ThingSpeak will only accept updates every 15 seconds. 
}
