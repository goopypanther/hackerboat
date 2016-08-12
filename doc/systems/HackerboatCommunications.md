# Hackerboat Long Range Communications #

## Motivation ##

The previous communications mechanism for the Hackerboat required the boat to run a server to serve the control interface to the control computer. This is not possible given the limitations of most cellular and satellite communications providers, where all connections are initiated by the end user, in this case the boat. 

There are therefore three roles -- the user, who issues commands to the boat, the boat itself, and the intermediate server, which stores and forwards data in both directions. The server receives commands from the user and relays them to the boat when the boat next connects. It receives data from the boat and presents it to the user the next time the user connects. 

## User Communications ##

The communication with the user must allow the user to send commands and see the data returned from the boat. Specification beyond that is not in the scope of this document. 

## Structure of a Boat Communication ##

The boat SHALL begin each communication by issuing an HTTPS POST request to the appropriate end point. The body of the request SHALL consist of a JSON object containing, at minimum, the following fields:

    {BoatName:"<name\>", 
	 BoatKey:<id\>}

Where BoatName SHALL be a unique identifier for the boat and BoatKey SHALL be a 256-bit secret key generated and stored by the server. Unless the BoatName and BoatKey match corresponding values stored in a single record by the server, the server MUST NOT respond to the request. 

If the BoatName and BoatKey match, the server SHALL respond to the command as determined by the contents of the body of the POST and the endpoint to which it is addressed. 

The server SHALL provide the following endpoints:

	<server name>/boat
	<server name>/kml
	<server name>/ais

## <server name\>/boat ##

A valid communication to this endpoint SHALL contain the following fields:

	{BoatName:"<name>",
     BoatKey:<boat key>,
	 Mode:"<boat mode>",
	 NavMode:"<nav mode>",
	 RCmode:"<rc mode>",
	 FaultString:"<faults>",
	 Pose:{timestamp:<decimal seconds>,
		   location:{latitude:<lat>,
					 longitude:<long>},
		   orientation:{heading:<heading>,
						pitch:<pitch>,
						roll:<roll>},
		   GPSheading:<true heading>,
		   GroundSpeed:<GPS speed>
		  },
	}

The variables are defined as follows:

- **BoatName:** The name of the boat, type std::string
- **BoatKey:** A 256-bit secret key shared by the boat and the server.
- **Mode:** The current mode of the boat.
- **NavMode:** The current mode of the navigation subsystem.
- **RCmode:** The current mode of the RC subsystem.
- **FaultString:** A string containing all the current faults.
- **Pose:** The current location and orientation of the boat. 
	- **timestamp** The time at which this data was taken. 
	- **location:** The current position of the boat.
		- **latitude:** The current latitude of the boat, in decimal degrees north.
		- **longitude:** The current longitude of the boat, in decimal degrees east.
	- **orientation:** 
		- **heading:** The current heading of the boat, in degrees clockwise from magnetic north.
		- **pitch:** The current pitch angle of the boat, in degrees from horizontal.
		- **roll:** The current roll angle of the boat, in degrees from horizontal.
	- **GPSheading:** The current course made good, in degrees clockwise from true north.
	- **GroundSpeed:** The speed over the ground, in knots.

The communication MAY also contain the following fields:

	{TargetWaypoint:<waypoint number>,
	 TargetHeading:<target heading>,
	 BatteryData:{timestamp:<decimal seconds>,
		   		  MainBatV:<main battery voltage>,
				  MainBatI:<main battery current>,
				  ChargeV:<charge voltage>,
				  ChargeI:<charge current>,
				  MotorV:<motor voltage>,
				  MotorI:<motor current>,
				  BatMon:<battery monitor>},
	 OutputState:{timestamp:<decimal seconds>,
		   		  RudderPosn:<rudder position>,
				  RudderEnb:<rudder enable>,
				  MotorRelayOutputs:<motor relay bit string>,
				  HornRelay:<horn relay>,
				  StopRelay:<stop relay>,
				  EnbRelay:<enable relay>},
	 HealthMon:{timestamp:<decimal seconds>,
		   		MotorRelayFault:<motor relay fault bit string>,
				HornRelayFault:<horn relay fault>,
				StopRelayFault:<stop relay fault>,
				EnbRelayFault:<enable relay fault>,
				MotorRelay0I:<motor relay 0 current>,
				MotorRelay1I:<motor relay 1 current>,
				MotorRelay2I:<motor relay 2 current>,
				MotorRelay3I:<motor relay 3 current>,
				MotorRelay4I:<motor relay 4 current>,
				MotorRelay5I:<motor relay 5 current>,
				HornRelayI:<horn relay current>,
				StopRelayI:<stop relay current>,
				EnbRelayI:<enable relay current>,
				ServoI:<servo power current>},

	 LinkRSSI:<rssi>,
	 RudderPID:{Kp:<Kp>,Ki:<Ki>,Kd:<Kd>} 
	}

The variables are defined as follows:

- **TargetWaypoint:** The target waypoint, counting the coordinate tuples from zero.
- **TargetHeading:** The target heading, in degress clockwise from magnetic north. 
- **BatteryData:** Data on the various main bus voltages and currents.
	- **timestamp** The time at which this data was taken. 
- **OutputState:** The state of all outputs.
	- **timestamp** The time at which this data was taken. 
	- **RudderPosn:** Rudder position, in microseconds.
	- **RudderEnb:** Rudder enable signal, boolean.
	- **MotorRelayOutpus:** A bit string representing the state of all motor relay outputs.
	- **HornRelay:**
	- **StopRelay:**
	- **EnbRelay:**
