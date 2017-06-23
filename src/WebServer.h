//Web server to host the credentials page
ESP8266WebServer server(80);

//Sends index page
void homePage(){
  server.send(200, "text/html",  INDEX);
}

//Sends not found page
void notFoundPage(){
  server.send(404, "text/html", "<h1>This page was not found</h1>");
}

void setCredPage(){
    //Obtains the request method
  int requestMethod = server.method();

  //If the request is post the server will save the credentials to the eeprom, then it will send the page
  if(requestMethod == HTTP_POST){
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    writeCredentials(ssid, password);


    server.send(200, "text/html", CREDSET);
    drawBoot("Credentials set!");
    delay(2000);
    ESP.restart();

  }
}

void serverSetup(){
    //A different function will be called for each url. This function sends the page data
    server.on("/", homePage);
    server.on("/setCredentials", HTTP_POST, setCredPage);
    server.onNotFound(notFoundPage);

    //Starts the web server
    server.begin();
}
