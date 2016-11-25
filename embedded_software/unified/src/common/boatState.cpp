/******************************************************************************
 * Hackerboat Beaglebone boat state module
 * boatState.cpp
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include "hackerboatRoot.hpp"
#include "enumtable.hpp"
#include "location.hpp"
#include "gps.hpp"
#include "sqliteStorage.hpp"
#include "hal/config.h"
#include "logs.hpp"
#include "enumdefs.hpp"
#include "healthMonitor.hpp"
#include "waypoint.hpp"
#include "dodge.hpp"
#include "hal/relay.hpp"
#include "hal/gpio.hpp"
#include "hal/adcInput.hpp"
#include "hal/RCinput.hpp"
#include "hal/gpsdInput.hpp"
#include "boatState.hpp"

#define GET_VAR(var) ::parse(json_object_get(input, #var), &var)
#define MAKE_FUNC(func) { #func, std::function<bool(json_t*, BoatState*)>( Command::func ) }

BoatState::BoatState () {
	relays = RelayMap::instance();
	rudder = new Servo();
	rudder->attach(RUDDER_PORT, RUDDER_PIN);
	if (!disarmInput.isInit()) {
		disarmInput.setPort(SYSTEM_DISARM_INPUT_PORT);
		disarmInput.setPin(SYSTEM_DISARM_INPUT_PIN);
		disarmInput.setDir(false);
		disarmInput.init();
	}
	if (!armInput.isInit()) {
		armInput.setPort(SYSTEM_ARM_INPUT_PORT);
		armInput.setPin(SYSTEM_ARM_INPUT_PIN);
		armInput.setDir(false);
		armInput.init();
	}
	if (!servoEnable.isInit()) {
		servoEnable.setPort(SYSTEM_SERVO_ENB_PORT);
		servoEnable.setPin(SYSTEM_SERVO_ENB_PIN);
		servoEnable.setDir(false);
		servoEnable.init();
	}
	K = {PID_KP, PID_KI, PID_KD};
}

bool BoatState::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		if (this->faultCount()) faultString += ":";
		faultString += fault;
	}
	return true;
}

bool BoatState::hasFault (const string fault) const {
	if (faultString.find(fault) != std::string::npos) return true;
	return false;
}

bool BoatState::removeFault (const string fault) {
	size_t index;
	index = faultString.find(fault);
	if (index != std::string::npos) {
		if (index > 0) index--;
		faultString.erase(index, fault.length() + 1);	// captures the leading colon
		return true;
	} else return false;
}

int BoatState::faultCount (void) const {
	if (faultString.size() == 0) return 0;	// if the string is empty, there are no faults
	int cnt = 1;							// if it's not empty, there's at least one fault here
	size_t index = faultString.find(':');	// find the first seperator (if any)
	while (index != std::string::npos) {
		cnt++;
		index = faultString.find(':', (index + 1));
	}
	return cnt;
}

bool BoatState::setBoatMode (std::string mode) {
	BoatModeEnum m;
	if (!boatModeNames.get(mode, &m)) {
		return false;
	}
	_boat = m;
	return true;
}

bool BoatState::setNavMode (std::string mode) {
	NavModeEnum m;
	if (!navModeNames.get(mode, &m)) {
		return false;
	}
	_nav = m;
	return true;
}
						
bool BoatState::setAutoMode (std::string mode) {
	AutoModeEnum m;
	if (!autoModeNames.get(mode, &m)) {
		return false;
	}
	_auto = m;
	return true;
}
						
bool BoatState::setRCmode (std::string mode) {
	RCModeEnum m;
	if (!rcModeNames.get(mode, &m)) {
		return false;
	}
	_rc = m;
	return true;
}

bool BoatState::isValid() {
	bool result = true;
	result &= lastFix.isValid();
	result &= launchPoint.isValid();
	result &= (health && health->isValid());
	return result;
}

json_t* BoatState::pack () const {
	json_t *output = json_object();
	int packResult = 0;
	packResult += json_object_set_new(output, "recordTime", json(HackerboatState::packTime(this->recordTime)));
	packResult += json_object_set_new(output, "currentWaypoint", json_integer(currentWaypoint));
	packResult += json_object_set_new(output, "waypointStrength", json_real(waypointStrength));
	packResult += json_object_set_new(output, "lastContact", json(HackerboatState::packTime(this->lastContact)));
	packResult += json_object_set_new(output, "lastRC", json(HackerboatState::packTime(this->lastRC)));
	packResult += json_object_set_new(output, "lastFix", this->lastFix.pack());
	packResult += json_object_set_new(output, "launchPoint", this->launchPoint.pack());
	//packResult += json_object_set_new(output, "action", json(Waypoints::actionNames.get(action)));
	packResult += json_object_set_new(output, "faultString", json(this->faultString));
	packResult += json_object_set_new(output, "boatMode", json(boatModeNames.get(_boat)));
	packResult += json_object_set_new(output, "navMode", json(navModeNames.get(_nav)));
	packResult += json_object_set_new(output, "autoMode", json(autoModeNames.get(_auto)));
	packResult += json_object_set_new(output, "rcMode", json(rcModeNames.get(_rc)));
	packResult += json_object_set_new(output, "relays", this->relays->pack());
	
	if (packResult != 0) {
		json_decref(output);
		return NULL;
	}
	return output;
}

bool BoatState::parse (json_t* input ) {
	std::string recordTimeIn, lastContactIn, lastRCin, boatMode, navMode, autoMode, rcMode;
	bool result = true;
	
	result &= GET_VAR(currentWaypoint);
	result &= GET_VAR(waypointStrength);
	result &= GET_VAR(faultString);
	result &= GET_VAR(boatMode);
	result &= GET_VAR(navMode);
	result &= GET_VAR(autoMode);
	result &= GET_VAR(rcMode);
	result &= ::parse(json_object_get(input, "recordTime"), &recordTimeIn);
	result &= ::parse(json_object_get(input, "lastContact"), &lastContactIn);
	result &= ::parse(json_object_get(input, "lastRC"), &lastRCin);
	result &= HackerboatState::parseTime(recordTimeIn, this->recordTime);
	result &= HackerboatState::parseTime(lastContactIn, this->lastContact);
	result &= HackerboatState::parseTime(lastRCin, this->lastRC);
	result &= boatModeNames.get(boatMode, &(this->_boat));
	result &= navModeNames.get(navMode, &(this->_nav));
	result &= autoModeNames.get(autoMode, &(this->_auto));
	result &= rcModeNames.get(rcMode, &(this->_rc));
	
	return result;
}

HackerboatStateStorage &BoatState::storage () {
	if (!stateStorage) {
		stateStorage = new HackerboatStateStorage(HackerboatStateStorage::databaseConnection(STATE_DB_FILE), 
							"BOAT_STATE",
							{ { "recordTime", "TEXT" },
							  { "currentWaypoint", "INTEGER" },
							  { "waypointStrength", "REAL" },
							  { "lastContact", "TEXT" },
							  { "lastRC", "TEXT" },
							  { "lastFix", "TEXT" },
							  { "launchPoint", "TEXT" },
//							  { "action", "TEXT" },
							  { "faultString", "TEXT" },
							  { "boatMode", "TEXT" },
							  { "navMode", "TEXT" },
							  { "autoMode", "TEXT" },
							  { "rcMode", "TEXT" },
							  { "relays", "TEXT" } });
		stateStorage->createTable();						 
	}
	return *stateStorage;
}

bool BoatState::fillRow(SQLiteParameterSlice row) const {
	row.assertWidth(14);
	json_t* out;
	
	row.bind(0, HackerboatState::packTime(recordTime));
	row.bind(1, currentWaypoint);
	row.bind(2, waypointStrength);
	row.bind(3, HackerboatState::packTime(lastContact));
	row.bind(4, HackerboatState::packTime(lastRC));
	out = this->lastFix.pack();
	row.bind(5, json_dumps(out,0));
	json_decref(out);
	out  = this->launchPoint.pack();
	row.bind(6, json_dumps(out,0));
	json_decref(out);
//	row.bind(7, Waypoints::actionNames.get(action));
	row.bind(8, faultString);
	row.bind(9, boatModeNames.get(_boat));
	row.bind(10, navModeNames.get(_nav));
	row.bind(11, autoModeNames.get(_auto));
	row.bind(12, rcModeNames.get(_rc));
	out = this->relays->pack();
	row.bind(13, json_dumps(out,0));
	json_decref(out);
	
	return true;
}

bool BoatState::readFromRow(SQLiteRowReference row, sequence seq) {
	bool result = true;
	_sequenceNum = seq;
	row.assertWidth(14);

	std::string str = row.string_field(0);
	result &= HackerboatState::parseTime(str, this->recordTime);
	this->currentWaypoint = row.int_field(1);
	this->waypointStrength = row.double_field(2);
	str = row.string_field(3);
	result &= HackerboatState::parseTime(str, this->lastContact);
	str = row.string_field(4);
	result &= HackerboatState::parseTime(str, this->lastRC);
	str = row.string_field(5);
	result &= this->lastFix.parse(json_loads(str.c_str(), str.size(), NULL));
	str = row.string_field(6);
	result &= this->launchPoint.parse(json_loads(str.c_str(), str.size(), NULL));
	str = row.string_field(7);
//	result &= Waypoints::actionNames.get(str, &(this->action));
	this->faultString = row.string_field(8);
	str = row.string_field(9);
	result &= boatModeNames.get(str, &(this->_boat));
	str = row.string_field(10);
	result &= navModeNames.get(str, &(this->_nav));
	str = row.string_field(11);
	result &= autoModeNames.get(str, &(this->_auto));
	str = row.string_field(12);
	result &= rcModeNames.get(str, &(this->_rc));
	
	return result;
}

void BoatState::pushCmd (std::string name, json_t* args) {
	cmdvec.emplace_back(Command(this, name, args));
}

int BoatState::executeCmds (int num) {
	if (num == 0) num = this->cmdvec.size();		// If we got a zero, eat everything
	int result = 0;
	for (int i = 0; i < num; i++) {					// iterate over the given set of commands
		if (cmdvec.size()) {						// this is here to guard against any modifications elsewhere
			if (cmdvec.front().execute()) result++;	// execute the command at the head of the queue
			cmdvec.pop_front();			// remove the head element
		}
	}
	return result;
}

std::string BoatState::getCSV() {
	std::string csv;
	csv =  HackerboatState::packTime(recordTime);
	csv += ",";
	csv	+= std::to_string(currentWaypoint);
	csv += ",";
	csv += std::to_string(waypointStrength);
	csv += ",";
//	csv	+= HackerboatState::packTime(lastContact);
//	csv += ",";
//	csv += HackerboatState::packTime(lastRC);
//	csv += ",";
	csv += std::to_string(lastFix.fix.lat);
	csv += ",";
	csv += std::to_string(lastFix.fix.lon);
	csv += ",";
	csv += std::to_string(lastFix.track);
	csv += ",";
	csv += std::to_string(lastFix.speed);
	csv += ",";
	csv += std::to_string(lastFix.fixValid);
	csv += ",";
	csv += std::to_string(disarmInput.get());
	csv += ",";
	csv += std::to_string(armInput.get());
	csv += ",";
	csv += servoEnable.get();
	csv += ",";
	csv += std::to_string(throttle->getThrottle());
	csv += ",";
	csv += std::to_string(rudder->readMicroseconds());
	csv += ",";
	csv += std::to_string(orient->getOrientation()->heading);
	csv += ",";
	csv += std::to_string(rc->getThrottle());
	csv += ",";
	csv += std::to_string(rc->getCourse());
	csv += ",";
	csv += rc->isFailSafe();
	csv += ",";
	csv += rcModeNames.get(rc->getMode());
	//csv += ",";
	//csv += adc->getRawValues()["mot_i"];
	//csv += ",";
	//csv += adc->getRawValues()["battery_mon"];
	csv += "\n";
	return csv;
}

std::string BoatState::getCSVheaders() {
	std::string headers;
	headers = "RecordTime,CurrentWaypoint,WaypointStrength,/*LastContactTime,LastRCTime,*/";
	headers += "Lat,Lon,Track,Speed,FixValid,StopButtonState,ArmButtonState,ServoEnableState,";
	headers += "ThrottlePosition,RudderCommand,CurrentHeading,ThrottleInput,CourseInput,RCFailSafe,";
	headers += "RCMode,RawMotorCurrent,RawBatteryVoltage";
	return headers;
}

