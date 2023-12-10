#ifndef OXIM_CLIENT_INCLUDED_
#define OXIM_CLIENT_INCLUDED_

#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

class oxim_client{
    bool ready = false;
    const char* server_address = "";
    uint16_t server_port = 80;
    WiFiClient w;
    HttpClient client = HttpClient(w, server_address, server_port);
public:
    oxim_client();
    bool is_ready() const;
    void set();
    void reset();
    void upload_data(const uint8_t* heartRateArray, double heartRatePrecision, const uint8_t* spo2Array, double spo2Precision);
};

#endif // OXIM_CLIENT_INCLUDED_