#ifndef FOTOSINTETIC_CLIENT_INCLUDED_
#define FOTOSINTETIC_CLIENT_INCLUDED_

#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

class fotosintetic_client{
    bool ready = false;
    const char* server_address = "enhanced-muskrat-uncommon.ngrok-free.app";
    uint16_t server_port = 80;
    WiFiClient w;
    HttpClient client = HttpClient(w, server_address, server_port);
public:
    fotosintetic_client();
    bool is_ready() const;
    void set();
    void reset();
    void upload_data(const double* ph, const double* ambientHumidity, const double* ambientTemperature,
                     const double* roll, const double* pitch, const double* moisture, 
                     const double* windSpeed);
};

#endif // FOTOSINTETIC_CLIENT_INCLUDED_