Command::Command (BoatState *state, std::string cmd, json_t *args) :
	_state(state), _cmd(cmd), _args(args) {
		this->_funcs.at(_cmd);	// force an exception on an invalid command name
	};

bool Command::execute () {
	std::function<bool(json_t*, BoatState*)> cmd = this->_funcs.at(_cmd);
	return cmd(_args, _state);
}
	
bool Command::SetMode(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	std::string modeString;
	if (::parse(json_object_get(args, "mode"), &modeString)) {
		BoatModeEnum newmode;
		if (state->boatModeNames.get(modeString, &newmode)) {
			state->setBoatMode(newmode);
			return true;
		} else return false;
	}  
	return false;
}

bool Command::SetNavMode(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	std::string modeString;
	if (::parse(json_object_get(args, "mode"), &modeString)) {
		NavModeEnum newmode;
		if (state->navModeNames.get(modeString, &newmode)) {
			state->setNavMode(newmode);
			return true;
		} else return false;
	} 
	return false;
}

bool Command::SetAutoMode(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	std::string modeString;
	if (::parse(json_object_get(args, "mode"), &modeString)) {
		AutoModeEnum newmode;
		if (state->autoModeNames.get(modeString, &newmode)) {
			state->setAutoMode(newmode);
			return true;
		} else return false;
	} 
	return false;
}

