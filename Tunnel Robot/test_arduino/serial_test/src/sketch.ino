#define LED1 8
#define LED2 9
#define LED3 10

//parsing inputs
String command; //used to store command from serial
String value; //used to store value from serial
String previousValue;
String response; //used to store response to main program


void setup() {
  // put your setup code here, to run once:
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  Serial.begin(115200);
  Serial.write(1);
  previousValue = "";
}

void loop() {
  // put your main code here, to run repeatedly:
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
    Serial.println(response+previousValue); //sends response with \n at the end
    previousValue = value;
  }
  //small delay
  delay(20);
}

String interpretCommand(String command, String value) {
  
  String responseString = "n";
  String returnString = "";

  //check if motor stuff
  if (command == "L") {
    returnString = getLED(value);
    responseString = "1";
    responseString += returnString;
  }

  return responseString;

}

String getLED(String value) {
  if (value == "Y") {
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
  }
  else if (value == "B") {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,HIGH);
    digitalWrite(LED3,LOW);
  }
  else if (value == "G") {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,HIGH);
  }
  else if (value == "N") {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
  }
  else {
    return "BAd"+value;
  }
  return "1ABCDEFGH"+value;
}
