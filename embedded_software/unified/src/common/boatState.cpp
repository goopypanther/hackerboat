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
#include "easylogging++.h"
#include "util.hpp"

using namespace std;
using namespace rapidjson;

#define MAKE_FUNC(func) { #func, std::function<bool(Value&, BoatState*)>( Command::func ) }

BoatState::BoatState () {
	VLOG(2) << "Creating new BoatState object";
	relays = RelayMap::instance();
	rudder = new Servo();
	rudder->attach(Conf::get()->rudderPort(), Conf::get()->rudderPin());
	if (!disarmInput.isInit()) {
		disarmInput.setPort(Conf::get()->disarmInputPort());
		disarmInput.setPin(Conf::get()->disarmInputPin());
		disarmInput.init();
		disarmInput.setDir(false);
	}
	if (!armInput.isInit()) {
		armInput.setPort(Conf::get()->armInputPort());
		armInput.setPin(Conf::get()->armInputPin());
		armInput.init();
		armInput.setDir(false);
	}
	if (!servoEnable.isInit()) {
		servoEnable.setPort(Conf::get()->servoEnbPort());
		servoEnable.setPin(Conf::get()->servoEnbPin());
		servoEnable.init();
		servoEnable.setDir(false);
	}
	K = {Conf::get()->pidKp(), Conf::get()->pidKi(), Conf::get()->pidKd()};
}

bool BoatState::insertFault (const string fault) {
	if (!this->hasFault(fault)) {
		if (this->faultCount()) faultString += ":";
		faultString += fault;
		LOG(INFO) << "Inserting fault " << fault;
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
		LOG(INFO) << "Removing fault " << fault;
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
		LOG(ERROR) << "Boat mode [" << mode << "] does not exist";
		return false;
	}
	_boat = m;
	return true;
}

bool BoatState::setNavMode (std::string mode) {
	NavModeEnum m;
	if (!navModeNames.get(mode, &m)) {
		LOG(ERROR) << "Navigation mode [" << mode << "] does not exist";
		return false;
	}
	_nav = m;
	return true;
}

bool BoatState::setAutoMode (std::string mode) {
	AutoModeEnum m;
	if (!autoModeNames.get(mode, &m)) {
		LOG(ERROR) << "Autonomous mode [" << mode << "] does not exist";
		return false;
	}
	_auto = m;
	return true;
}

