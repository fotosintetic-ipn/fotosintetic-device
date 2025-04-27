#ifndef POLIH_SERVER_INCLUDED_
#define POLIH_SERVER_INCLUDED_

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class polih_server{
    AsyncWebServer server = AsyncWebServer(80);
public:
    polih_server() = default;
    void init();
    static void notFound(AsyncWebServerRequest *request);
    static void connect(AsyncWebServerRequest* request);
    static void credentials(AsyncWebServerRequest* request);
};

#endif // POLIH_SERVER_INCLUDED_