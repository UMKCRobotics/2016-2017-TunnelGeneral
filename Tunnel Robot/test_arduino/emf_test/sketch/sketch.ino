#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif
//test LED pins
#define LED1 22
#define LED2 23
//EMF PINS
#define EMF1 A0

void setup() {
  //initialize led pins
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  //start serial
  Serial.begin(115200);
  //start serial1 to motor controller
  //Serial1.begin(115200);
  //send READY byte
  Serial.write('1');
  
}

void loop() {
  getEMFreading(EMF1);
  Serial.write('\n');
}

// command interpreter
void loopcommand() { 

  command = "";
  value = "";
  int addTo = 0; //0 for command, 1 for value
  //ButtonStates();
  if(Serial.available()){
    while (Serial.available() > 0)
    {
      char readIn = (char)Serial.read();
      if (readIn == '\n') {
        break;
      }
      else if (readIn == '|') {
        addTo = 1;
        continue;
      }
      //other stuff that is important
      if (addTo == 0) {
        command += readIn;
      }
      else if (addTo == 1) {
        value += readIn;
      }
    }
    //clear anything remaining in serial
    while (Serial.available() > 0) {
      Serial.read();
    }
    response = interpretCommand(command,value);
    Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);

}


String interpretCommand(String command, String value) {
  
  String responseString = "n";  // what this function returns
  String returnString = "";     // holds the return value of the command function
  if (command == "e") {
    responseString = "1";
    responseString += String(getEMFReading());
  }
  else if (command == "S") {
    if (value == "E") {
      responseString = "1";
      responseString += String(getEMFReading());
    }
  }

  //check if any BADs were obtained
  if (returnString == "BAD") {
    return "n";
  }
  return responseString;

}
//START OF SENSOR STUFF
void sensorReport(String name, int number) {
  char tbs[16];

  sprintf(tbs, " %4d ", number);

  Serial.write(' ');
  Serial.print(name);
  // Serial.write(' ');
  Serial.print(tbs);
  // Serial.write(' ');
}

int getIRReading(int whichPin) {
  // average of lots of readings
  const long sampleCount = 200;
  long total = 0;
  for (int i = sampleCount; i > 0; --i) {
    total += analogRead(whichPin);
  }

  return total / sampleCount;
}

String getEMFReading() {
  int emf1 = getIRReading(EMF1);

  String report = "";
  //report format: right,front,left,back
  int threshold = 190; //set this to something reasonable
  //check right
  sensorReport("emf1", r1);
  return report;
}

int getEMFReading(int port) {
  int count = 0;
  long int sum = 0;
  for (int i = 0; i < 500; i++) {
    int reading = analogRead(port);
    if (reading != 0) {
      count++;
      //sum += reading*reading;
      sum += pow(reading,2);
      //sum += abs(reading);
    }
  }
  long int average = 0;
  if (count > 0)
    average = sum/count;
  return average;
}
//END OF SENSOR STUFF
