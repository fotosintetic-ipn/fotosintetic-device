#include "oxim.hpp"
#include "oxim_server.hpp"
#include "oxim_client.hpp"

oxim::oxim() : sim900(4, 5) {}

void oxim::configure_particle_sensor(){
    if(!particleSensor.begin(Wire, I2C_SPEED_FAST)){
        digitalWrite(fatalErrorLED, HIGH);
        while(true);
    }

    uint8_t ledBrightness = 60; // 0=Off to 255=50mA
    uint8_t sampleAverage = 4; // 1, 2, 4, 8, 16, 32
    uint8_t ledMode = 2; // 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    uint8_t sampleRate = 100; // 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411; // 69, 118, 215, 411
    int adcRange = 4096; // 2048, 4096, 8192, 16384
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

    get_data_from_particle_sensor(100, 0);
}

void oxim::configure_screen(){
    display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        isDisplayOk = false;
}

void oxim::configure_valve(){
    valve = Stepper(2048, 23, 25, 19, 18);
    valve.setSpeed(10);
}

void oxim::configure_sim900(){
    sim900.begin(9600);
    sim900.println((char) 27); // Cancel any previous ongoing operations

    while(sim900Strikes <= 3) {
        sim900.println("AT");
        if(!check_sim900_response())
            continue;
        
        sim900.println("AT+CSQ");
        if(!check_sim900_response())
            continue;

        sim900.println("AT+CCID");
        if(!check_sim900_response())
            continue;

        sim900.println("AT+CREG?");
        if(!check_sim900_response())
            continue;
        else break;
    }
    sim900Strikes = 0;
}

void oxim::get_data_from_particle_sensor(uint64_t count, uint64_t discard){
    if(discard != 0)
        for(uint64_t i = discard; i < bufferLength; i++){
            redBuffer[i - discard] = redBuffer[i];
            irBuffer[i - discard] = irBuffer[i];
        }

    for(uint64_t i = bufferLength - count; i < bufferLength; i++){
        while (particleSensor.available() == false)
            particleSensor.check();

        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        particleSensor.nextSample();
    }
}

void oxim::display_information(uint64_t const& saturation, uint64_t const& heartbeat){
    if(!isDisplayOk) return;

    display.clearDisplay();
    display.drawBitmap(0, 0, mainBitmap, 128, 64, 1);

    display.drawBitmap(FIRST_COLUMN_X_COORDINATE, FIRST_ROW_Y_COORDINATE, numbers[saturation / 100], 18, 18, 1);
    display.drawBitmap(SECOND_COLUMN_X_COORDINATE, FIRST_ROW_Y_COORDINATE, numbers[saturation % 100 / 10], 18, 18, 1);
    display.drawBitmap(THIRD_COLUMN_X_COORDINATE, FIRST_ROW_Y_COORDINATE, numbers[saturation % 10], 18, 18, 1);

    display.drawBitmap(FIRST_COLUMN_X_COORDINATE, SECOND_ROW_Y_COORDINATE, numbers[heartbeat / 100], 18, 18, 1);
    display.drawBitmap(SECOND_COLUMN_X_COORDINATE, SECOND_ROW_Y_COORDINATE, numbers[heartbeat % 100 / 10], 18, 18, 1);
    display.drawBitmap(THIRD_COLUMN_X_COORDINATE, SECOND_ROW_Y_COORDINATE, numbers[heartbeat % 10], 18, 18, 1);

    display.display();
}

bool oxim::check_sim900_response(String const& toCheck, uint32_t const& offset){
    delay(500);
    if(!sim900.available()){
        sim900Strikes++;
        return false;
    }

    String response = sim900.readString();
    String status = response.substring(response.length() - offset,response.length() - offset + toCheck.length());
    if(!toCheck.equals(status)){
        sim900Strikes++;
        return false;
    }
    return true;
}

void oxim::send_sms(String const& content){
    String phone_number;

    Preferences prefs;
    prefs.begin("oximPrefs");
    if(!prefs.isKey("phone_number") || prefs.getString("phone_number") == "")
        return;
    phone_number = prefs.getString("phone_number");
    prefs.end();

    while(sim900Strikes <= 3) {
        sim900.println((char) 27); // Cancel any previous ongoing operations

        sim900.println("AT+CMGF=1");
        if(!check_sim900_response())
            continue;
        
        sim900.print("AT+CMGS=\"");
        sim900.print(phone_number);
        sim900.println("\"\r");
        if(!check_sim900_response(String(">"), 2))
            continue;

        sim900.println(content);
        if(!check_sim900_response(String(">"), 2))
            continue;

        sim900.println((char)26); // Send
        break;
    }
    sim900Strikes = 0;
}

void oxim::init(){
    pinMode(fatalErrorLED, OUTPUT);
    digitalWrite(fatalErrorLED, LOW);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(resetButton, INPUT);
    pinMode(bypassSensor, INPUT);
    pinMode(spo2Potentiometer, INPUT);
    pinMode(heartRatePotentiometer, INPUT);

    configure_particle_sensor();
    configure_screen();
    display_information(0, 0);
    configure_valve();
    configure_sim900();
    WiFiClass::mode(WIFI_AP_STA);
    WiFi.softAP("OXIM");
    Preferences prefs;
    prefs.begin("oximPrefs");
    if(prefs.isKey("wifi_ssid") && prefs.isKey("wifi_password")
      && prefs.getString("wifi_ssid") != "" && prefs.getString("wifi_password") != ""){
        WiFi.begin(prefs.getString("wifi_ssid"), prefs.getString("wifi_password"));
        attemptingToReconnect = true;
    }
    prefs.end();
    server.init();
}

