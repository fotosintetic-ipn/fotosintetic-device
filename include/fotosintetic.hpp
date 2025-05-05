#ifndef FOTOSINTETIC_HPP_INCLUDED_
#define FOTOSINTETIC_HPP_INCLUDED_

#include "fotosintetic_server.hpp"
#include "fotosintetic_client.hpp"

#include <WiFi.h>
#include <Preferences.h>
#include <DHT.h>
#include <Wire.h>
#include <MPU6050.h>

constexpr uint8_t uploadPackageLength = 10;

constexpr uint8_t resetButton = 39;
constexpr uint8_t phSensor = 34;
constexpr uint8_t dhtSensor = 32;
constexpr uint8_t moistureSensor = 35;
constexpr uint8_t anemometer = 36;

constexpr uint32_t wifiAttemptReconnectTimeout = 30000;
constexpr uint32_t samplesPerHour = 1000;

class fotosintetic{
    fotosintetic_server server;
    fotosintetic_client client;

    DHT dht = DHT(0, 0);
    MPU6050 mpu;

    double ph[uploadPackageLength];
    double ambientHumidity[uploadPackageLength];
    double ambientTemperature[uploadPackageLength];
    double roll[uploadPackageLength];
    double pitch[uploadPackageLength];
    double moisture[uploadPackageLength];
    double windSpeed[uploadPackageLength];

    uint8_t arrayIndex = 0;

    bool attemptingToReconnect = false;
public:
    fotosintetic() = default;
    void init();
    void tick();
};

#endif // FOTOSINTETIC_HPP_INCLUDED_