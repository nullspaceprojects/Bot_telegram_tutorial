#include <stdio.h>
#include <ArduinoJson.h> //configuratore: https://arduinojson.org/v6/assistant/#/step1
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "telegram_api_cert.h"
#include <WiFi.h>
#include "bot_telegram.h" //https://telegram-bot-sdk.readme.io/reference/getupdates
#include "frame.h"
#include <U8g2lib.h> //dal library manager:U8g2
#include <Adafruit_NeoPixel.h>

/**** CREAZIONE CERTIFICATO*****/
/*
C:\Users\GT STUPIRE\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2\tools
python.exe .\cert.py -s api.telegram.org -n telegram > telegram_api_cert.h
creare file telegram_api_cert.h in nuova Tab di arduino e copiare il contenuto del del file appena creato alla riga sopra
*/

/********* ISTANZA WIFI e certificato per POST HTTPS ************/
WiFiClientSecure client;
const char* ssid = "dubmocio";                          
const char* password = "Cubasesx3";

const String TELEGRAM_BOT_TOKEN = "486740449:AAETUubUiK0foIa0Sz4gPm62i8LGxyOjWTM";

/********* ISTANZA CLASSE DISPLAY I2C SH1106 128X64 ************/
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/********* LEDS NEO-PIXELS ************/
// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, 15, NEO_GRB + NEO_KHZ800); //24 LEDS, GPIO15
void rainbow(uint16_t first_hue=0, int8_t reps=1,uint8_t saturation=255, uint8_t brightness=255, bool gammify=false)
{
  uint16_t numLEDs  = strip.numPixels();
  for (uint16_t i=0; i<numLEDs; i++) 
  {
    uint16_t hue = first_hue + (i * reps * 65536) / numLEDs;
    uint32_t color = strip.ColorHSV(hue, saturation, brightness);
    if (gammify) color = strip.gamma32(color);
    strip.setPixelColor(i, color);
  }
  strip.show();
}

BotTelegram* bot;
TimerC looptimer;

void ConnectWiFi()
{
  //CONNESSIONE AL WIFI
  WiFi.begin(ssid, password);
  delay(2000);
  //TENTATIVI DI CONNESSIONE
  int counter = 0;
  while (!WiFi.isConnected()) 
  {
    delay(500);    
    if (++counter > 100)
    {
      //SE NON SI CONNETTE DOPO 100 TENTATIVI, RESETTA ESP 
      Serial.println("\nESP.restart()");
      ESP.restart();
    }
     
  }
  Serial.println("\nWiFi Connesso");
}

void setup() {
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setBitmapMode(1); //set the background to transparant

  //visualize splash screen
  u8g2.clearBuffer();
  u8g2.drawXBMP(32,
                0, 
                logo_ninf_width, 
                logo_ninf_height, 
                logo_ninf_bits);
  u8g2.sendBuffer();
  
  //SETUP LEDS NEOPIXELS
  //strip.begin();
  //strip.setBrightness(75);
  //strip.show(); // Initialize all pixels to 'off'

  WiFi.setAutoReconnect(true);
  ConnectWiFi();
  
  client.setCACert(cert_Go_Daddy_Secure_Certificate_Authority___G2);

  bot = new BotTelegram(&client, TELEGRAM_BOT_TOKEN);
  looptimer.start();
}

void loop() {

  if(looptimer.getET()>1000)
  {
    looptimer.reset();
    String received_cmd="";
    bot->ReceivedCommand(received_cmd);
    LastMsgReceived last_msg_rcved = bot->GetLastMsgReceived();
    bool isPrivateChat = last_msg_rcved.chat_type == "private";

    if(received_cmd=="/ledon")
    {
      //rainbow();
      Serial.println(received_cmd);
    }
    else if(received_cmd=="/ledoff")
    {
      //strip.clear();
      //strip.show();
      Serial.println(received_cmd);
    }
    else if( 
          (received_cmd.indexOf("/")==0 && received_cmd.indexOf("#")==1 && !isPrivateChat)
      ||  (received_cmd.indexOf("#")==0 && isPrivateChat) )
    {
      String oled_text;
      if(isPrivateChat)
      {
        oled_text = received_cmd.substring(1); //start from char id=1 (remove "#")
      }
      else
      {
        oled_text = received_cmd.substring(2); //start from char id=2 (remove "/#")
      }
      
      oled_text.trim();
      Serial.println(oled_text);
      
      //write on the screen the last msg received
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      uint8_t ypos=10;
      uint8_t xpos=0;
      u8g2.drawStr(xpos+20,ypos," == NEW MSG ==");
      ypos += 8+4;
      u8g2.drawStr(xpos,ypos,String("From: " + last_msg_rcved.from).c_str());
      ypos += 8+4;
      u8g2.drawStr(xpos,ypos,String("Chat: " + last_msg_rcved.chat_name).c_str());
      ypos += 8+4;
      u8g2.drawStr(xpos,ypos,String("ChatType: " + last_msg_rcved.chat_type).c_str());
      ypos += 8+4;
      u8g2.drawStr(xpos,ypos,String("Txt: " + oled_text).c_str());
      u8g2.sendBuffer();

    }
  }
  
}
