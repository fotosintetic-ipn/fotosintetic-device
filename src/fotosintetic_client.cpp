#include "fotosintetic.hpp"
#include "fotosintetic_server.hpp"
#include "fotosintetic_client.hpp"

fotosintetic_client::fotosintetic_client(){
    client.setTimeout(500);
}

bool fotosintetic_client::is_ready() const{
    return ready;
}

void fotosintetic_client::set(){
    Preferences prefs;
    prefs.begin("fotosintetic");
    if(prefs.isKey("device_name") && prefs.isKey("password")
      && prefs.getString("device_name") != "" && prefs.getString("password") != "")
        ready = true;

    prefs.end();
}

void fotosintetic_client::reset(){
    ready = false;
}

void fotosintetic_client::upload_data(const double* ph, const double* ambientHumidity, const double* ambientTemperature,
                                      const double* roll, const double* pitch, const double* moisture,
                                      const double* windSpeed){
    DynamicJsonDocument output(4096);

    JsonArray phJson = output.createNestedArray("ph");
    JsonArray ambientHumidityJson = output.createNestedArray("ambient_humidity");
    JsonArray ambientTemperatureJson = output.createNestedArray("ambient_temperature");
    JsonArray rollJson = output.createNestedArray("roll");
    JsonArray pitchJson = output.createNestedArray("pitch");
    JsonArray moistureJson = output.createNestedArray("moisture");
    JsonArray windSpeedJson = output.createNestedArray("wind_speed");
    for(int i = 0; i != uploadPackageLength; i++){
        phJson.add(ph[i]);
        ambientHumidityJson.add(ambientHumidity[i]);
        ambientTemperatureJson.add(ambientTemperature[i]);
        rollJson.add(roll[i]);
        pitchJson.add(pitch[i]);
        moistureJson.add(moisture[i]);
        windSpeedJson.add(windSpeed[i]);
    }

    String toSend;
    serializeJson(output, toSend);

    Preferences prefs;
    prefs.begin("fotosintetic");

    client.beginRequest();
    client.post("/submit");
    client.sendBasicAuth(prefs.getString("device_name"), prefs.getString("password"));
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", toSend.length());
    client.print(toSend);
    client.endRequest();

    prefs.end();
}