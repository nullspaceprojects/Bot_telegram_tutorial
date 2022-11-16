#pragma once
#include <ArduinoJson.h> //configuratore: https://arduinojson.org/v6/assistant/#/step1
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

/********* DEFINIZIONE CLASSE HELPER TIMER ************/
class TimerC {

  private:
        unsigned long _start_time;
        unsigned long _elapsed_time;
  
  public:
      TimerC()
      {
        _start_time = 0;
        _elapsed_time = 0;
      }

      void start()
      {
        if(_start_time != 0)
        {
          return;
        }
        _start_time = millis();
      }
      
      unsigned long getET()
      {
        //in millisecondi
        _elapsed_time = millis() - _start_time;
        return _elapsed_time;
      }
      double getETSec()
      {
        unsigned long et_milli =  getET();
        return et_milli/1000.0;
      }
      void reset()
      {
        _start_time = millis();
        _elapsed_time = 0;
      }
      void stop_()
      {
        if(_start_time == 0)
        {
          return;
        }
        _start_time = 0;
        _elapsed_time = 0;
        
      }
};

class LastMsgReceived
{
  public:
    String text;
    String from;
    String chat_name;
    String chat_type;
    long datetime;
};

class BotTelegram
{
  public:
    //WiFiClientSecure client;
    WiFiClientSecure* client;
    String bot_token;
    LastMsgReceived last_msg;
  private:
    String get_telegram_msgs;
    const int maxMessageLength=1500;
    long int update_id;
    long int update_id_old;
    bool first_scan;

  public:
    BotTelegram(WiFiClientSecure* client_, String bot_token_)
    {
      client = client_;
      bot_token = bot_token_;
      get_telegram_msgs = "https://api.telegram.org/bot"+bot_token+"/getUpdates?timeout=100&offset=-1";
      Serial.print(get_telegram_msgs);
      //maxMessageLength = 1500;
      update_id=-1;
      update_id_old=-1;
      first_scan=true;
    }
  private:
    bool GetNewMessage(String& payload)
    {
      HTTPClient http;
      http.begin(*client, get_telegram_msgs );
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      if (httpResponseCode>0) {
        payload = http.getString();
        //Serial.println(payload);
        http.end();
        return true;
      }
      else {
        Serial.print("GET Error code: ");
        Serial.print(httpResponseCode);
        Serial.print(" ");
        Serial.println(HTTPClient::errorToString(httpResponseCode));
        // Free resources
        http.end();
        return false;
      }
    }
  public:
    bool ReceivedCommand(String& cmd)
    {
      String payload = "";
      bool ok= GetNewMessage(payload);
      if (!ok)
        return false;
      // Parse response into Json object
      DynamicJsonDocument doc(maxMessageLength);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) 
      {
          if (doc.containsKey("result")) 
          {
            ok = doc["ok"];
            if (!ok)
            {
              Serial.println(F("ok=False"));
              return false;
            }
              
            int resultArrayLength = doc["result"].size();
            if (resultArrayLength > 0) 
            {
                update_id = doc["result"][0]["update_id"];
                JsonObject result_0_message = doc["result"][0]["message"];
                if(first_scan)
                {
                  first_scan=false;
                  update_id_old = update_id;
                  return false;
                }
                if(update_id!=update_id_old)
                {
                  //datetime                 
                  long datetime = result_0_message["date"];
                  last_msg.datetime=datetime;
                  //nuovo messaggio ricevuto
                  cmd = String((const char*)result_0_message["text"]);
                  last_msg.text=cmd;
                  //da chi?
                  JsonObject result_0_message_from = result_0_message["from"];
                  String username = String((const char*)result_0_message_from["username"]);
                  last_msg.from=username;
                  //che tipo di chat?
                  JsonObject result_0_message_chat = result_0_message["chat"];
                  String result_0_message_chat_type = String((const char*)result_0_message_chat["type"]); // "private"
                  last_msg.chat_type=result_0_message_chat_type;
                  //da quale chat?
                  String chat_name="";                 
                  if(result_0_message_chat_type=="supergroup")
                  {
                    chat_name = String((const char*)result_0_message_chat["title"]);
                    
                    Serial.println("Nuovo Msg Ricevuto\ndt: "+String(datetime)+"\nDa:"+username+"\nDalla chat: "+chat_name+"\nContenuto: "+cmd);                   
                  }
                  else if(result_0_message_chat_type=="private")
                  {
                    chat_name = String((const char*)result_0_message_chat["username"]);
                    Serial.println("Nuovo Msg Ricevuto\ndt: "+String(datetime)+"\nDa:"+username+"\nIn Privato da: "+chat_name+"\nContenuto: "+cmd);
                    
                  }
                  last_msg.chat_name = chat_name;
                  Serial.println();
                  
                }
                update_id_old = update_id;
            }
            else
            {
              //Serial.println(F("no new messages"));
            }
          }
          else
          {
            Serial.println(F("Response contained no 'result'"));
          }
      }
      else
      {
        Serial.print(F("ReceivedCommand deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
      }
      return true;
      
    }
    LastMsgReceived GetLastMsgReceived()
    {
      return last_msg;
    }


};