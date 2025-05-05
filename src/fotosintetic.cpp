#include "fotosintetic.hpp"
#include "fotosintetic_server.hpp"
#include "fotosintetic_client.hpp"

void fotosintetic::init(){
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(resetButton, INPUT);
    pinMode(phSensor, INPUT);

    dht = DHT(dhtSensor, DHT11);
    Wire.begin();
    mpu.initialize();

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

    if(digitalRead(resetButton)){
        prefs.begin("fotosinteticPrefs");
        prefs.remove("wifi_ssid");
        prefs.remove("wifi_password");
        prefs.remove("device_name");
        prefs.remove("password");
        prefs.end();

        ESP.restart();
    }

    if(mainTimer >= 3600000 / samplesPerHour){
        if(arrayIndex < uploadPackageLength){
            ph[arrayIndex] = 14.0 * analogReadMilliVolts(phSensor) / 3300.0;

            if(!isnan(dht.readHumidity()))
                ambientHumidity[arrayIndex] = dht.readHumidity();

            if(!isnan(dht.readTemperature()))
                ambientTemperature[arrayIndex] = dht.readTemperature();
            
            int16_t ax;
            int16_t ay;
            int16_t az;
            mpu.getAcceleration(&ax, &ay, &az);
            double axg = ax / 16384.0;
            double ayg = ay / 16384.0;
            double azg = az / 16384.0;
            double rollValue = atan2(ayg, azg) * 180.0 / PI;
            double pitchValue = atan2(-axg, sqrt(ayg * ayg + azg * azg)) * 180.0 / PI;
            roll[arrayIndex] = rollValue;
            pitch[arrayIndex] = pitchValue;

            moisture[arrayIndex] = map(analogReadMilliVolts(moistureSensor), 1200, 2300, 0, 100);
            windSpeed[arrayIndex] = map(analogReadMilliVolts(anemometer), 0, 3300, 0, 40);

            arrayIndex++;
        }

        if(arrayIndex >= uploadPackageLength){
            if(client.is_ready())
                client.upload_data(ph, ambientHumidity, ambientTemperature,
                                   roll, pitch, moisture, windSpeed);
            arrayIndex = 0;
        }

        mainTimer = 0;
    }

    sleep(250);

    mainTimer += millis() - pastTime;
    wifiTimer += millis() - pastTime;
    pastTime = millis();
}