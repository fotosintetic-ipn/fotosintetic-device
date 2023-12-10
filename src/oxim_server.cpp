#include "oxim.hpp"
#include "oxim_server.hpp"
#include "oxim_client.hpp"

void oxim_server::init(){
    server.on("/connect", HTTP_POST, connect);
    server.on("/credentials", HTTP_POST, credentials);
    server.onNotFound(notFound);
    server.begin();
}

void oxim_server::notFound(AsyncWebServerRequest *request){
    request -> send(404, "text/plain", "Not found");
}

void oxim_server::connect(AsyncWebServerRequest* request){
    if(!request -> hasParam("ssid") || !request -> hasParam("password")){
        request -> send(400, "text/plain", "Missing ssid or password.");
        return;
    }

    WiFi.disconnect();
    WiFi.begin(request -> getParam("ssid") -> value(), request -> getParam("password") -> value());

    Preferences prefs;
    prefs.begin("oximPrefs");
    prefs.putString("wifi_ssid", request -> getParam("ssid") -> value());
    prefs.putString("wifi_password", request -> getParam("password") -> value());
    prefs.end();

    request -> send("text/plain", 3, [](uint8_t* buffer, size_t, size_t){
        static bool indicator = false;
        if(WiFiClass::status() == WL_CONNECTED){
            memcpy(buffer, "200", 3);
            digitalWrite(LED_BUILTIN, HIGH);
            return 3;
        }
        if(WiFiClass::status() == WL_CONNECT_FAILED){
            memcpy(buffer, "401", 3);
            digitalWrite(LED_BUILTIN, LOW);
            return 3;
        }
        digitalWrite(LED_BUILTIN, indicator);
        indicator = !indicator;
        return 0;
    });
}

void oxim_server::credentials(AsyncWebServerRequest* request){
    if(!request -> hasParam("username") || !request -> hasParam("password")){
        request -> send(400, "text/plain", "Missing username or password");
        return;
    }

    Preferences prefs;
    prefs.begin("oximPrefs");
    prefs.putString("username", request -> getParam("username") -> value());
    prefs.putString("password", request -> getParam("password") -> value());
    prefs.end();

    request -> send(200);
}