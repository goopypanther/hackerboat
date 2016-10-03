/******************************************************************************
 * Hackerboat Beaglebone PID module
 * pid.cpp
 * This module provides a PID type
 * It is copied from br3ttb's Arduino PID library 
 * https://github.com/br3ttb/Arduino-PID-Library
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Mar 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <stdlib.h>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <vector>
#include <tuple>
#include "pid.hpp" 

using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;

/*Constructor (...)*********************************************************
 *    The parameters specified here are those for for which we can't set up 
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
PID::PID(double* Input, double* Output, double* Setpoint,
        double Kp, double Ki, double Kd, int ControllerDirection) :
		myOutput(Output), myInput(Input), mySetpoint(Setpoint),
		inAuto(false), SampleTime(100), controllerDirection(ControllerDirection)
{
    PID::SetTunings(Kp, Ki, Kd);
	lastTime = PID_CLOCK::now();
	lastTime = lastTime - SampleTime;				
}
 
 
/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done. If the loop is not in automatic mode, this will
 * 	 execute every time it is called. 
 **********************************************************************************/ 
bool PID::Compute()
{
   time_point<PID_CLOCK> thisTime = PID_CLOCK::now();
   if (((thisTime - lastTime) >= SampleTime) || !inAuto)
   {
      /*Compute all the working error variables*/
	  double input = *myInput;
      double error = *mySetpoint - input;
      ITerm+= (ki * error);
      if(ITerm > outMax) ITerm= outMax;
      else if(ITerm < outMin) ITerm= outMin;
      double dInput = (input - lastInput);
 
      /*Compute PID Output*/
      double output = kp * error + ITerm- kd * dInput;
      
	  if(output > outMax) output = outMax;
      else if(output < outMin) output = outMin;
	  *myOutput = output;
	  
      /*Remember some variables for next time*/
      lastInput = input;
      lastTime = thisTime;
	  return true;
   }
   else return false;
}


/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted. 
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/ 
void PID::SetTunings(tuple<double, double, double> K) {
	SetTunings (get<0>(K), get<1>(K), get<2>(K));
}

void PID::SetTunings(double Kp, double Ki, double Kd)
{
   if (Kp<0 || Ki<0 || Kd<0) return;
   if (!isfinite(Kp) || !isfinite(Ki) || !isfinite(Kd)) return;
 
   dispKp = Kp; dispKi = Ki; dispKd = Kd;
   
   double SampleTimeInSec = (SampleTime.count())/1000.0;
   kp = Kp;
   ki = Ki * SampleTimeInSec;
   kd = Kd / SampleTimeInSec;
 
  if(controllerDirection == REVERSE)
   {
      kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }
}
  
/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed	
 ******************************************************************************/
void PID::SetSampleTime(long int NewSampleTime)
{
	if (NewSampleTime > 0) {
		double ratio = NewSampleTime/(double)(SampleTime.count());
		ki *= ratio;
		kd /= ratio;
		SampleTime = milliseconds(NewSampleTime);
	}
}
 
/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void PID::SetOutputLimits(double Min, double Max)
{
   if(Min >= Max) return;
   outMin = Min;
   outMax = Max;
 
   if(inAuto)
   {
	   if(*myOutput > outMax) *myOutput = outMax;
	   else if(*myOutput < outMin) *myOutput = outMin;
	 
	   if(ITerm > outMax) ITerm= outMax;
	   else if(ITerm < outMin) ITerm= outMin;
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs or vice versa, the controller is
 * automatically initialized
 ******************************************************************************/ 
void PID::SetMode(int Mode)
{
    bool newAuto = (Mode == AUTOMATIC);
    if(newAuto != inAuto)
    {  /*we just changed modes */
        PID::Initialize();
    }
    inAuto = newAuto;
}
 
/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/ 
void PID::Initialize()
{
   ITerm = *myOutput;
   lastInput = *myInput;
   if(ITerm > outMax) ITerm = outMax;
   else if(ITerm < outMin) ITerm = outMin;
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads 
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void PID::SetControllerDirection(int Direction)
{
   if(Direction != controllerDirection)
   {
	  kp = (0 - kp);
      ki = (0 - ki);
      kd = (0 - kd);
   }   
   controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display 
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
double PID::GetKp(){ return  dispKp; }
double PID::GetKi(){ return  dispKi;}
double PID::GetKd(){ return  dispKd;}
int PID::GetMode(){ return  inAuto ? AUTOMATIC : MANUAL;}
int PID::GetDirection(){ return controllerDirection;}
vector<double> PID::getRealK() {
	vector<double> k;
	k.push_back(kp);
	k.push_back(ki);
	k.push_back(kd);
	return k;
}