bool BoatState::setRCmode (std::string mode) {
	RCModeEnum m;
	if (!rcModeNames.get(mode, &m)) {
		LOG(ERROR) << "RC mode [" << mode << "] does not exist";
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

Value BoatState::pack () const {
	Value d;
	int p = 0;

	p += PutVar("recordTime", HackerboatState::packTime(this->recordTime), d);
	p += PutVar("lastContact", HackerboatState::packTime(this->lastContact), d);
	p += PutVar("lastRC", HackerboatState::packTime(this->lastRC), d);
	p += PutVar("lastFix", this->lastFix.pack(), d);
	p += PutVar("launchPoint", this->launchPoint.pack(), d);
	p += PutVar("faultString", this->faultString, d);
	p += PutVar("boatMode", boatModeNames.get(this->_boat), d);
	p += PutVar("navMode", navModeNames.get(this->_nav), d);
	p += PutVar("autoMode", autoModeNames.get(this->_auto), d);
	p += PutVar("rcMode", rcModeNames.get(this->_rc), d);
	p += PutVar("disarmInput", this->disarmInput.getState(), d);
	p += PutVar("armInput", this->armInput.getState(), d);
	p += PutVar("servoEnable", this->servoEnable.getState(), d);
	try {
		if (throttle) p += PutVar("throttlePosition", throttle->getThrottle(), d);
	} catch (...) {};
	try {
		if (rudder) p += PutVar("rudderPosition", rudder->read(), d);
	} catch (...) {};
	try {
		if (relays) p += PutVar("relays", this->relays->pack(), d);
	} catch (...) {};
	if (commandCnt()) {
		Value cmdarray(kArrayType);
		for (auto &x : cmdvec) {
			cmdarray.PushBack(x->pack(), root.GetAllocator());
		}
		p += PutVar("commands", cmdarray, d);
	} else {
		p += PutVar("commands", d);
	}
	

	return d;
}

bool BoatState::parse (Value& d) {
	string time, mode;
	Value tmp;
	bool result = true;

	result &= GetVar("faultString", faultString, d);
	result &= GetVar("boatMode", mode, d);
	result &= boatModeNames.get(mode, &(this->_boat));
	result &= GetVar("navMode", mode, d);
	result &= navModeNames.get(mode, &(this->_nav));
	result &= GetVar("autoMode", mode, d);
	result &= autoModeNames.get(mode, &(this->_auto));
	result &= GetVar("rcMode", mode, d);
	result &= rcModeNames.get(mode, &(this->_rc));
	result &= GetVar("recordTime", time, d);
	result &= HackerboatState::parseTime(time, this->recordTime);
	result &= GetVar("lastContact", time, d);
	result &= HackerboatState::parseTime(time, this->lastContact);
	result &= GetVar("lastRC", time, d);
	result &= HackerboatState::parseTime(time, this->lastRC);
	result &= GetVar("lastFix", tmp, d);
	result &= this->lastFix.parse(tmp);
	result &= GetVar("launchPoint", tmp, d);
	result &= this->launchPoint.parse(tmp);

	LOG_IF(!result, ERROR) << "Parsing BoatState input failed: " << d;

	return result;
}

void BoatState::pushCmd (std::string name, const Value& args) {
	try {
		cmdvec.emplace_back(new Command(this, name, args));
	} catch (...) {
		LOG(ERROR) << "Attempted to push invalid command " << name << " with arguments " << args;
	}
	LOG(DEBUG) << "Emplacing command [" << name << "] with arguments: " << args;
}

int BoatState::executeCmds (int num) {
	if (num == 0) num = this->cmdvec.size();		// If we got a zero, eat everything
	int result = 0;
	for (int i = 0; i < num; i++) {					// iterate over the given set of commands
		if (cmdvec.size()) {						// this is here to guard against any modifications elsewhere
			LOG(INFO) << "Executing command " << *(cmdvec.front());
			try {
				if (cmdvec.front()->execute()) result++;	// execute the command at the head of the queue
			} catch (...) {
				LOG(ERROR) << "Attempted to execute invalid command";
				cmdvec.pop_front();			// remove the head element
				break;
			}
			LOG(DEBUG) << "Result: " << result;
			cmdvec.pop_front();			// remove the head element
		}
	}
	return result;
}
		
std::string BoatState::printCurrentWaypointNum() {
	if (this->_nav == NavModeEnum::AUTONOMOUS) {
		switch (this->_auto) {
			case AutoModeEnum::WAYPOINT:
				return std::to_string(this->waypointList.current());
				break;
			case AutoModeEnum::ANCHOR:
				return "ANCHOR";
				break;
			case AutoModeEnum::RETURN:
				return "RETURN";
				break;
			default:
				return "NONE";
		}
	} else return "NONE";
}

Location BoatState::getCurrentTarget()  {
	if (this->_nav == NavModeEnum::AUTONOMOUS) {
		switch (this->_auto) {
			case AutoModeEnum::WAYPOINT:
				return this->waypointList.getWaypoint();
				break;
			case AutoModeEnum::ANCHOR:
				return this->anchorPoint;
				break;
			case AutoModeEnum::RETURN:
				return this->launchPoint;
				break;
			default:
				return Location();
		}
	} else return Location();
}								/**< Returns the current target location, or an invalid Location if there isn't one right now */

std::string BoatState::getCSV() {
	std::string csv;
	csv =  HackerboatState::packTime(recordTime);
	csv += ",";
	csv += std::to_string(lastFix.fix.lat);
	csv += ",";
	csv += std::to_string(lastFix.fix.lon);
	csv += ",";
	csv += std::to_string(lastFix.track);
	csv += ",";
	csv += std::to_string(lastFix.speed);
	csv += ",";
	csv += GPSFix::NMEAModeNames.get(lastFix.mode);
	csv += ",";
	csv	+= this->printCurrentWaypointNum();
	csv += ",";
	csv += std::to_string(this->getCurrentTarget().lat);
	csv += ",";
	csv += std::to_string(this->getCurrentTarget().lon);
	csv += ",";
	csv += std::to_string(throttle->getThrottle());
	csv += ",";
	if (this->getCurrentTarget().isValid()) {
		csv += std::to_string(lastFix.fix.bearing(this->getCurrentTarget()));
	} else csv += "N/A";
	csv += ",";
	csv += std::to_string(rudder->readMicroseconds());
	csv += ",";
	csv += std::to_string(orient->getOrientation()->heading);
	csv += ",";
	csv += boatModeNames.get(getBoatMode());
	csv += ",";
	csv += navModeNames.get(getNavMode());
	csv += ",";
	csv += autoModeNames.get(getAutoMode());
	csv += ",";
	csv += rcModeNames.get(getRCMode());
	csv += ",";
	csv += std::to_string(adc->getRawValues()["mot_i"]);
	csv += ",";
	csv += std::to_string(adc->getRawValues()["battery_mon"]);
	return csv;
}

std::string BoatState::getCSVheaders() {
	std::string headers;
	headers = "Record Time, Lat, Lon, GPS Track (deg true), Speed (m/s),Fix Type, Waypoint #,Waypoint Lat,Waypoint Lon, Target Course, ";
	headers += "Throttle Position,Rudder Command (ms),Current Heading (deg mag),Boat Mode, Nav Mode, Auto Mode, ";
	headers += "RC Mode, Raw Motor Current, Raw Battery Voltage";
	return headers;
}

ArmButtonStateEnum BoatState::getArmState () {
	int armval = this->armInput.get();
	int disarmval = this->disarmInput.get();
	if (armval < 0) {
		LOG(WARNING) << "Arm GPIO input is invalid";
		return ArmButtonStateEnum::INVALID;
	}
	if (disarmval < 0) {
		LOG(WARNING) << "Disarm GPIO input is invalid";
		return ArmButtonStateEnum::INVALID;
	}
	// This conditional compilation is so that this will work both with the directly connected arm/stop buttons
	// and also with the new power distribution setup
	#ifdef DISTRIB_IMPLEMENTED
		#pragma message "Compiling for new power distribution box"
		if (armval == disarmval) {
			LOG(WARNING) << "Arm and disarm inputs are equal: " << std::to_string(armval);
			return ArmButtonStateEnum::INVALID;
		}
		if (disarmval > 0) return ArmButtonStateEnum::DISARM;
		if (armval > 0) return ArmButtonStateEnum::ARM;
	#else
		#pragma message "Compiling for direct arm/disarm buttons"
		if ((disarmval == 0) && (armval == 0)) {
			LOG(WARNING) << "Arm and disarm inputs are equal: " << std::to_string(armval);
			return ArmButtonStateEnum::INVALID;
		}
		sysclock now = std::chrono::system_clock::now();
		if (disarmval > 0) disarmedStart = std::chrono::system_clock::now();
		if (armval > 0) armedStart = std::chrono::system_clock::now();
		if ((disarmval == 0) && ((now - disarmedStart) > 500ms)) {
			buttonArmed = false;
			return ArmButtonStateEnum::DISARM;
		}
		if ((armval == 0) && ((now - armedStart) > 5s)) {
			buttonArmed = true;
			return ArmButtonStateEnum::ARM;
		}
		if (buttonArmed) return ArmButtonStateEnum::ARM;
		return ArmButtonStateEnum::DISARM;
	#endif /* DISTRIB_IMPLEMENTED */
}

Command::Command (BoatState *state, const string cmd, const Value& args) :
	_state(state), _cmd(cmd), root() {
		_args.CopyFrom(args, root.GetAllocator());
		this->_funcs.at(_cmd);	// force an exception on an invalid command name
	};

Command::Command (BoatState *state, const string cmd) :
	_state(state), _cmd(cmd) {
		this->_funcs.at(_cmd);	// force an exception on an invalid command name
	};

bool Command::execute () {
	function<bool(Value&, BoatState*)> cmd = this->_funcs.at(_cmd);
	return cmd(_args, _state);
}

Value Command::pack () const {
	Value d;
	int p = 0;
	p += HackerboatState::PutVar("command", _cmd, d);
	p += HackerboatState::PutVar("argument", _args, d);
	
	return d;
}

bool Command::SetMode(Value& args, BoatState *state) {
	if (!state) {
		LOG(WARNING) << "Command function called without valid BoatState pointer";
		return false;
	}
	string mode;
	if (HackerboatState::GetVar("mode", mode, args)) {
		BoatModeEnum newmode;
		if (state->boatModeNames.get(mode, &newmode)) {
			state->setBoatMode(newmode);
			LOG(DEBUG) << "Setting boat mode to " << mode;
			return true;
		} else {
			LOG(ERROR) << "Invalid boat mode " << mode;
			return false;
		}
	} else
	LOG(DEBUG) << "No mode argument for boat mode";
	return false;
}

bool Command::SetNavMode(Value& args, BoatState *state) {
	if (!state) {
		LOG(WARNING) << "Command function called without valid BoatState pointer";
		return false;
	}
	string mode;
	if (HackerboatState::GetVar("mode", mode, args)) {
		NavModeEnum newmode;
		if (state->navModeNames.get(mode, &newmode)) {
			state->setNavMode(newmode);
			LOG(DEBUG) << "Setting nav mode to " << mode;
			return true;
		} else {
			LOG(DEBUG) << "Invalid nav mode " << mode;
			return false;
		}
	}
	LOG(DEBUG) << "No mode argument for nav mode";
	return false;
}

bool Command::SetAutoMode(Value& args, BoatState *state) {
	if (!state) {
		LOG(WARNING) << "Command function called without valid BoatState pointer";
		return false;
	}
	string mode;
	if (HackerboatState::GetVar("mode", mode, args)) {
		AutoModeEnum newmode;
		if (state->autoModeNames.get(mode, &newmode)) {
			state->setAutoMode(newmode);
			LOG(DEBUG) << "Setting auto mode to " << mode;
			return true;
		} else {
			LOG(DEBUG) << "Invalid auto mode " << mode;
			return false;
		}
	}
	LOG(DEBUG) << "No mode argument for auto mode";
	return false;
}

bool Command::SetHome(Value& args, BoatState *state) {
	if (!state) {
		LOG(WARNING) << "Command function called without valid BoatState pointer";
		return false;
	}
	if ((!args.HasMember("location")) && state->lastFix.isValid()) {
		state->launchPoint = state->lastFix.fix;
		LOG(INFO) << "Setting launch point to current location, " << state->gps->getAverageFix();
		return true;
	} else {
		Location newhome;
		Value loc;
		if (HackerboatState::GetVar("location", loc, args)) {
			if (newhome.parse(loc) && (newhome.isValid())) {
				state->launchPoint = newhome;
				LOG(INFO) << "Setting launch point to " << state->launchPoint;
				return true;
			}
		}
	}
	LOG(DEBUG) << "No home point set";
	return false;
}

bool Command::ReverseShell(Value& args, BoatState *state) {
	return false;
}

bool Command::SetWaypoint(Value& args, BoatState *state) {
	if ((!state) || (!args.IsObject())) {
		LOG(WARNING) << "SetWaypoint command called without valid BoatState pointer and/or arguments";
		return false;
	}
	int number;
	if (HackerboatState::GetVar("number", number, args)) {
		if (number > state->waypointList.count()) number = state->waypointList.count();
		if (number < 0) number = 0;
		state->waypointList.setCurrent(number);
		LOG(INFO) << "Setting current waypoint to " << number;
		return true;
	}
	LOG(DEBUG) << "No valid Waypoint argument";
	return false;
}

bool Command::SetWaypointAction(Value& args, BoatState *state) {
	if ((!state) || (!args.IsObject())) {
		LOG(WARNING) << "SetWaypointAction command called without valid BoatState pointer and/or arguments";
		return false;
	}
	string mode;
	if (HackerboatState::GetVar("action", mode, args)) {
		WaypointActionEnum action;
		if (state->waypointList.actionNames.get(mode, &action)) {
			state->waypointList.setAction(action);
			LOG(INFO) << "Setting current waypoint action to " << mode;
			return true;
		} else {
			LOG(DEBUG) << "Invalid waypoint action " << mode;
			return false;
		}
	}
	LOG(DEBUG) << "No valid Waypoint action argument";
	return false;
}

bool Command::DumpPathKML(Value& args, BoatState *state) {
	return false;
}

bool Command::DumpWaypointKML(Value& args, BoatState *state) {
	return false;
}

bool Command::DumpObstacleKML(Value& args, BoatState *state) {
	return false;
}

bool Command::DumpAIS(Value& args, BoatState *state) {
	return false;
}

bool Command::FetchWaypoints(Value& args, BoatState *state) {
	return false;
}

bool Command::PushPath(Value& args, BoatState *state) {
	return false;
}

bool Command::SetPID(Value& args, BoatState *state) {
	if ((!state) || (!args.IsObject())) {
		LOG(WARNING) << "SetPID command called without valid BoatState pointer and/or arguments";
		return false;
	}
	double Kp, Ki, Kd;
	bool result = false;
	if (HackerboatState::GetVar("Kp", Kp, args)) {
		std::get<0>(state->K) = Kp;
		LOG(INFO) << "Setting proportional gain to " << Kp;
		result = true;
	}
	if (HackerboatState::GetVar("Ki", Ki, args)) {
		std::get<1>(state->K) = Ki;
		LOG(INFO) << "Setting integral gain to " << Ki;
		result = true;
		result = true;
	}
	if (HackerboatState::GetVar("Kd", Kd, args)) {
		std::get<2>(state->K) = Kd;
		LOG(INFO) << "Setting differential gain to " << Kd;
		result = true;
		result = true;
	}
	return result;
}

std::ostream& operator<< (std::ostream& stream, const Command& cmd) {
	Value json;
	json = cmd.pack();
	if (json.IsObject()) {
		stream << json;
	} else {
		stream << "{}";
	}
	return stream;
}

const map<std::string, std::function<bool(Value&, BoatState*)>> Command::_funcs = {
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
