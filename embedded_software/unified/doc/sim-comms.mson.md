# SimOutput (object)
This is the object that the simulation shim writes to the output file. 
Each object is output on a single line -- no pretty printing!

## Properties
- ThrottlePosition (number, required)
	This is the throttle position, and it ranges from -5 to 5
- RudderPosition (number, required)
	This is the rudder position, and it ranges from -100 to 100

# SimInput (object)
This is the object that the simulation shim reads from the input file. 
Each object is output on a single line -- no pretty printing!

## Properties
- GPSposition (object, required)
	- lat (number, required)
	- lon (number, required)
- One Of 
	- HeadingTrue (number)
	- HeadingMag (number)
