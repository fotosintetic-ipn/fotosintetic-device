#include <Arduino.h>

#include "polih.hpp"

constexpr uint32_t samplesPerMinute = 10;

polih core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
    delay(60000 / samplesPerMinute);
}