//SCL: D1, SDA: D2
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

long ui_blink_time;
bool ui_blink_state = false;

//Draw bootscreen
void drawBoot(String str){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_guildenstern_nbp_tf);
    char buffer[20];
    str.toCharArray(buffer, 20);
    u8g2.drawStr(0, 10, buffer);
    u8g2.sendBuffer();
}

//Draw the time and date on the top
void drawStatusBar(int wifi_strength){
    u8g2.setDrawColor(1);
    RtcDateTime curr = getCurrentTime();
    char date[11];
    char tim[6];

    //Creating the strings
    snprintf(date, sizeof(date), "%02d/%02d/%d", curr.Day(), curr.Month(), curr.Year()-2000);
    snprintf(tim, sizeof(tim), "%02d:%02d", curr.Hour(), curr.Minute());

    u8g2.setFont(u8g2_font_guildenstern_nbp_tf);
    u8g2.drawStr(23,10, tim);
    u8g2.drawStr(60, 10, date);


    u8g2.setDrawColor(0);

    //If there is no connection
    if(wifi_strength == 99){
        u8g2.drawXBM(118, 0, wifi_0_width, wifi_0_height, wifi_0_bits);
    }
    //If the rssi is lower than -80dB
    else if(wifi_strength < -80){
        u8g2.drawXBM(118, 0, wifi_1_width, wifi_1_height, wifi_1_bits);
    }
    //If the rssi is lower than -80dB
    else if(wifi_strength < -70){
        u8g2.drawXBM(118, 0, wifi_2_width, wifi_2_height, wifi_2_bits);
    }
    //If the rssi is lower than -80dB
    else if(wifi_strength < -63){
        u8g2.drawXBM(118, 0, wifi_3_width, wifi_3_height, wifi_3_bits);
    }
    else{
        u8g2.drawXBM(118, 0, wifi_4_width, wifi_4_height, wifi_4_bits);
    }


}

void drawMode(int mode){

    u8g2.setFont(u8g2_font_guildenstern_nbp_tf);
    u8g2.setDrawColor(1);
    switch(mode){
        case 0:
            u8g2.drawStr(23,64, "Mode: Toggle");
            break;
        case 1:
            u8g2.drawStr(23,64, "Mode: Timer");
            break;
        case 2:
            u8g2.drawStr(23,64, "Mode: Temperature");
            break;
    }
}

//Draw the side menu
void drawMenu(byte index, bool is_on){
    u8g2.setDrawColor(0);

    switch(index){
        case 0:
            u8g2.drawXBM(0, 0, menu_0_sel_width, menu_0_sel_height, menu_0_sel_bits);
            if(is_on)
                u8g2.drawXBM(0, 16, menu_1_no_on_width, menu_1_no_on_height, menu_1_no_on_bits);
            else
                u8g2.drawXBM(0, 16, menu_1_no_off_width, menu_1_no_off_height, menu_1_no_off_bits);
            u8g2.drawXBM(0, 32, menu_2_no_width, menu_2_no_height, menu_2_no_bits);
            u8g2.drawXBM(0, 48, menu_3_no_width, menu_3_no_height, menu_3_no_bits);
        break;

        case 1:
            u8g2.drawXBM(0, 0, menu_0_no_width, menu_0_no_height, menu_0_no_bits);
            if(is_on)
                u8g2.drawXBM(0, 16, menu_1_sel_on_width, menu_1_sel_on_height, menu_1_sel_on_bits);
            else
                u8g2.drawXBM(0, 16, menu_1_sel_off_width, menu_1_sel_off_height, menu_1_sel_off_bits);
            u8g2.drawXBM(0, 32, menu_2_no_width, menu_2_no_height, menu_2_no_bits);
            u8g2.drawXBM(0, 48, menu_3_no_width, menu_3_no_height, menu_3_no_bits);
        break;

        case 2:
            u8g2.drawXBM(0, 0, menu_0_no_width, menu_0_no_height, menu_0_no_bits);
            if(is_on)
                u8g2.drawXBM(0, 16, menu_1_no_on_width, menu_1_no_on_height, menu_1_no_on_bits);
            else
                u8g2.drawXBM(0, 16, menu_1_no_off_width, menu_1_no_off_height, menu_1_no_off_bits);
            u8g2.drawXBM(0, 32, menu_2_sel_width, menu_2_sel_height, menu_2_sel_bits);
            u8g2.drawXBM(0, 48, menu_3_no_width, menu_3_no_height, menu_3_no_bits);
        break;

        case 3:
            u8g2.drawXBM(0, 0, menu_0_no_width, menu_0_no_height, menu_0_no_bits);
            if(is_on)
                u8g2.drawXBM(0, 16, menu_1_no_on_width, menu_1_no_on_height, menu_1_no_on_bits);
            else
                u8g2.drawXBM(0, 16, menu_1_no_off_width, menu_1_no_off_height, menu_1_no_off_bits);
            u8g2.drawXBM(0, 32, menu_2_no_width, menu_2_no_height, menu_2_no_bits);
            u8g2.drawXBM(0, 48, menu_3_sel_width, menu_3_sel_height, menu_3_sel_bits);
        break;
    }

    u8g2.setDrawColor(1);
    u8g2.drawVLine(16, 0, 64);
}

