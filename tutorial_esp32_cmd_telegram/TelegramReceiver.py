import json
import requests
import time
import urllib
import sys
import math

bot_token = '486740449:AAETUubUiK0foIa0Sz4gPm62i8LGxyOjWTM'
chat_id = '-220394361'
BOT_URL = f"https://api.telegram.org/bot{bot_token}/"

def get_url(url):
    response = requests.get(url)
    content = response.content.decode("utf8")
    return content

def get_json_from_url(url):
    content = get_url(url)
    js = json.loads(content)
    return js

def get_updates(offset=None):
    url = BOT_URL + "getUpdates?timeout=100"
    if offset:
        url += "&offset={}".format(offset)
    print(f"url: {url}")
    js = get_json_from_url(url)
    return js

def get_last_update_id(updates):
    update_ids = []
    for update in updates["result"]:
        update_ids.append(int(update["update_id"]))
    return max(update_ids)

if __name__ == '__main__':
    update_id_old = -1
    #all'iinizio prendi update_id del ultmo msg inviato al robot
    update_id = -1 #questo incrementa per ogni messaggio arrivato al Bot da qualsiasi chat (sia privata che altri gruppi)
    #se si vuole filtrare soloi msg inviati al bot da una specifica chat bisogna filtrare per il campo:
    #'chat': {'id': 491091415, 'first_name': 'Giorgio', 'last_name': 'Toscana', 'username': 'GiorgioTi', 'type': 'private'}
    #e vedere il type: se 'private' allora avrà il dizionario avrà valori come 'first_name' 'last_name' 'username'
    #se 'supergroup' avrà "title": "Commands2ESP32"
    while True:
        updates = get_updates(-1)
        # print(f"updates: {updates}")
        if len(updates["result"]) > 0:
            update_id = updates["result"][-1]['update_id']
            update_id_old = update_id
            break

    while True:
        #-1 prende sempre l'ultimo messaggio arrivato al bot
        updates = get_updates(-1)
        #print(f"updates: {updates}")
        if len(updates["result"]) > 0:
            #print(f"updates: {updates}")
            #ottengo il nuovo update_id
            update_id = updates["result"][-1]['update_id']
            if update_id != update_id_old:
                #nuovo messaggio ricevuto
                text = updates["result"][-1]['message']['text']
                from_ = updates["result"][-1]['message']['from']['username']
                tipo_chat = updates["result"][-1]['message']['chat']['type']
                nome_chat = ''
                if tipo_chat == "supergroup":
                    nome_chat = updates["result"][-1]['message']['chat']['title']
                    print(f"Nuovo Msg Ricevuto\nDa: {from_}\nNella chat: {nome_chat}\nContenuto: {text}")
                elif tipo_chat == "private":
                    nome_chat = updates["result"][-1]['message']['chat']['username']
                    print(f"Nuovo Msg Ricevuto\nDa: {from_}\nIn Privato da: {nome_chat}\nContenuto: {text}")
                #print(f"Nuovo Msg Ricevuto\nDa: {from_}\nNella chat: {nome_chat}\nContenuto: {text}")
                #todo: qui si può filtrare per CHAT
            update_id_old = update_id
        time.sleep(1)

