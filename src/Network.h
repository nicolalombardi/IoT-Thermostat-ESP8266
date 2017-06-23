//HTTP client to make requests
HTTPClient http;

//Acquires all the measurements and sends them to the server using a POST request
void sendDataToServer(float sensor_temperature_avg, int is_on){
  Serial.println("Sending data");

  //Code to initialize the json library
  StaticJsonBuffer<300> JSONBuffer;
  JsonObject& JSONEncoder = JSONBuffer.createObject();

  //Adds the values taken from the respective functions
  JSONEncoder["timestamp"] = getTimestamp();
  JSONEncoder["temperature"] = sensor_temperature_avg;
  JSONEncoder["isOn"] = is_on;

  //Creates a buffer to hold the encoded JSON. The "+1" is there because otherwise the library would cut off the final "{" bracket
  char body[JSONEncoder.measureLength()+1];
  //Prints the JSON to the buffer
  JSONEncoder.printTo(Serial);
  JSONEncoder.printTo(body, sizeof(body));

  //Starts the http connection
  http.begin("http://tesinaiot.altervista.org/api/measurements");
  //Sends the request with the prepared body
  int httpCode = http.POST(String(body));

  //Server responded correctly
  if(httpCode == HTTP_CODE_OK){
    Serial.println(http.getString());
  }
  //Something went wrong TODO:save the data in some sort of buffer and try to send it again some other time
  else{
    //Right now it does nothing
  }
  http.end();
}
