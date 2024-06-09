#include <Arduino.h>
#include <Preferences.h>

#include "assets.h"
#include "oxim.hpp"
#include "oxim_server.hpp"

oxim core;

void setup(){
    core.init();
}

void loop(){
    core.tick();
}