//import things here

String command = "";
String value = "";
String response = "";

void setup() {
 //start serial
  Serial.begin(115200);
  Serial.write('1');
}

void loop() { 
  command = "";
  value = "";
  int addTo = 0; //0 for command, 1 for value
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
    response = interpretCommand(command,value);
    Serial.println(response); //sends response with \n at the end
  }
  //small delay
  delay(20);
}

String interpretCommand(String command, String value) {
	//do motor things here
	return "1";
}