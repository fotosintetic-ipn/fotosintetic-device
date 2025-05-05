#ifndef FOTOSINTETIC_HPP_INCLUDED_
#define FOTOSINTETIC_HPP_INCLUDED_

#include "fotosintetic_server.hpp"
#include "fotosintetic_client.hpp"

#include <WiFi.h>
#include <Preferences.h>

constexpr uint8_t uploadPackageLength = 10;

constexpr uint8_t resetButton = 34;
constexpr uint8_t phSensor = 35;

constexpr uint32_t wifiAttemptReconnectTimeout = 30000;
constexpr uint32_t samplesPerHour = 1000;

class fotosintetic{
    fotosintetic_server server;
    fotosintetic_client client;

    double phArray[uploadPackageLength];
    uint8_t phArrayCurrentIndex = 0;

    bool attemptingToReconnect = false;
public:
    fotosintetic() = default;
    void init();
    void tick();
};

#endif // FOTOSINTETIC_HPP_INCLUDED_