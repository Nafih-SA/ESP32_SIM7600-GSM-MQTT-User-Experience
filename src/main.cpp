#include <Arduino.h>
#include <string.h>
// #include <EEPROM.h>

/********** Macros & Varaibles ******************/
#define DEBUG 1
#define HOST "api.asksensors.com"
#define HOST2 "httpbin.org"
#define interval 30000
const char *apiKeyIn = "E3Rqsw9UksfyZfpcX4gbBJ7cTRurlweT";
bool stringComplete = false;
String msg = "";
unsigned long previousMillis = 0; 

struct coord
{
  float Latitude = 0;
  float Longitude = 0;
  float speed = 0;
  char *Hem;
  char E_W;
} data;

/********* Function Definitions ****************/
void AT_init()
{
#ifdef DEBUG
  Serial.print("\nInitializing SIM7600.");
#endif
  delay(500);
  Serial2.println("AT");
  delay(100);
  while (Serial2.available())
  {
    // get the new byte:
    Serial.print(".");
    delay(40);
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    msg += inChar;
    if (msg == "OK")
      break;
  }
  Serial.println("Successfully Init SIM7600");
  delay(500);
}

void getCoord()
{
  Serial2.println("AT+CGPS=1");
  delay(40);
  Serial2.println("AT+CGPSINFO");
  // delay(100);
  // Serial2.println("AT+CGPS=0");
  // delay(4000);
}

void serialEvent()
{
  while (Serial2.available())
  {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    msg += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
    {
      stringComplete = true;
    }
  }
}

double mm2dd(double x)
{
  int intpart = (int)x;
  double decpart = (x - intpart) * 5 / 3;
  return (intpart + decpart);
}

void setCoord(float lat, float lang)
{
#ifdef DEBUG
  Serial.printf("\nSending Lat:%f & Long:%f to Web server\n", lat, lang);
#endif
  delay(50);
  Serial2.print("AT+CHTTPACT=\"");
  Serial2.print(HOST);
  Serial2.print("\",80\r\n");
  delay(2000);

  String url = "GET http://api.asksensors.com/write/";
  url += apiKeyIn;
  url += "/?module1=";
  url += String(lat,6);
  url += ",+";
  url += String(lang,6);
  url += " HTTP/1.1\r\n";
  Serial2.print(url);

  delay(50);
  Serial2.print("Host: ");
  Serial2.print(HOST);
  Serial2.write("\r\n");
  
  // delay(50);
  // Serial2.print("User-Agent: MY WEB AGENT\r");
  
  // delay(50);
  // Serial2.print("Content-Length: 0\r");

  delay(50);
  Serial2.print("\r\n");
  delay(50);
  Serial2.print("\r\n");
  delay(50);
  Serial2.println("\r\n");
}

/************ Setup ****************/
void setup()
{
  Serial2.begin(115200);
  Serial.begin(115200);
  AT_init();
}

/************ Loop ****************/
void loop()
{

  serialEvent();
  unsigned long currentMillis = millis();
#ifdef DEBUG
  /*********To Send AT Commands through Serial monitor**********/
  if (Serial.available())
  {                               // If anything comes in Serial (USB),
    Serial2.write(Serial.read()); // read it and send it out Serial2
  }
#endif

  if (stringComplete)
  {
#ifdef DEBUG
    Serial.println(msg);
#endif

    if (msg.indexOf("CGPSINFO") > 0)
    {
      char buf[512];
      msg.toCharArray(buf, msg.length());
      char *token = strtok(buf, ":");
      while (msg.indexOf(",,,,,,,,") > 0)
      {
        Serial2.println("AT+CGPSINFO");
        delay(200);
        serialEvent();
      }
      token = strtok(NULL, ":");
      data.Latitude = atof(strtok(token, ",")) / 100;
      data.Latitude = mm2dd(data.Latitude);
      data.Hem = strtok(NULL, ",");
      data.Longitude = atof(strtok(NULL, ",")) / 100;
      data.Longitude = mm2dd(data.Longitude);
#ifdef DEBUG
      Serial.printf("\t\t%s \nLatitude: %10.6f \nLongitude: %11.6f", data.Hem, data.Latitude, data.Longitude);
#endif
      setCoord(data.Latitude, data.Longitude);
    }
    msg = "";
    stringComplete = false;
  }  

  else if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    getCoord();
  }
}