bool Command::SetHome(json_t* args, BoatState *state) {
	if (!state) return false;
	if ((!args) && state->lastFix.isValid()) {
		state->launchPoint = state->lastFix.fix;
		return true;
	} else {
		Location newhome;
		if ((newhome.parse(json_object_get(args, "location"))) && (newhome.isValid())) {
			state->launchPoint = newhome;
			return true;
		}
	}
	return false;
}

bool Command::ReverseShell(json_t* args, BoatState *state) {
	return false;
}

bool Command::SetWaypoint(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	int number;
	if (::parse(json_object_get(args, "number"), &number)) {
		if (number > state->waypointList.count()) number = state->waypointList.count();
		if (number < 0) number = 0;
		state->waypointList.setCurrent(number);
		return true;
	}
	return false;
}

bool Command::SetWaypointAction(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	std::string modeString;
	if (::parse(json_object_get(args, "action"), &modeString)) {
		WaypointActionEnum action;
		if (state->waypointList.actionNames.get(modeString, &action)) {
			state->waypointList.setAction(action);
			return true;
		}
	}
	return false;
}

bool Command::DumpPathKML(json_t* args, BoatState *state) {
	return false;
}

bool Command::DumpWaypointKML(json_t* args, BoatState *state) {
	return false;
}

bool Command::DumpObstacleKML(json_t* args, BoatState *state) {
	return false;
}

