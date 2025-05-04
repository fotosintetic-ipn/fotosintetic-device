#ifndef FOTOSINTETIC_SERVER_INCLUDED_
#define FOTOSINTETIC_SERVER_INCLUDED_

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

class fotosintetic_server{
    AsyncWebServer server = AsyncWebServer(80);
public:
    fotosintetic_server() = default;
    void init();
    static void notFound(AsyncWebServerRequest *request);
    static void connect(AsyncWebServerRequest* request);
    static void credentials(AsyncWebServerRequest* request);
};

#endif // FOTOSINTETIC_SERVER_INCLUDED_