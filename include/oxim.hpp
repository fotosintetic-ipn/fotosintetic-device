#ifndef OXIM_HPP_INCLUDED_
#define OXIM_HPP_INCLUDED_

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Stepper.h>
#include <SoftwareSerial.h>

#include "assets.h"
#include "oxim_server.hpp"
#include "oxim_client.hpp"

#include <WiFi.h>

constexpr uint64_t bufferLength = 100;
constexpr uint8_t uploadPackageLength = 10;

static_assert(bufferLength >= uploadPackageLength);

constexpr uint8_t fatalErrorLED = 26;
constexpr uint8_t resetButton = 34;
constexpr uint8_t bypassSensor = 27;
constexpr uint8_t spo2Potentiometer = 32;
constexpr uint8_t heartRatePotentiometer = 33;

constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr int8_t OLED_RESET = -1;
constexpr uint8_t SCREEN_ADDRESS = 0x3C; // < See datasheet for Address
constexpr uint8_t NUMBER_ASSET_WIDTH = 18;
constexpr uint8_t NUMBER_ASSET_HEIGHT = 18;
constexpr uint8_t FIRST_COLUMN_X_COORDINATE = 73;
constexpr uint8_t SECOND_COLUMN_X_COORDINATE = FIRST_COLUMN_X_COORDINATE + NUMBER_ASSET_WIDTH;
constexpr uint8_t THIRD_COLUMN_X_COORDINATE = FIRST_COLUMN_X_COORDINATE + 2 * NUMBER_ASSET_WIDTH;
constexpr uint8_t FIRST_ROW_Y_COORDINATE = 5;
constexpr uint8_t SECOND_ROW_Y_COORDINATE = FIRST_ROW_Y_COORDINATE + 2 * NUMBER_ASSET_HEIGHT;

constexpr uint32_t wifiAttemptReconnectTimeout = 30000;

constexpr int32_t valveStepsPerRevolution = 4096;

constexpr uint32_t smsWaitTime = 90000;

class oxim{
    uint32_t irBuffer[bufferLength];
    uint32_t redBuffer[bufferLength];
    int32_t spo2;
    int32_t lastValidSpo2 = 0;
    int8_t isSpo2Valid;
    int32_t heartRate;
    int32_t lastValidHeartRate = 0;
    int8_t isHeartRateValid;
    MAX30105 particleSensor;

    bool isDisplayOk = true;
    Adafruit_SSD1306 display;

    Stepper valve = Stepper(0, 0, 0, 0, 0);

    int sim900Strikes = 0;
    SoftwareSerial sim900;

    uint8_t spo2Precision = 0;
    uint8_t spo2ArrayCurrentIndex = 0;
    uint8_t spo2Array[uploadPackageLength];
    uint8_t heartRatePrecision = 0;
    uint8_t heartRateArrayCurrentIndex = 0;
    uint8_t heartRateArray[uploadPackageLength];

    oxim_server server;
    oxim_client client;

    bool attemptingToReconnect = false;
public:
    oxim();
    void configure_particle_sensor();
    void configure_screen();
    void configure_valve();
    void configure_sim900();

    void get_data_from_particle_sensor(uint64_t count, uint64_t discard);

    void display_information(uint64_t const& saturation, uint64_t const& heartbeat);

    bool check_sim900_response(String const& toCheck = String("OK"), uint32_t const& offset = 4);
    void send_sms(String const& content);

    void init();
    void tick();
};

#endif // OXIM_HPP_INCLUDED_