#include <Arduino.h>

#include "polih.hpp"

polih core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
}