#define EMF1 A1
#define EMF2 A3
//int EMF3 = A3;


String command; //used to store command from serial
String response; //used to store response to main program

void setup() {
  // put your setup code here, to run once:
  // set up all sensors here
  pinMode(EMF1,INPUT);
  //pinMode(EMF3,INPUT);
  Serial.begin(115200);
  Serial.write('1');
}

void loop() {
  command = "";
  if(Serial.available()){
    while (Serial.available() > 0)
    {
      char readIn = (char)Serial.read();
      if (readIn == '\n') {
        break;
      }
      command += readIn;
    }
    response = interpretCommand(command);
    Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);
}

//do whatever is required
String interpretCommand(String cmd) {
  char cmd_char = cmd[0];
  String resp = "";
  if (cmd_char != null) {
    resp = "emf=";
    resp += String(getEMFreading(EMF1));
  }
  else {
    resp = "n";
  }
  return resp;
}


int getEMFreading(int port) {
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