void oxim::tick(){
    static int samples = 0;
    static uint64_t timeElapsed = 0;
    static uint64_t pastTime = millis();
    static bool status;
    static uint8_t valveCurrentPosition = 5;
    static uint64_t lastSentSms = 90000;

    get_data_from_particle_sensor(25, 25);
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &isSpo2Valid, &heartRate, &isHeartRateValid);
    if(digitalRead(bypassSensor)){
        isSpo2Valid = true;
        isHeartRateValid = true;
        spo2 = constrain(map(analogRead(spo2Potentiometer), 200, 4000, 0, 100), 0, 100);
        heartRate = constrain(map(analogRead(heartRatePotentiometer), 200, 4000, 0, 255), 0, 255);
    }
    samples++;

    if(isSpo2Valid && spo2ArrayCurrentIndex < uploadPackageLength) spo2Precision++;
    if(isHeartRateValid && heartRateArrayCurrentIndex < uploadPackageLength) heartRatePrecision++;

    lastValidSpo2 = isSpo2Valid ? spo2 : lastValidSpo2;
    lastValidHeartRate = isHeartRateValid ? heartRate : lastValidHeartRate;
    display_information(lastValidSpo2, lastValidHeartRate);

    heartRateArrayCurrentIndex = min((uint8_t)(uploadPackageLength - 1), heartRateArrayCurrentIndex);
    spo2ArrayCurrentIndex = min((uint8_t)(uploadPackageLength - 1), spo2ArrayCurrentIndex);

    heartRateArray[heartRateArrayCurrentIndex] = static_cast<uint8_t>(min(lastValidHeartRate, 255));
    heartRateArrayCurrentIndex++;
    spo2Array[spo2ArrayCurrentIndex] = static_cast<uint8_t>(lastValidSpo2);
    spo2ArrayCurrentIndex++;

    if(!client.is_ready() && WiFiClass::status() == WL_CONNECTED){
        digitalWrite(BUILTIN_LED, HIGH);
        attemptingToReconnect = false;
        client.set();
    }
    if(client.is_ready() && WiFiClass::status() != WL_CONNECTED){
        client.reset();
        digitalWrite(BUILTIN_LED, LOW);
        attemptingToReconnect = false;
    }
    Preferences prefs;
    prefs.begin("oximPrefs");
    if(WiFiClass::status() == WL_CONNECTION_LOST && timeElapsed >= wifiAttemptReconnectTimeout
      && prefs.isKey("wifi_ssid") && prefs.isKey("wifi_password")
      && prefs.getString("wifi_ssid") != "" && prefs.getString("wifi_password") != ""){
        WiFi.disconnect();
        WiFi.begin(prefs.getString("wifi_ssid"), prefs.getString("wifi_password"));
        attemptingToReconnect = true;
    }
    prefs.end();

    if(digitalRead(resetButton)){
        prefs.begin("oximPrefs");
        prefs.remove("wifi_ssid");
        prefs.remove("wifi_password");
        prefs.remove("username");
        prefs.remove("password");
        prefs.remove("phone_number");
        prefs.end();

        ESP.restart();
    }
    
    if(attemptingToReconnect && timeElapsed / 500 >= 1){
        digitalWrite(BUILTIN_LED, status);
        status = !status;
    }

    if(samples >= uploadPackageLength){
        int spo2Average = 0;
        int heartRateAverage = 0;
        for(int i = 0; i != uploadPackageLength; i++){
            spo2Average += spo2Array[i];
            heartRateAverage += heartRateArray[i];
        }
        spo2Average /= uploadPackageLength;
        heartRateAverage /= uploadPackageLength;

        if(spo2Average < 90 && valveCurrentPosition < 10){
            valve.step(valveStepsPerRevolution / 12);
            valveCurrentPosition++;
        }
        else if(spo2Average > 95 && valveCurrentPosition > 0){
            valve.step(-valveStepsPerRevolution / 12);
            valveCurrentPosition--;
        }

        if(lastSentSms >= smsWaitTime){
            if(spo2Average < 75){
                send_sms("Oxygen abnormally low!\nCurrent value: " + String(spo2Average) + "%");
                delay(2500);
            }
            if(heartRateAverage <= 50)
                send_sms("Heart rate abnormally low!\nCurrent value: " + String(heartRateAverage) + "bpm");
            if(heartRateAverage >= 110)
                send_sms("Heart rate abnormally high!\nCurrent value: " + String(heartRateAverage) + "bpm");
            lastSentSms = 0;
        }    

        if(client.is_ready())
            client.upload_data(heartRateArray, heartRatePrecision / static_cast<double>(uploadPackageLength),
                            spo2Array, spo2Precision / static_cast<double>(uploadPackageLength));

        heartRatePrecision = 0;
        spo2Precision = 0;
        heartRateArrayCurrentIndex = 0;
        spo2ArrayCurrentIndex = 0;
        samples = 0;
    }

    lastSentSms += millis() - pastTime;
    timeElapsed += millis() - pastTime;
    pastTime = millis();
}