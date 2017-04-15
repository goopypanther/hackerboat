# Ladon Sim Configuration (object)
This is the top level configuration for the configuration of the Project 
Ladon simulator shim.

## Properties
- Version (object, required)
	This gives the version of the sim configuration this is written in.
	- Major (number, required)
	- Minor (number, required)
	- Patch (number)
- SimType (enum, required)
	This defines whether the sim is run internally or connects to an 
	external sim runner. 
	- internal (string)
	- external (string)
- InputFile (string)
	The name of the input file to use in external mode. Default is stdin.
- OutputFile (string)
	The name of the output file to use in internal mode. Default is stdout.
- StartPoint (object)
	The GPS location to start the sim at. Used only in internal mode.
	- lat (number, required)
	- lon (number, required)
- CompassError (number)
	This is added to the calculated heading before it is given to the 
	rest of the code. 
- ThrottlePolynomial (number, array)
	Polynomial to map a throttle input into a speed in m/s. Used only for
	internal mode. 
- RudderPolynomial (number, array)
	Polynomial to map a rudder input to a rate of turn in deg/s. Used only
	for internal mode.  
	
