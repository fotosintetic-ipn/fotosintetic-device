#include <Arduino.h>

#include "fotosintetic.hpp"

fotosintetic core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
}