#include <Arduino.h>
#include <Preferences.h>

#include "assets.h"
#include "polih.hpp"
#include "polih_server.hpp"

polih core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
}