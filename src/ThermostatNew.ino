#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <pgmspace.h>
#include <RtcDS1307.h>
#include <U8g2lib.h>
#include <OneWire.h>
#include <Wire.h>
#include <EEPROM.h>


#include "Pins.h"
#include "Bitmaps.h"
#include "Temperature.h"
#include "Rtc.h"
#include "Display.h"
#include "Memory.h"
#include "html.h"
#include "WebClient.h"
#include "WebServer.h"



//Display dimensions
#define WIDTH 128
#define HEIGHT 64

//Input pins
#define ENCODER_A D3
#define ENCODER_B D4
#define ENCODER_SW D6
#define RELAY D7

//Intervals
#define UPLOAD_INTERVAL_MINUTES 30
#define DOWNLOAD_INTERVAL_MINUTES 5


//UI related variables
byte ui_page_index = 0;
long ui_redraw_last = 0;
bool ui_redraw_require = true;
bool ui_editing_temp = false;
int ui_wifi_rssi = -70;

//Sensor related variables
int volatile sensor_encoder_pos = 0;
int volatile sensor_encoder_last = 0;
int volatile sensor_button_state = 0; //Button states are: 0 = not pressed, 1 = short pressed, 2 = long pressed;
long volatile sensor_button_down_time = 0;
int volatile sensor_button_state_last = HIGH;
bool volatile sensor_button_hasBeenCounted = false;

int sensor_temperature_counter = 0;
long sensor_temperature_last = 0;
float sensor_temperature_sum = 0;
float sensor_temperature_avg = 0;


//Misc
long server_update_last = 0;
long server_download_last = 0;
bool wifi_connected = false;
byte op_mode = 0; //Operating mode: 0 = Normal mode, 1 = credentials insertion mode

//Heating related variables
bool heating_on = false;
byte heating_mode = 0; //Modes are: 0 = TOGGLE, 1 = timer, 2 = temperature based
int heating_timer_target;
int heating_timer_minutes;
float heating_temperature_target = 20.0f;

void setup(void) {
    //Begin serial communication at 115200 baud
    Serial.begin(115200);

    //Begin function for the display library
    u8g2.begin();
    drawBoot("BOOTING...");


    //Setup function for the RTC library
    RtcSetup();

    //Pin setup
    pinSetup();

    //If the credentials are stored on the eeprom boot normally
    if(areCredentialsStored()){
        //Read the credentials from the eeprom
        readCredentials();

        //Start wifi connect
        WiFi.begin(getSSID(), getPassword());

        Serial.print("Connecting to WiFi");
        drawBoot("CONNECTING...");

        //Try to connect for ten seconds, if it can't boot anyway
        for(int i = 0; i < 10; i++){
            if(WiFi.status() == WL_CONNECTED){
                drawBoot("CONNECTED!");
                i = 10;
            }
            delay(1000);
        }

        //Set the operational mode to 0 (normal)
        op_mode = 0;

        //Prints to the serial port the connection details
        Serial.println ( "" );
        Serial.print ( "Connected to " );
        Serial.println ( WiFi.SSID() );
        Serial.print ( "IP address: " );
        Serial.println ( WiFi.localIP() );

    }else{ //Boot into credentials insertion mode
        const char *ssid = "Thermostat-A03D";
        const char *password = "password";
        WiFi.softAP(ssid, password);

        //Call the server setup function
        serverSetup();
        Serial.println("HTTP Server started");

        drawCredentialsPage();
        //Set the operational mode to 1 (credentials insertion)
        op_mode = 1;
    }

    //Starts mdns server
    if(MDNS.begin("esp8266")){
        Serial.println("MDNS responder started");
    }

}