bool Command::DumpAIS(json_t* args, BoatState *state) {
	return false;
}

bool Command::FetchWaypoints(json_t* args, BoatState *state) {
	return false;
}

bool Command::PushPath(json_t* args, BoatState *state) {
	return false;
}

bool Command::SetPID(json_t* args, BoatState *state) {
	if ((!state) || (!args)) return false;
	json_t *input = args;
	double Kp, Ki, Kd;
	bool result = false;
	if (GET_VAR(Kp)) {
		std::get<0>(state->K) = Kp;
		result = true;
	}
	if (GET_VAR(Ki)) {
		std::get<1>(state->K) = Ki;
		result = true;
	}
	if (GET_VAR(Kd)) {
		std::get<2>(state->K) = Kd;
		result = true;
	}
	return result;
}

const map<std::string, std::function<bool(json_t*, BoatState*)>> Command::_funcs = {
	MAKE_FUNC(SetMode),
	MAKE_FUNC(SetNavMode),
	MAKE_FUNC(SetAutoMode),
	MAKE_FUNC(SetHome),
	MAKE_FUNC(ReverseShell),
	MAKE_FUNC(SetWaypoint),
	MAKE_FUNC(SetWaypointAction),
	MAKE_FUNC(DumpPathKML),
	MAKE_FUNC(DumpWaypointKML),
	MAKE_FUNC(DumpObstacleKML),
	MAKE_FUNC(DumpAIS),
	MAKE_FUNC(FetchWaypoints),
	MAKE_FUNC(PushPath),
	MAKE_FUNC(SetPID)
};

const EnumNameTable<BoatModeEnum> BoatState::boatModeNames = {
	"Start",
	"SelfTest",
	"Disarmed",
	"Fault",
	"Navigation",
	"ArmedTest",
	"LowBattery",
	"None"
};

const EnumNameTable<NavModeEnum> BoatState::navModeNames = {
	"Idle",
	"Fault",
	"RC",
	"Autonomous",
	"None"
};

const EnumNameTable<AutoModeEnum> BoatState::autoModeNames = {
	"Idle",
	"Waypoint",
	"Return",
	"Anchor",
	"None"
};

const EnumNameTable<RCModeEnum> BoatState::rcModeNames = {
	"Idle",
	"Rudder",
	"Course",
	"Failsafe",
	"None"
};