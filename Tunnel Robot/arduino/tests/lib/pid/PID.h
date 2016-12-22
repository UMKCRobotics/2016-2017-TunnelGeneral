#ifndef PID_H
#define PID_H
#include "Arduino.h"


typedef long double bigfloat;

template<class T> class PID{
 private:
 
  long prev_time;
  long dt; //fuck your continuous math
  bigfloat integral;
  bigfloat derivative;
  T prev_error;
  bigfloat integral_gain;
  bigfloat derivative_gain;
  bigfloat proportional_gain;
  long currentTime;
  long prev_output;
 public:
	void logDiagnosticInfo(){
		Serial.print("output:" );
		Serial.println(prev_output);
		Serial.print("error: ");
		Serial.println(prev_error);
		Serial.print("dt: ");
		Serial.println(dt);
	}

	PID(){
	  setPrevTimeNow();
	  setGains(1,1,1);
	  integral = 0;
	  prev_error = 0;
	}

	PID(bigfloat p_gain, bigfloat i_gain, bigfloat d_gain){
		setPrevTimeNow();
  		setGains(p_gain,i_gain,d_gain);
  		integral = 0;
  		prev_error = 0;
	}

	void setGains(bigfloat p_gain, bigfloat i_gain, bigfloat d_gain){
	  proportional_gain = p_gain;
	  integral_gain = i_gain;
	  derivative_gain = d_gain;

	}

	long getCurrentTime(){
		return millis();
	}

	void setPrevTimeNow(){
	  prev_time = getCurrentTime();
	}

	//Pretty much taken directly from https://en.wikipedia.org/wiki/PID_controller#Pseudocode
	T calculate(T measured_value, T set_point){
	  long new_time = getCurrentTime();
	  dt = new_time - prev_time;
	  if(dt == 0){
		return prev_output; //curse you continuous math
	  }
	  prev_time = new_time;

	  T error = set_point - measured_value;
	  integral = integral + error * dt;
	  derivative = (error - prev_error) / dt;
	  T output = proportional_gain * error + integral_gain * integral + derivative_gain * derivative;
	  prev_error = error;

	  prev_output = output;
	  return output;
	}
};

#endif
