/*
Six channel ethernet thermometer based on Anduino Nano v3 board and ENC28J60 Ethernet board
@author Vitaly Maslyaninov i@vitos.od.ua
You can find latest code and schematics on Github https://github.com/vitos73/webthermometer

REQUIRED Libraries: 
         https://github.com/milesburton/Arduino-Temperature-Control-Library
         https://github.com/PaulStoffregen/OneWire
         https://github.com/jcw/ethercard  

      
Device configured to use only DHCP address
It can be accesses as      
      index page - http://device_ip_from_dhcp/   e.g http://192.168.0.100
      single sensor channel  http://device_ip_from_dhcp/?channel=X   where X is {0..5} e.g http://192.168.0.100/?channel=4
      use serial monitor to read an ip or check your router/server's dhcp lease logs
*/


#include <EtherCard.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define DS_PIN 2  // pin for sensors data, needs to be pulled up to 5v via 4.7kOhm resistor
#define CS_PIN 8  // CS pin of ENC28J60

// tell onewire which pin is for sensors
OneWire oneWire_in(DS_PIN); 
DallasTemperature sensors(&oneWire_in);

// this sketch is only for DHCP configuration
// ENC28J60 doesn't have own MAC address, so MAC is required!
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x34 }; 
uint8_t sens_count;

// allocate 900 bytes of RAM to programm IO buffer for ethernet
byte Ethernet::buffer[900];
BufferFiller bfill;


// litle style for main page
// remember! - you have only 900 bytes for answer footprint 
const char css[] PROGMEM = "\r\n<style>* {font-size:18pt;font-family:Tahoma,Verdana,Arial;color:#777;}</style>\r\n";

// header for mainpage
const char http200b[] PROGMEM =
   "HTTP/1.0 200 OK\r\n"
   "Content-Type: text/html\r\n"
   "Pragma: no-cache\r\n"
   "\r\n"
   "<!DOCTYPE html>\r\n"
   "<html>\r\n<head>\r\n<title>WEB THERMOMETER</title>\r\n</head>\r\n"
   "<body>";
   
// footer for mainpage
const char http200e[] PROGMEM =  "\r\n</body>\r\n</html>";

// JS to reload main page every 60s
const char js_reload[] PROGMEM = "<script language=\"Javascript\">setTimeout(function(){window.location.reload(1);}, 60000);</script>";   
    
// text/plain header for answer - for zabbix you need only text without any html tags
const char http_OK[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/plain\r\n"
"Pragma: no-cache\r\n\r\n";

void mainPage()
{
  float temp = 0; 
  char temp_string[8];
  char channel_str[2];
  
  oneWire_in.reset();
  delay(5);
  sensors.begin();
  delay(5);
  uint8_t devs = sensors.getDeviceCount();
 
  if(devs>0) {
      temp=0;
      sensors.requestTemperatures();
      bfill.emit_p(PSTR("$F"), http200b);
      bfill.emit_p(PSTR("$F"), css);
      for (uint8_t i = 0; i < devs; i++)
      {
        temp = sensors.getTempCByIndex(i);  
        dtostrf(temp,3,2,temp_string);  
        dtostrf(i,1,0,channel_str);
        bfill.emit_p(PSTR("\r\n<br /><a href=\"?channel=$S\">Channel: $S</a>, temperature: $S &deg;C"),channel_str,channel_str,temp_string);    
        delay(5); 
      }
      bfill.emit_p(PSTR("$F"), js_reload);
      bfill.emit_p(PSTR("$F"), http200e);
  }
  else {
      bfill.emit_p(PSTR("$F"), http200b);
      bfill.emit_p(PSTR("$F"), css);
      bfill.emit_p(PSTR("NO SENSORS FOUND. CHECK WIRING & CONNECTION"));
      bfill.emit_p(PSTR("$F"), js_reload);
      bfill.emit_p(PSTR("$F"), http200e);
    }
  
}

void sensor_page(uint8_t sensor_id)
{
  
 oneWire_in.reset();
 delay(5);
 sensors.begin();
 delay(5);
 uint8_t devs = sensors.getDeviceCount();
 if (devs > 0 && devs > sensor_id ) {
 sensors.requestTemperatures();
 delay(5);
 float temp = 0;
 temp = sensors.getTempCByIndex(sensor_id);
 char outstr[8];
 dtostrf(temp,3,2,outstr); 
 bfill.emit_p(  
    PSTR("$F"
    "$S"
     ),
 http_OK,
 outstr
  );
 }
  else {
     bfill.emit_p(PSTR("$F"), http200b);
     bfill.emit_p(PSTR("$F"), css);
     bfill.emit_p(PSTR("NO SENSOR FOUND. CHECK WIRING & CONNECTION"));
     bfill.emit_p(PSTR("$F"), js_reload);
     bfill.emit_p(PSTR("$F"), http200e);
    } 
  
}

void setup () {
  delay(100);
  Serial.begin(57600);  
  delay(100);
  if (ether.begin(sizeof Ethernet::buffer, mymac,CS_PIN) == 0)     Serial.println( "Failed to access Ethernet controller");
  delay(300);
  if (!ether.dhcpSetup())   Serial.println("DHCP failed");
  ether.printIp("MyIP: ", ether.myip);
  ether.printIp("GW:   ", ether.gwip);   
  sensors.begin();
  delay(100); 
  Serial.println("Sensors count:");
  Serial.print(sensors.getDeviceCount(), DEC);
}

void loop () {
   
   delay(1);
   
   word len = ether.packetReceive();
   word pos = ether.packetLoop(len);

   if (pos) {
    bfill = ether.tcpOffset();
    char *data = (char *) Ethernet::buffer + pos;
    if (strncmp("GET /", data, 5) != 0) {

      mainPage();
    }
   
   
    else {

      data += 5;
      if (data[0] == ' ') {       
        mainPage();
      }
      else if (strncmp("?channel=0 ", data, 11) == 0) {
        sensor_page(0);
      }
      else if (strncmp("?channel=1 ", data, 11) == 0) {
        sensor_page(1);
      }
      else if (strncmp("?channel=2 ", data, 11) == 0) {
        sensor_page(2);
      }
      else if (strncmp("?channel=3 ", data, 11) == 0) {
        sensor_page(3);
      }
      else if (strncmp("?channel=4 ", data, 11) == 0) {
        sensor_page(4);
      }
      else if (strncmp("?channel=5 ", data, 11) == 0) {
        sensor_page(5);
      }
      else if (strncmp("?channel=6 ", data, 11) == 0) {
        sensor_page(6);
      }
      else if (strncmp("?channel=7 ", data, 11) == 0) {
        sensor_page(7);
      }

      else {
        // where is no 404 handlers, for every invalid page we show main page
        mainPage();
      }
    }
    ether.httpServerReply(bfill.position());    // send http response
  }

}