- **HealthMon:** State of the various relay outputs and servo current
	- **timestamp** The time at which this data was taken. 
	- **MotorRelayFault:** A bit string representing the faults for each of the motor relay channels. 1 is a fault, 0 is no fault.
	- **HornRelayFault:** This is a boolean; true is faulted.
	- **StopRelayFault:** This is a boolean; true is faulted.
	- **EnbRelayFault:** This is a boolean; true is faulted.
	- **MotorRelay0I:** The current flowing through motor relay 0, in amps.
	- **MotorRelay1I:** The current flowing through motor relay 1, in amps.
	- **MotorRelay2I:** The current flowing through motor relay 2, in amps.
	- **MotorRelay3I:** The current flowing through motor relay 3, in amps.
	- **MotorRelay4I:** The current flowing through motor relay 4, in amps.
	- **MotorRelay5I:** The current flowing through motor relay 5, in amps.
- **LinkRSSI:** RSSI of the network link.
- **RudderPID:** PID constants for rudder control

The response from the server to the boat SHALL contain the following fields:

	{BoatName:"<name>",
	 ServerKey:<server key>,
	 Command:[{Command:<command>,
			   Argument:{<arg object>}},
						  .
						  .
			  . . . . .]}

There MAY be any number of commands, and the boat SHALL execute them in the order presented, from first to last. The list of commands and their arguments is as follows:

	SetMode(mode)
	SetNavMode(nav mode)
	SetAutoMode(auto mode)
	SetHome(optional: location)
	ReverseShell(optional: endpoint)
	SetWaypoint(number)
	SetWaypointAction(action)
	DumpPathKML
	DumpWaypointKML
	DumpObstacleKML
	DumpAIS
	FetchWaypoints(optional: repository)
	PushPath(optional: repository)
	GetPID
	SetPID(Kp, Ki, Kd)

See Commands section, below, for details. 

## <server name\>/kml/<target file\>.kml ##

This endpoint SHALL serve one of the following three [KML](https://developers.google.com/kml/) files:

- waypoints.kml
- obstacle.kml
- path.kml

The waypoints.kml file SHALL contain a single Placemark object that SHALL be named WAYPOINTS and SHALL contain a single LineString object that SHALL contain the waypoints, in order. All references to waypoints SHALL be ordinal numbers referring to this file. 

The obstacle.kml file SHALL contain a polygon for every known obstacle and exclusion zone in the predicted operational area of the vessel.

The path.kml file SHALL contain an individual timestamped Placemark object for every time the boat checks in with the shore server, or as often as directed by compilation variables within the software.

This endpoint SHALL also accept file uploads of these three files with a POST request containing a JSON object of the following format:

	{BoatName:"<name>",
     BoatKey:<boat key>, 
	 <file name>:<file contents>}

Upon receiving such a request, the server SHALL add a timestamp to the filename and store it for inspection.

## <server name\>/ais ##

This endpoint SHALL accept a JSON object of the following format, and store it locally with an attached timestamp.

	{BoatName:"<name>",
     BoatKey:<boat key>, 
	 timestamp:<decimal seconds>,
	 location:{latitude:<lat>,
			   longitude:<long>}
	 AIS:<AIS data>}

Where <AIS data\> is the current database of AIS targets in an array of JSON objects. 

## Commands ##

### SetMode(mode) ###

Set the boat mode to mode. Format of argument SHALL be {mode:"<mode name\>"}.

### SetNavMode(nav_mode) ###

Set the nav system mode to one of:

	IDLE
	AUTONOMOUS

Format of argument shall be {mode:"<mode name\>"}

### SetAutoMode(auto_mode) ###

Set the nav system autonomous mode to one of:

	ANCHOR
	RTLS
	WAYPOINT

Format of argument shall be {mode:"<mode name\>"}

### SetHome(optional: location) ###

Set the boat's home point to either its current location or the optional location argument. Format of argument, if present, SHALL be {home:{latitude:<lat\>,longitude:<long\>}}.

### ReverseShell(optional: endpoint) ###

Open a reverse shell to the given endpoint. If present, the argument SHALL be of the form {endpoint:"<url\>"}.

### SetWaypoint(number) ###

Set the target waypoint to the ordinal number. Argument format SHALL be {waypoint:<number\>}.

### SetWaypointAction(action) ###

Set the action at the end of the waypoint list to one of:

	IDLE
	ANCHOR
	RETURN
	REPEAT

Argument format SHALL be {action:"<action name\>"}

### DumpPathKML(optional: endpoint) ###



### DumpWaypointKML(optional: endpoint) ###
### DumpObstacleKML(optional: endpoint) ###
### DumpAIS(optional: endpoint) ###
### PullWaypoints(optional: repository, key) ###

Fetch waypoints.kml from github using internally stored repository and API key. A new respository and key MAY be specified with the argument format {repository:"<url\>",key:<API key\>.

### PushPath(optional: repository, key) ###

Push current path.kml to github using internally stored repository and API key. A new respository and key MAY be specified with the argument format {repository:"<url\>",key:<API key\>}.

### GetPID ###

Get the current steering PID constant values.

### SetPID(Kp, Ki, Kd) ###

Set the current steering PID constants. Argument format SHALL be {Kp:<kp\>,Ki:<ki\>,Kd:<kd\>}. Omitted values SHALL be left unchanged. 