//Draw the temp page
void drawTempPage(float sensor_temperature_avg, bool edit){
    //Some conversions to get a string from the float
    String temp_str = String(sensor_temperature_avg);
    char temp[10];
    sprintf(temp, "%s", temp_str.c_str());

    u8g2.setDrawColor(1);
    u8g2.setFontMode(1);

    if(edit){
        if(ui_blink_state){
            if(millis() - ui_blink_time > 800){
                ui_blink_state = !ui_blink_state;
                ui_blink_time = millis();
            }
        }else{
            if(millis() - ui_blink_time > 250){
                ui_blink_state = !ui_blink_state;
                ui_blink_time = millis();
            }
        }
    }


    if(ui_blink_state || !edit){
        u8g2.setFont(u8g2_font_helvR24_tf);
        u8g2.drawUTF8(23,45, temp);
        u8g2.setFont(u8g2_font_10x20_tf);
        u8g2.drawUTF8(108 ,35, "°C");
    }



}
//Draw the on off page
void drawOnOffPage(bool heating_on){
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_helvR24_tf);
    if(heating_on){
        u8g2.drawUTF8(23,45, "ON");
    }else{
        u8g2.drawUTF8(23,45, "OFF");
    }
}

//Draw the timer page
void drawTimerPage(int heating_timer_minutes){
    int minutes = heating_timer_minutes%60;
    int hours = (heating_timer_minutes - minutes)/60;
    char str[5];
    sprintf(str, "%02dh %02dm", hours, minutes);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.drawUTF8(23,45, str);
}

//Draw the info page
void drawInfoPage(){
    u8g2.setFont(u8g2_font_guildenstern_nbp_tf);
    u8g2.drawStr(23,30, "Long press to reset");
    u8g2.drawStr(23, 42, "WiFi credentials");

}

void drawCredentialsPage(){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_guildenstern_nbp_tf);
    u8g2.drawStr(0,10, "Connect to:");
    u8g2.drawStr(0, 22, "WiFi: Thermostat-A03D");
    u8g2.drawStr(0, 34, "Password: password");

    u8g2.drawStr(0, 48, "And go to: ");
    u8g2.drawStr(0, 60, "http://192.168.4.1");
    u8g2.sendBuffer();
}
 void drawQR(){
     u8g2.setDrawColor(0);
     u8g2.clearBuffer();
     u8g2.drawXBM(0, 0, qr_width,qr_height, qr_bits);
     u8g2.sendBuffer();
 }
