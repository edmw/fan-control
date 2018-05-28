#include <Arduino.h>

#define REALM F("S2S")

#include "OS/OS.h"

#include "save2server.h"
#include "global.h"
#include "private.h"

#include "SoftwareSerial.h"
SoftwareSerial ESPSerial(8, 7);

#include "WiFiEsp.h"
WiFiEspClient client;

const char S2S_SSID[] = WIFI_SSID;
const char S2S_PASSWORD[] = WiFI_PASSWORD;

IPAddress S2S_SERVER(SERVER_IP);

#if PRODUCTION
const uint32_t S2S_INTERVAL = 300000; // 5 minutes
#else
const uint32_t S2S_INTERVAL = 60000; // 1 minute
#endif

elapsed_millis s2s_interval = S2S_INTERVAL;

String get_id() {
    byte mac[6];
    WiFi.macAddress(mac);
    char id[12];
    snprintf(id, 12, "APM-%02X%02X%02X", mac[2], mac[1], mac[0]);
    return String(id);
}

void wifi_info() {
    byte mac[6];
    WiFi.macAddress(mac);
    String mac_str = String(mac[5], HEX) + ":" // base mac
                   + String(mac[4], HEX) + ":" // base mac
                   + String(mac[3], HEX) + ":" // base mac
                   + String(mac[2], HEX) + ":" // chip id
                   + String(mac[1], HEX) + ":" // chip id
                   + String(mac[0], HEX);      // chip id
    os.notification.info(REALM, F("MAC"), mac_str);
    IPAddress ip = WiFi.localIP();
    String ip_str = String((ip&0x000000ff))     + "."
                  + String((ip&0x0000ff00)>>8)  + "."
                  + String((ip&0x00ff0000)>>16) + "."
                  + String((ip&0xff000000)>>24);
    os.notification.info(REALM, F("IP"), ip_str);
}

void save2server_setup() {
    os.notification.info(REALM, F("Setup ..."));
    ESPSerial.begin(9600);
    WiFi.init(&ESPSerial);
}

s2s_result_t save2server(float temperature, bool fan) {
    if (s2s_interval < S2S_INTERVAL) {
        return skip;
    }
    s2s_interval = 0;

    if (WiFi.status() == WL_NO_SHIELD) {
        return no_wifi_shield;
    }

    os.notification.info(REALM, F("Connect to WiFi"));
    int status = WiFi.begin(S2S_SSID, S2S_PASSWORD);
    if (status == WL_CONNECTED) {
        if (!PRODUCTION) { wifi_info(); }

        os.notification.info(REALM, F("Connect to Server"));
        if (client.connect(S2S_SERVER, 18086)) {
            String data = "weather,location=livingroom,location2=lowboard,logger=" +
                get_id() +
                " temperature0=" +
                String(temperature, 2) +
                ",fan0=" +
                String(fan ? "100" : "0") +
                "\n";
            os.notification.info(REALM, F("Save to Server"), data);
            client.print(F("POST /write?db="));
            #if PRODUCTION
            client.print(F("homemonitor"));
            #else
            client.print(F("test"));
            #endif
            client.print(F("&precision=s&user="));
            client.println(F(" HTTP/1.1"));
            client.println(F("Host: tokyo.local"));
            client.println(F("Content-Type: application/x-www-form-urlencoded"));
            client.print(F("Content-Length: "));
            client.println(data.length());
            client.println(F("Connection: close"));
            client.println();
            client.print(data);
            delay(200);
            while (client.available()) {
                if (!PRODUCTION) {
                    char c = client.read();
                    Serial.write(c);
                }
                else {
                    client.read();
                }
            }
            delay(200);
            os.notification.info(REALM, F("Stop client"));
            client.stop();
        }
        else {
            os.notification.warn(REALM, F("No Server connection"));
        }
        if (PRODUCTION) {
            WiFi.disconnect();
            os.notification.info(REALM, F("Disconnect from WiFi"));
        }
        return ok;
    }
    else {
        os.notification.warn(REALM, F("No WiFi connection"));
        return no_wifi_connection;
    }
}