void loop() {
    if(op_mode == 1){
        //Handle client connection
        server.handleClient();
    }
    else{
        //Turns off soft ap if it's running
        if(WiFi.softAPdisconnect(true))
        //Button check code
        if(sensor_button_state_last == LOW && !sensor_button_hasBeenCounted){ //If it's currently being pressed
        if(millis() - sensor_button_down_time > 1000){ //If the button has been pressed for more than 1,0s count it as a long press
            sensor_button_state = 2;
            requireRedraw(true);
            sensor_button_hasBeenCounted = true;
        }
    }

    //If the button press was short
    if(sensor_button_state == 1){
        //ignore the press if the device is in temperature editing mode
        if(!ui_editing_temp){
            ui_page_index = (ui_page_index+1)%4;
            Serial.println("Button short press");
        }

    } //If the button press was long
    else if(sensor_button_state == 2){
        Serial.println("Button long press");
        switch(ui_page_index){

            //Temperature page
            case 0:
                //If you're already editing the temperature switch "on" (it obviously depends on the target temperature) the thermostat
                if(ui_editing_temp){
                    ui_editing_temp = false;
                    heating_mode = 2;
                }
                //If you are not enter the editing mode
                else{
                    ui_editing_temp = true;
                    heating_mode = 2;
                }
            break;

            //On off page
            case 1:
                setHeating(0, !heating_on);
            break;

            //Timer page
            case 2:
                heating_timer_target = getCurrentTime();
                heating_timer_target += heating_timer_minutes*60;
                setHeating(1, true);
            break;

            //Info/reset page
            case 3:
                clearEEPROM();
                ESP.restart();
            break;
        }
    }
    //Reset the button state
    sensor_button_state = 0;

    //Update heating status
    //If the mode is 1 (timer) check if the timer has expired
    if(heating_mode == 1 && heating_on){
        heating_timer_minutes = (heating_timer_target - getCurrentTime().TotalSeconds())/60;
        if(getCurrentTime().TotalSeconds() >= heating_timer_target){
            setHeating(1, false);
        }
    }
    //If the mode is 2 (temp) check if the temperature has been reached or if the temperature has fallen below the target
    else if(heating_mode == 2){
        if(heating_temperature_target > sensor_temperature_avg && !heating_on){
            setHeating(2, true);
        }
        if(heating_temperature_target <= sensor_temperature_avg && heating_on){
            setHeating(2, false);
        }
    }

    //Redraw the UI every 10 seconds (10s), when required or when editing call it faster
    if(millis() - ui_redraw_last > 10000 || ui_redraw_require || ui_editing_temp){

        //Update wifi rssi variable
        if(WiFi.status() != WL_CONNECTED){
            ui_wifi_rssi = 99;
        }else{
            ui_wifi_rssi = WiFi.RSSI();
        }

        Serial.println(ui_wifi_rssi);



        //Clear the screen
        u8g2.clearBuffer();

        //Main draw functions
        drawMenu(ui_page_index, heating_on);
        drawStatusBar(ui_wifi_rssi);
        drawMode(heating_mode);

        /*
        * Page specific draw functions
        */

        if(ui_page_index == 0){
            //If you are editing the temperature target
            if(ui_editing_temp){
                drawTempPage(heating_temperature_target, ui_editing_temp);
            }else{
                drawTempPage(sensor_temperature_avg, ui_editing_temp);
            }

        }
        else if(ui_page_index == 1){
            drawOnOffPage(heating_on);
        }
        else if(ui_page_index == 2){
            drawTimerPage(heating_timer_minutes);
        }
        else if(ui_page_index == 3){
            drawInfoPage();
        }

        //Draw on the screen
        u8g2.sendBuffer();

        //Sets timing variable only if the redraw happened because of the regular interval
        if(!ui_redraw_require){
            ui_redraw_last = millis();
            Serial.println("UI redrawn because of timeout");
        }else{
            Serial.println("UI redrawn because of request");
        }
        requireRedraw(false);
    }

    //Reads temperature every 5 seconds (5s) and every 4 times (20s) it updates the average variable
    if(millis() - sensor_temperature_last > 5000){
        if(sensor_temperature_counter == 4){
            sensor_temperature_counter = 0;
            sensor_temperature_avg = sensor_temperature_sum/4.0f;
            sensor_temperature_sum = 0;
        }else{
            sensor_temperature_sum += getCurrentTemperature();
            sensor_temperature_counter ++;
        }
        sensor_temperature_last = millis();
    }

    //Send the data to the server every :UPLOAD_INTERVAL_MINUTES
    if(millis() - server_update_last > (60000 * UPLOAD_INTERVAL_MINUTES) ){
        sendDataToServer(sensor_temperature_avg, heating_on ? 1 : 0);
        server_update_last = millis();
    }

    //Gets the command from the server and executes it

    if(millis() - server_download_last > (60000 * DOWNLOAD_INTERVAL_MINUTES) ){
        server_download_last = millis();

        struct command c = getCommandFromServer();


        int ID = c.ID;


        const char* type = c.type;
        Serial.println(c.type);

        if(String(c.type) == "toggle"){

            int value = c.value;
            setHeating(0, (value == 1) ? true:false);
        }else if(String(c.type) == "timer"){

            int value = c.value;
            heating_timer_target = getCurrentTime();
            heating_timer_target += value*60;
            setHeating(1, true);
        }else if(String(c.type) == "temp"){

            float value = c.value;
            heating_temperature_target = value;
            heating_mode = 2;
        }





    }

    delay(50);
}

}

