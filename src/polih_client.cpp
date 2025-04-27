#include "polih.hpp"
#include "polih_server.hpp"
#include "polih_client.hpp"

polih_client::polih_client(){
    client.setTimeout(500);
}

bool polih_client::is_ready() const{
    return ready;
}

void polih_client::set(){
    Preferences prefs;
    prefs.begin("polihPrefs");
    if(prefs.isKey("username") && prefs.isKey("password")
      && prefs.getString("username") != "" && prefs.getString("password") != "")
        ready = true;

    prefs.end();
}

void polih_client::reset(){
    ready = false;
}

void polih_client::upload_data(const uint8_t* heartRateArray, double heartRatePrecision, const uint8_t* spo2Array, double spo2Precision){
    String contentType = "application/json";

    DynamicJsonDocument output(4096);
    output["heartRatePrecision"] = heartRatePrecision;
    output["spo2Precision"] = spo2Precision;
    output["sent"] = "10/12/2023";
    
    JsonArray heartRate = output.createNestedArray("heartRateArray");
    for(int i = 0; i != uploadPackageLength; i++)
        heartRate.add(heartRateArray[i]);

    JsonArray spo2 = output.createNestedArray("spo2Array");
    for(int i = 0; i != uploadPackageLength; i++)
        spo2.add(spo2Array[i]);

    String toSend;
    serializeJson(output, toSend);
    
    Preferences prefs;
    prefs.begin("polihPrefs");

    client.beginRequest();
    client.post("/submit");
    client.sendBasicAuth(prefs.getString("username"), prefs.getString("password"));
    client.sendHeader("Content-Type", contentType);
    client.sendHeader("Content-Length", toSend.length());
    client.print(toSend);
    client.endRequest();

    prefs.end();
}