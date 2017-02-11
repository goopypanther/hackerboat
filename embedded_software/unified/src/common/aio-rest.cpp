/******************************************************************************
 * Hackerboat MQTT interface module
 * mqtt.cpp
 * This module provides a shim for interfacing with the PAHO MQTT client
 * in order to allow the boat to talk to Adafruit.IO and other brokers.
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Oct 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <jansson.h>
#include <cstdlib>
#include <inttypes.h>
#include <cstdio>
#include <string>
#include <list>
#include <iomanip>
#include <sstream>
#include <cctype>
#include "json_utilities.hpp"
#include "hal/config.h"
#include "private-config.h"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"
#include "aio-rest.hpp"
#include "easylogging++.h"
#include "pstream.h"

using namespace std;
using namespace redi;

AIO_Rest::AIO_Rest (BoatState *me, string host, string username, string key, string group, string datatype,
	std::chrono::system_clock::duration subper, std::chrono::system_clock::duration pubper) :
	_uri(host), _name(username), _key(key), _group(group), _datatype(datatype), state(me),
	_pubper(pubper), _subper(subper) {
		// We use the Euclidean algorithm to find the GCD of the two times...
		int cnt = 0;
		while (subper != pubper) {
			cnt++;
			if (subper > pubper) {
				subper = subper - pubper;
			} else {
				pubper = pubper - subper;
			}
			if (cnt > 100) break;
		}
		this->period = subper;
		LOG(DEBUG) << "Thread period is " << std::chrono::duration_cast<std::chrono::milliseconds>(this->period).count() << " ms";
		lastpub = chrono::system_clock::now();
		lastsub = lastpub;
	}

void AIO_Rest::setPubFuncMap (PubFuncMap *pubmap) {
	_pub = pubmap;
	pubit = _pub->begin();
}

int AIO_Rest::publishNext() {
	if (!state) return -1;
	int result = pubit->second->pub();
	VLOG(2) << "Publishing " << pubit->first;
	if (++pubit == _pub->end()) {
		pubit = _pub->begin();
	}
	return result;
}

int AIO_Rest::publishAll() {
	if (!state) return -1;
	int cnt = 0;
	VLOG(1) << "Publishing all published items";
	for (auto r: *_pub) {
		VLOG(2) << "Publishing " << r.first;
		int result = r.second->pub();
		if ((result >= 200) && (result < 300)) cnt++;
	}
	return cnt;
}

void AIO_Rest::setSubFuncMap (SubFuncMap *submap) {
	_sub = submap;
}

int AIO_Rest::pollSubs() {
	if (!state) return -1;
	int cnt = 0;
	VLOG(1) << "Polling all subscribed channels";
	for (auto r: *_sub) {
		LOG(DEBUG) << "Polling " << r.first;
		int result = r.second->poll();
		if (result) cnt++;
	}
	return cnt;
}

// thread functions

bool AIO_Rest::begin() {
	this->myThread = new std::thread (InputThread::InputThreadRunner(this));
	myThread->detach();
	lastsub = chrono::system_clock::now();
	lastpub = lastsub;
	LOG(INFO) << "AIO REST subsystem started";
	return true;
}

bool AIO_Rest::execute() {
	bool status = true;
	chrono::system_clock::time_point thistime = chrono::system_clock::now();
	LOG(DEBUG) << "Hitting AIO_REST thread";
	if ((lastpub + _pubper) < thistime) {
		int pubResult = this->publishNext();
		lastpub = thistime;
		LOG(DEBUG) << "Publishing to next feeds, result: " << to_string(pubResult);
	}
	if ((lastsub + _subper) < thistime) {
		if (!pollSubs()) status = false;
		lastsub = thistime;
		LOG(DEBUG) << "Polling all feeds";
	}
	LOG(DEBUG) << "Exiting AIO_REST thread";
	return status;
}

// communication functions

int AIO_Rest::transmit (string feedkey, string payload) {
	pstreambuf curlstr;
	string respbuf, statusbuf;
	int i = 0;

	// build command string
	string cmd = REST_CURL;												// start with the curl command
	cmd += " -H \"" + string(REST_AIO_KEY_HEADER); 						// Insert header for the API key
	cmd += string(REST_KEY) + "\"";										// Insert the API key
	cmd += " -H \"Content-Type: application/json\" -X POST -d "; 		// This is the preamble required to POST JSON correctly
	cmd += "'" + payload + "' -w '\\n%{http_code}\\n' ";				// Add the payload and specify that the HTTP response code will be printed on the next line
	cmd += this->_uri + this->_name + "/feeds/" + feedkey + "/data";	// Build the URL

	// fire the command off and wait for a response
	curlstr.open(cmd, pstreams::pstdout | pstreams::pstderr);
	LOG(DEBUG) << "Querying AIO with " << cmd;
	while (!curlstr.in_avail() && (i < REST_COUNT_OUT)) {
		i++;
		this_thread::sleep_for(REST_DELAY);
	}
	if (i >= REST_COUNT_OUT) {
		curlstr.close();
		return -1;
	}

	// read the data
	respbuf = getResponse(&curlstr);
	statusbuf = getResponse(&curlstr, 10);

	// parse the status code
	int response = stoi(statusbuf);

	// if we have a good status code, record this as last contact
	if ((response >= 200) && (response < 300)) {
		state->lastContact = std::chrono::system_clock::now();
	}

	//LOG(DEBUG) << "Publishing payload [" << payload << "] to feed: [" << feedkey << "]";
	LOG_IF(((response < 200) || (response >= 300)), WARNING) << "AIO REST transmission failed: " << cmd;
	LOG_EVERY_N(8, INFO) << "Publishing payload [" << payload << "] to feed: [" << feedkey << "] with HTTP code " << response;
	return response;
}

string AIO_Rest::fetch(string feedkey, string specifier, int *httpStatus) {
	pstreambuf curlstr;
	string respbuf, statusbuf;
	int i = 0;

	// build command string
	string cmd = REST_CURL;												// start with the curl command
	cmd += " -H \"" + string(REST_AIO_KEY_HEADER); 						// Insert header for the API key
	cmd += string(REST_KEY) + "\"";										// Insert the API key
	cmd += " -w '\\n%{http_code}\\n' ";								// Specify that the HTTP response code will be printed on the next line after the response
	cmd += this->_uri + this->_name + "/feeds/" + feedkey + "/data";	// Build the URL
	if (specifier.length()) cmd += "?start_time=" + specifier;

	// fire the command off and wait for a response
	curlstr.open(cmd, pstreams::pstdout | pstreams::pstderr);
	LOG(DEBUG) << "Querying AIO with " << cmd;
	while (!curlstr.in_avail() && (i < REST_COUNT_OUT)) {
		i++;
		this_thread::sleep_for(REST_DELAY);
	}
	if (i >= REST_COUNT_OUT) {
		curlstr.close();
		return "";
	}

	// read the data
	respbuf = getResponse(&curlstr);
	statusbuf = getResponse(&curlstr, 10);

	// parse the status code
	*httpStatus = stoi(statusbuf);

	// if we have a good status code, record this as last contact
	if ((*httpStatus >= 200) && (*httpStatus < 300)) {
		state->lastContact = std::chrono::system_clock::now();
	}

	LOG_IF(((*httpStatus < 200) || (*httpStatus >= 300)), WARNING) << "AIO REST request failed: " << cmd;
	return respbuf;
}

string AIO_Rest::getResponse(	redi::pstreambuf *str,
								size_t maxlen,
								std::chrono::system_clock::duration timeout) {
	string buf = "";																// allocate a buffer
	auto endTime = std::chrono::system_clock::now() + timeout;						// figure out what time we should stop
	while (str->in_avail() || (std::chrono::system_clock::now() < endTime)) {		// wait for characters to show up on the input buffer and read them
		if (str->in_avail()) buf.push_back(str->sbumpc());							// grab the next character, if there is one
		if (buf.back() == '\n') break;												// look for the end of the line. This character should match the one passed to curl as a separator.
	}
	return buf;
}

// AIO_Subscriber class functions

// function shamelessly stolen from http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
// And since that didn't work reliably, here it is from their original source, http://www.geekhideout.com/urlcode.shtml
char AIO_Subscriber::toHex(char code) {
	static char hex[] = "0123456789ABCDEF";
	return hex[code & 15];
}

string AIO_Subscriber::urlEncode(const string &value) {
    string escaped = "";

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        unsigned char c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped += c;
        } else if (c == ' ') {
			escaped += "%20";
		} else {
			escaped += '%';
			escaped += toHex(c >> 4);	// hexify the top nibble
			escaped += toHex(c & 15);	// hexify the bottom nibble
		}
    }

    return escaped;
}

string AIO_Subscriber::stripEscape(const string &value) {
	string stripped = "";
    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        char c = (*i);
        if (c != '\\') stripped += c;
	}
	return stripped;
}

// Publish functors

int pub_SpeedLocation::pub() {
	string payload = "{\"value\":" + to_string(_me->lastFix.speed) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_Mode::pub() {
	string payload = "{\"value\":\"";
	payload += _me->boatModeNames.get(_me->getBoatMode());
	payload += "," + _me->navModeNames.get(_me->getNavMode());
	payload += "," + _me->autoModeNames.get(_me->getAutoMode());
	payload += "," + _me->rcModeNames.get(_me->getRCMode());
	payload += "\"}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_MagHeading::pub() {
	if (!_me->orient) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->orient->getOrientation()->heading) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_GPSCourse::pub() {
	string payload = "{\"value\":";
	payload += to_string(_me->lastFix.track) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_BatteryVoltage::pub() {
	if (!_me->health) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->health->batteryMon) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_RudderPosition::pub() {
	if (!_me->rudder) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->rudder->read()) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_ThrottlePosition::pub() {
	if (!_me->throttle) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->throttle->getThrottle()) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	return this->_rest->transmit(this->_key, payload);
}

int pub_FaultString::pub() {
	string payload = "{\"value\":\"";
	payload += _me->getFaultString();
	payload += "\"}";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	if (_me->getFaultString().length()) {
		_last = true;
		return this->_rest->transmit(this->_key, payload);
	} else if (this->_last) {
		_last = false;
		return this->_rest->transmit(this->_key, payload);
	} else return 200;
}

// Subscriber functors

int sub_Command::poll() {
	json_error_t err;
	string payload, mostRecent;
	try {
		payload = _rest->fetch(_key, urlEncode(_lastMessageTime), &httpStatus);		// Fetch the desired feed, excluding anything that arrived before the last item processed.
	} catch (...) {
		LOG(ERROR) << "Subscription poll failed for unknown reasons" << endl;
		return -1;
	}
	if (!payload.length()) return 0;										// If no payload string, depart
	if ((httpStatus < 200) || (httpStatus >= 300)) return 0;				// If we didn't get a good HTTP status code, depart

	json_t *element, *input = json_loads(payload.c_str(), 0, &err);
	if (input) {
		for (int i = json_array_size(input) - 1; i >= 0; i--) {			// we run this backwards so that the commands end up on the queue in chronological order
			json_t *val;
			string value;
			element = json_array_get(input, i);
			if (!element) continue;														// if we should somehow get a null, skip to the next
			mostRecent = json_string_value(json_object_get(element, "created_at"));		// grab the time this was created at...
			if (mostRecent.compare(_lastMessageTime) == 0) {							// if we've seen this before, skip it
				json_decref(element);
				continue;
			}
			value = json_string_value(json_object_get(element, "value"));	// grab the command string from the element
			value = stripEscape(value);										// strip any escape characters we had to use to push it through Adafruit.IO
			val = json_loads(value.c_str(), 0, &err);						// load in the JSON from the string
			if (val) {
				string cmdstring = json_string_value(json_object_get(val, "command"));	// grab the command
				json_t *argjson =  json_object_get(val, "argument");					// grab the JSON in the arguments
				json_incref(argjson);													// we need to increment the reference because it's borrowed and would otherwise be destroyed when we call json_decref(val)
				_me->pushCmd(cmdstring, argjson);
				LOG(DEBUG) << "Pushed command is [" << json_string_value(json_object_get(val, "command")) << "]" << endl;
				LOG(DEBUG) << "Pushed argument is [" << json_object_get(val, "argument") << "]";
			} else {
				LOG(ERROR) << "Failed to parse command [" << value << "]";
			}
			if (val) json_decref(val);
			if (element) json_decref(element);
		}
	} else {
		LOG(ERROR) << "Failed to parse incoming payload [" << payload << "]";
		LOG(ERROR) << "JSON error: " << err.text << " source: " << err.source
					<< " line: " << to_string(err.line) << " column: " << to_string(err.column);
		return 0;
	}
	// We haven't really validated this input, so it's possible that it's garbage and that could cause us to re-execute old commands
	// This needs to be fixed b/c it's a potential security hole
	if (mostRecent != "") _lastMessageTime = mostRecent;
	json_decref(input);
	return payload.length();
}
