#ifndef OXIM_SERVER_INCLUDED_
#define OXIM_SERVER_INCLUDED_

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class oxim_server{
    AsyncWebServer server = AsyncWebServer(80);
public:
    oxim_server() = default;
    void init();
    static void notFound(AsyncWebServerRequest *request);
    static void connect(AsyncWebServerRequest* request);
    static void credentials(AsyncWebServerRequest* request);
};

#endif // OXIM_SERVER_INCLUDED_