//Pin setup function
void pinSetup(){
    pinMode(RELAY, OUTPUT);

    //Set pin mode for the encoder
    pinMode(ENCODER_A, INPUT);
    digitalWrite(ENCODER_A, HIGH);
    pinMode(ENCODER_B, INPUT);
    digitalWrite(ENCODER_B, HIGH);
    pinMode(ENCODER_SW, INPUT);

    //Attach interrupt for the encoder knob and push button
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), doEncoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), doSwitch, CHANGE);
}



//Turn the heating on or off
void setHeating(byte h_mode, bool on){
    heating_mode = h_mode;

    switch(h_mode){
        //Toggle
        case 0:
        digitalWrite(RELAY, on ? HIGH:LOW);
        heating_on = on;
        break;

        //Timer
        case 1:

        digitalWrite(RELAY, on ? HIGH:LOW);
        heating_on = on;
        break;

        case 2:
        digitalWrite(RELAY, on ? HIGH:LOW);
        heating_on = on;
        break;


    }
}

//Interrupt function for the rotary encoder
void doEncoder() {
    //Debouncing
    if(millis() - sensor_encoder_last > 25){
        //Increase or decrease the position depending on the encoder pins state
        if (digitalRead(ENCODER_A) == digitalRead(ENCODER_B)) {
            sensor_encoder_pos++;

            if(ui_page_index == 0 && ui_editing_temp){
                heating_temperature_target += 0.5f;
            }
            //If you are on the timer page and the heating is in timer mode but off or in other modes
            if(ui_page_index == 2 && !(heating_on && heating_mode == 1)){
                heating_timer_minutes += 1;
            }
        } else{
            sensor_encoder_pos--;

            if(ui_page_index == 0 && ui_editing_temp){
                heating_temperature_target -= 0.5f;
            }

            if(ui_page_index == 2){
                if(heating_timer_minutes > 0){
                    heating_timer_minutes -= 1;
                }
            }
        }
        requireRedraw(true);
    }
    sensor_encoder_last = millis();



}

//Interrupt function for the switch button
void doSwitch(){

    //If the switch has been pressed down, we save the current millis() and we set the last state
    if(digitalRead(ENCODER_SW) == LOW && sensor_button_state_last == HIGH){
        if(op_mode == 1){
            drawQR();
        }
        sensor_button_down_time = millis();
        sensor_button_state_last = LOW;
    }

    //If the switch has been released and the last state was low, we register a short or a long pres depending on the millis when the button was pressed
    if(digitalRead(ENCODER_SW) == HIGH && sensor_button_state_last == LOW){
        if(millis() - sensor_button_down_time < 1000){
            sensor_button_state = 1;
            requireRedraw(true);
        }
        sensor_button_hasBeenCounted = false;
        /*
        else{
        sensor_button_state = 2;
        requireRedraw(true);
    }
    */
    sensor_button_state_last = HIGH;
}

}

//If called the screen will be redrawn on the next cycle
void requireRedraw(bool doesIt){
    ui_redraw_require = doesIt;
}
