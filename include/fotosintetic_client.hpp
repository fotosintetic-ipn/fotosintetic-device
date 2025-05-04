#ifndef FOTOSINTETIC_CLIENT_INCLUDED_
#define FOTOSINTETIC_CLIENT_INCLUDED_

#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

class fotosintetic_client{
    bool ready = false;
    const char* server_address = "";
    uint16_t server_port = 80;
    WiFiClient w;
    HttpClient client = HttpClient(w, server_address, server_port);
public:
    fotosintetic_client();
    bool is_ready() const;
    void set();
    void reset();
    void upload_data(const double* phArray);
};

#endif // FOTOSINTETIC_CLIENT_INCLUDED_