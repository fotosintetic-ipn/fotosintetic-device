#include "oxim.hpp"
#include "oxim_server.hpp"
#include "oxim_client.hpp"

oxim_client::oxim_client(){
    client.setTimeout(500);
}

bool oxim_client::is_ready() const{
    return ready;
}

void oxim_client::set(){
    Preferences prefs;
    prefs.begin("oximPrefs");
    if(prefs.isKey("username") && prefs.isKey("password")
      && prefs.getString("username") != "" && prefs.getString("password") != "")
        ready = true;

    prefs.end();
}

void oxim_client::reset(){
    ready = false;
}

void oxim_client::upload_data(const uint8_t* heartRateArray, double heartRatePrecision, const uint8_t* spo2Array, double spo2Precision){
    Serial.println("upload_data");
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

    Serial.println(toSend);
    
    Preferences prefs;
    prefs.begin("oximPrefs");

    client.beginRequest();
    client.post("/submit");
    client.sendBasicAuth(prefs.getString("username"), prefs.getString("password"));
    client.sendHeader("Content-Type", contentType);
    client.sendHeader("Content-Length", toSend.length());
    client.print(toSend);
    client.endRequest();

    prefs.end();

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
}