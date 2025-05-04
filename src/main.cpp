#include <Arduino.h>

#include "fotosintetic.hpp"

constexpr uint32_t samplesPerMinute = 10;

fotosintetic core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
    delay(60000 / samplesPerMinute);
}