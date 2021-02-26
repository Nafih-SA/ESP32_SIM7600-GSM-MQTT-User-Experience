#include <Arduino.h>
#include <string.h>
// #include <EEPROM.h>

/********** Macros & Varaibles ******************/
#define DEBUG 1
#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60        //Time ESP32 will go to sleep (in seconds)
#define BUTTON_PIN_BITMASK 0x8004
#define usrname "nafih_sa"
const char *apiKeyIn = "aio_bWlP66Yg8CgVrlmzy3gy6ZQil0Fl";
#define interval 30000
RTC_DATA_ATTR int fbCount = 0;
RTC_DATA_ATTR int feedback[30];
bool stringComplete = false;
String msg = "";
unsigned long previousMillis = 0;
uint64_t pin;
bool button_falg = 0;


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

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  pin = esp_sleep_get_ext1_wakeup_status();
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Thankyou for the feedback :)");button_falg = 1; break;
    case 4  : 
    {
      if(fbCount)
      {
        AT_init();
        Serial.println("Uploading data to cloud");
        Serial2.println("AT+CMQTTSTART");
        delay(2000);
        Serial2.println("AT+CMQTTACCQ=0,\"nafih_sa\"");
        delay(2000);
        Serial2.println("AT+CMQTTCONNECT=0,\"tcp://io.adafruit.com:1883\",90,1,\"nafih_sa\",\"aio_bWlP66Yg8CgVrlmzy3gy6ZQil0Fl\"");
        delay(2000);
      }
      break;
    }
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
  switch(pin)
  {
    case 0 : 
    {
      if(fbCount)
      {   
        // while(fbCount>0)
        // {
          Serial2.println("AT+CMQTTTOPIC=0,24");
          delay(1000);
          Serial2.print("nafih_sa/feeds/feedback\r\n");
          delay(1000);
          String payload = "AT+CMQTTPAYLOAD=0,";
          payload += String(2*fbCount-1); // to account fo the commas separating individual feedback
          Serial2.println(payload);
          delay(1000);
          int i = 1;
          while(i <= fbCount)
          {
            Serial2.print(feedback[i]);
            if (i < fbCount)
              Serial2.print(",");
            i++;
          }
          Serial2.print("\r\n");
          // Serial.print("\r\n");
          // char str[32];
          // int i=0;
          // int index = 0;
          // for (i=1; i<5; i++)
          //   index += sprintf(&str[index], "%d ,", feedback[i]);
          // Serial.println(str);
          // Serial2.println(str);
          delay(1000);
        // }
             
        Serial2.print("AT+CMQTTPUB=0,1,60\r\n");
        delay(1000);
        Serial2.print("AT+CMQTTDISC=0,60\r\n");
        delay(1000);
        Serial2.print("AT+CMQTTREL=0\r\n");
        delay(1000);
        Serial2.print("AT+CMQTTSTOP\r\n");
        delay(1000);
        fbCount = 0;
      }
      else
        Serial.println("No feedback received :(");
      break;
    }
    case 4 : Serial.println("rating: 1.0");feedback[++fbCount] = 1; break; //GPIO_2 pressed (2^2 = 4)
    case 32768 : Serial.println("rating: 5.0"); feedback[++fbCount] = 5; break; //GPIO_15 pressed (2^15 = 32768)
    default : Serial.println("Hmm... that's unusual"); break;
  }
}


/************ Setup ****************/
void setup()
{
  Serial2.begin(115200);
  Serial.begin(115200);
  delay(500);
  print_wakeup_reason(); 
  if(button_falg)
    Serial.printf("Feedback no.%d\n",fbCount); button_falg = 0; 
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH );
	Serial.println("Good night");

	//Go to sleep now
	esp_deep_sleep_start();
}

/************ Loop that will never run ****************/
void loop()
{

  serialEvent();
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

//     if (msg.indexOf("CGPSINFO") > 0)
//     {
//       char buf[512];
//       msg.toCharArray(buf, msg.length());
//       char *token = strtok(buf, ":");
//       token = strtok(NULL, ":");
      
// #ifdef DEBUG
//       Serial.printf("Inside string complete");
// #endif
//     }
    msg = "";
    stringComplete = false;
  }  

}