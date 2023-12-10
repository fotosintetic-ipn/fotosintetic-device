#include <Arduino.h>
#include <Preferences.h>

#include "assets.h"
#include "oxim.hpp"
#include "oxim_server.hpp"

oxim core;

void setup(){
    Serial.begin(115200);
    core.init();
}

void loop(){
    core.tick();
}