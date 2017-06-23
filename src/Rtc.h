RtcDS1307<TwoWire> Rtc(Wire);

RtcDateTime getCurrentTime(){
  return Rtc.GetDateTime();
}

void RtcSetup(){
    Rtc.Begin();

    //Check if the time on the RTC is valid, if not it will set it to the compile time
    RtcDateTime compileTime = RtcDateTime(__DATE__, __TIME__);
    if(!Rtc.IsDateTimeValid()){
        Rtc.SetDateTime(compileTime);
    }
    //Check if the RTC is running, if not start it
    if(!Rtc.GetIsRunning()){
        Rtc.SetIsRunning(true);
    }

    if (getCurrentTime() < compileTime)
    {
        Rtc.SetDateTime(compileTime);
    }
    Rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
}

//Generates and returns the current timestamp
String getTimestamp(){
  //Read the current time and date
  RtcDateTime curr = Rtc.GetDateTime();

  //Generate the correctly formatted string
  char timestamp[30];
  sprintf(timestamp, "%d-%02d-%02d %02d:%02d:%02d", curr.Year(), curr.Month(), curr.Day(), curr.Hour(), curr.Minute(), curr.Second());

  return timestamp;
}
