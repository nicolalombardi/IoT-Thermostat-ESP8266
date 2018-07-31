HTTPClient http;

struct command{
    bool valid;
    int ID;
    const char* type;
    float value;
};


//Raccoglie le misurazioni e invia i dati al server tramite ua richiesta POST
void sendDataToServer(float sensor_temperature_avg, int is_on){
  Serial.println("Sending data");

  //Crea l'oggetto che conterrà la stringa JSON
  StaticJsonBuffer<300> JSONBuffer;
  JsonObject& JSONEncoder = JSONBuffer.createObject();

  //Inserisce i valori registrati come coppia chiave-valore
  JSONEncoder["timestamp"] = getTimestamp();
  JSONEncoder["temperature"] = sensor_temperature_avg;
  JSONEncoder["isOn"] = is_on;

  //Crea un array di caratteri che conterrà la stringa.
  char body[JSONEncoder.measureLength()+1];

  //"Stampa" la stringa nell'array
  JSONEncoder.printTo(body, sizeof(body));

  //Imposta l'url della richiesta
  http.begin("http://thermostat.nlombardi.com/api/measurements");

  //Invia la richiesta di tipo POST con la string JSON
  int httpCode = http.POST(String(body));

  //Se il codice della risposta è 200 allora l'invio è andato a buon fine
  if(httpCode == HTTP_CODE_OK){
    Serial.println(http.getString());
  }
  http.end();
}

struct command getCommandFromServer(){
    Serial.println("Getting command");

    //Sets the connection url
    http.begin("http://thermostat.nlombardi.com/api/controls");

    //Makes the GET (rekt) request
    int httpCode = http.GET();

    //If the HTTP ersponse code is 200 then return the parsed json array
    if(httpCode == HTTP_CODE_OK){

        char response[100];
        http.getString().toCharArray(response, 100);

        StaticJsonBuffer<200> JSONBuffer;

        JsonArray& res = JSONBuffer.parseArray(response);
        if(res.success() && res.size() > 0){
            struct command c = {true, res[0]["ID"], res[0]["type"], res[0]["value"]};

            return c;
        }else{
            struct command c = {false, 0, "", 0.0f};
            return c;
        }

    }else{
        struct command c = {false, 0, "", 0.0f};
        return c;
    }




}
