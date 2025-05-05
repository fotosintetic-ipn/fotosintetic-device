#include "fotosintetic.hpp"
#include "fotosintetic_server.hpp"
#include "fotosintetic_client.hpp"

void fotosintetic::init(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(resetButton, INPUT);

    WiFiClass::mode(WIFI_AP_STA);
    WiFi.softAP("FOTOSINTETIC");
    Preferences prefs;
    prefs.begin("fotosinteticPrefs");
    if(prefs.isKey("wifi_ssid") && prefs.isKey("wifi_password")
      && prefs.getString("wifi_ssid") != "" && prefs.getString("wifi_password") != ""){
        WiFi.begin(prefs.getString("wifi_ssid"), prefs.getString("wifi_password"));
        attemptingToReconnect = true;
    }
    prefs.end();
    server.init();
}

void fotosintetic::tick(){
    static int samples = 0;
    static uint64_t wifiTimer = 0;
    static uint64_t mainTimer = 0;
    static uint64_t pastTime = millis();
    static bool status;

    Preferences prefs;
    prefs.begin("fotosinteticPrefs");
    if(WiFiClass::status() == WL_CONNECTION_LOST && wifiTimer >= wifiAttemptReconnectTimeout
      && prefs.isKey("wifi_ssid") && prefs.isKey("wifi_password")
      && prefs.getString("wifi_ssid") != "" && prefs.getString("wifi_password") != ""){
        WiFi.disconnect();
        WiFi.begin(prefs.getString("wifi_ssid"), prefs.getString("wifi_password"));
        attemptingToReconnect = true;
    }
    prefs.end();
    if(!client.is_ready() && WiFiClass::status() == WL_CONNECTED){
        digitalWrite(BUILTIN_LED, HIGH);
        attemptingToReconnect = false;
        client.set();
    }
    if(client.is_ready() && WiFiClass::status() != WL_CONNECTED){
        client.reset();
        digitalWrite(BUILTIN_LED, LOW);
        attemptingToReconnect = false;
    }
    if(attemptingToReconnect && wifiTimer >= 500){
        digitalWrite(BUILTIN_LED, status);
        wifiTimer = 0;
        status = !status;
    }

    if(mainTimer >= 3600000 / samplesPerHour){
        if(digitalRead(resetButton)){
            prefs.begin("fotosinteticPrefs");
            prefs.remove("wifi_ssid");
            prefs.remove("wifi_password");
            prefs.remove("device_name");
            prefs.remove("password");
            prefs.end();

            ESP.restart();
        }

        if(phArrayCurrentIndex < uploadPackageLength){
            phArray[phArrayCurrentIndex] = 14.0 * analogReadMilliVolts(phSensor) / 3300.0;
            phArrayCurrentIndex++;
            samples++;
        }

        if(samples >= uploadPackageLength){
            if(client.is_ready())
                client.upload_data(phArray);
            phArrayCurrentIndex = 0;
            samples = 0;
        }

        mainTimer = 0;
    }

    sleep(250);

    mainTimer += millis() - pastTime;
    wifiTimer += millis() - pastTime;
    pastTime = millis();
}