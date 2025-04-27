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
    if(prefs.isKey("id") && prefs.isKey("password")
      && prefs.getString("id") != "" && prefs.getString("password") != "")
        ready = true;

    prefs.end();
}

void polih_client::reset(){
    ready = false;
}

void polih_client::upload_data(const double* phArray){
    String contentType = "application/json";

    DynamicJsonDocument output(4096);

    JsonArray ph = output.createNestedArray("ph");
    for(int i = 0; i != uploadPackageLength; i++)
        ph.add(phArray[i]);

    String toSend;
    serializeJson(output, toSend);

    Preferences prefs;
    prefs.begin("polihPrefs");

    client.beginRequest();
    client.post("/submit");
    client.sendBasicAuth(prefs.getString("id"), prefs.getString("password"));
    client.sendHeader("Content-Type", contentType);
    client.sendHeader("Content-Length", toSend.length());
    client.print(toSend);
    client.endRequest();

    prefs.end();
}