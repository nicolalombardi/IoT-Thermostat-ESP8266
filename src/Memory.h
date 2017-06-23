char ssid[32];
char password[64];

//Returns whether the credentials are stored or not
bool areCredentialsStored(){
    EEPROM.begin(512);

    //The first byte determines wether the credentials are stored (1) or not (0)
    return EEPROM.read(0) == 1;
}

//Writes the credentials to the eeprom given the two strings
void writeCredentials(String ssid, String password){
    EEPROM.begin(512);

    //Create two char array to store the strings
    char s_c[32];
    char p_c[64];

    //Copy the strings inside the arrays
    ssid.toCharArray(s_c, 32);
    password.toCharArray(p_c, 64);

    //Write 1 to the first byte to indicate that there are stored credentials
    EEPROM.write(0, 1);
    //Write the ssid length to the third byte and the password length to the fourth
    EEPROM.write(2, ssid.length());
    EEPROM.write(3, password.length());



    EEPROM.put(4, s_c);
    EEPROM.put((4 + 32), p_c);
    EEPROM.commit();
}

void readCredentials(){
    //EEPROM.begin(512);

    int ssid_size = EEPROM.read(2);

    int password_size = EEPROM.read(3);


    for(int i = 0; i < ssid_size; i++){
        ssid[i] = EEPROM.read(i + 4); // i+3 because of the ssid offset
        //Serial.println(ssid[i]);
    }


    for(int i = 0; i < password_size; i++){
        password[i] = EEPROM.read(i + 4 + 32); //offset again
        //Serial.print(password[i]);
    }
    //Serial.println(password);
}

void clearEEPROM(){
    EEPROM.begin(512);
    // write a 0 to all 512 bytes of the EEPROM
    for (int i = 0; i < 512; i++){
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

char* getSSID(){
    return ssid;
}

char* getPassword(){
    return password;
}
