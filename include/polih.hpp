#ifndef POLIH_HPP_INCLUDED_
#define POLIH_HPP_INCLUDED_

#include "polih_server.hpp"
#include "polih_client.hpp"

#include <WiFi.h>
#include <Preferences.h>

constexpr uint8_t uploadPackageLength = 10;

constexpr uint8_t resetButton = 34;

constexpr uint32_t wifiAttemptReconnectTimeout = 30000;

class polih{
    polih_server server;
    polih_client client;

    bool attemptingToReconnect = false;
public:
    polih() = default;
    void init();
    void tick();
};

#endif // POLIH_HPP_INCLUDED_