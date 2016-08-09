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
	<server name>/kml/get
	<server name>/kml/put
	<server name>/kml/git
	<server name>/ais

## <server name\>/boat ##

A valid communication to this endpoint SHALL contain the following fields:

	{BoatName:"<name>",
     BoatKey:<boat key>,
	 Mode:"<boat mode>",
	 NavMode:"<nav mode>",
	 RCmode:"<rc mode>",
	 FaultString:"<faults>",
	 Pose:{location:{latitude:<lat>,
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
	 BatteryData:{MainBatV:<main battery voltage>,
				  MainBatI:<main battery current>,
				  ChargeV:<charge voltage>,
				  ChargeI:<charge current>,
				  MotorV:<motor voltage>,
				  MotorI:<motor current>,
				  BatMon:<battery monitor>},
	 OutputState:{RudderPosn:<rudder position>,
				  RudderEnb:<rudder enable>,
				  MotorRelayOutputs:<motor relay bit string>,
				  HornRelay:<horn relay>,
				  StopRelay:<stop relay>,
				  EnableRelay:<enable relay>},
	 HealthMon:{MotorRelayFault:<motor relay fault bit string>,
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
	 LinkRSSI:<rssi>
	}

The variables are defined as follows:

- **TargetWaypoint:** The target waypoint, counting the coordinate tuples from zero.
- **TargetHeading:** The target heading, in degress clockwise from magnetic north. 
- **BatteryData:** Data on the various main bus voltages and currents.
- **OutputState:** The state of all outputs.
- **HealthMon:** State of the various relay outputs and servo current
- **LinkRSSI:** RSSI of the network link.

The response from the server to the boat SHALL contain the following fields:

	{BoatName:"<name>",
	 ServerKey:<server key>,
	 Command:[{CommandString:<command>,
			   <arg1 name>:<arg1 val>,
						  .
						  .
			 }, . . . .]}

There MAY be any number of commands, and the boat SHALL execute them in the order presented, from first to last. The list of commands and their arguments is as follows:

	
	SetMode(mode)
	SetNavMode(nav mode)
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

## <server name\>/kml/get/<target file\>.kml ##

This endpoint SHALL serv one of the following three [KML](https://developers.google.com/kml/) files:

- waypoints.kml
- obstacle.kml
- path.kml

The waypoints.kml file SHALL contain a single Placemark object that SHALL be named WAYPOINTS and SHALL contain a single LineString object that SHALL contain the waypoints, in order. All references to waypoints SHALL be ordinal numbers referring to this file. 

The obstacle.kml file SHALL contain a polygon for every known obstacle and exclusion zone in the predicted operational area of the vessel.

The path.kml file SHALL contain an individual timestamped Placemark object for every time the boat checks in with the shore server, or as often as directed by compilation variables within the software.

## <server name\>/kml/put ##

This endpoint SHALL accept a third JSON item, named for whichever of the three types of KML file is being uploaded and containing the full text of the KML file. Any path.kml upload SHALL replace the existing path.kml file, assuming the upload is well-formed. Any other upload SHALL be saved to a sandbox for inspection.

## <server name\>/kml/git ##

This is a git server designed to allow the boat to maintain waypoint.kml, obstacle.kml, and path.kml files. 

## <server name\>/ais ##

This endpoint SHALL accept a third JSON item named "AIS" and containing all of the current AIS targets in CSV format. 

## Variables ##

## Commands ##