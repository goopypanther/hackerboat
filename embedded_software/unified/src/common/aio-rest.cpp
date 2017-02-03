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
#include <cstring>
#include <string>
#include <list>
#include <iomanip>
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

AIO_Rest::AIO_Rest (BoatState *me, string host, string username, string key, string group, string datatype, std::chrono::system_clock::duration subper) :
	_uri(host), _name(username), _key(key), _group(group), _datatype(datatype), state(me) {this->period = subper;}

void AIO_Rest::setPubFuncMap (PubFuncMap *pubmap) {
	_pub = pubmap;
	pubit = _pub->begin();
}

int AIO_Rest::publishNext() {
	if (!state) return -1;
	cout << "Publishing " << pubit->first << endl;
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
		cout << "Publishing " << r.first << endl;
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
		VLOG(2) << "Polling " << r.first;
		int result = r.second->poll();
		if (result) cnt++;
	}
	return cnt;
}

// thread functions

bool AIO_Rest::begin() {
	this->myThread = new std::thread (InputThread::InputThreadRunner(this));
	myThread->detach();
	LOG(INFO) << "AIO REST subsystem started";
	return true;
}

bool AIO_Rest::execute() {
	bool status = true;
	int pubResult = this->publishNext();
	if ((pubResult >= 200) && (pubResult < 300)) {
		status = true;
	} else status = false;
	if (!pollSubs()) status = false;
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

	cout << "Command is: " << cmd << endl;

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
	cmd += "' -w '\\n%{http_code}\\n' ";								// Specify that the HTTP response code will be printed on the next line after the response
	cmd += this->_uri + this->_name + "/feeds/" + feedkey + "/data";	// Build the URL
	if (specifier.length()) cmd += "?" + specifier;

	cout << "Command is: " << cmd << endl;

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

	//LOG(DEBUG) << "Publishing payload [" << payload << "] to feed: [" << feedkey << "]";
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
string AIO_Subscriber::urlEncode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

string AIO_Subscriber::stripEscape(const string &value) {
	ostringstream stripped;
    stripped.fill('0');
    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);
        if (c != '\\') stripped << c;
	}
	return stripped.str();
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
	string payload = _rest->fetch(_key, urlEncode(_lastMessageTime), &httpStatus);		// Fetch the desired feed, excluding anything that arrived before the last item processed.
	if (!payload.length()) return 0;										// If no payload string, depart
	if ((httpStatus < 200) || (httpStatus >= 300)) return 0;				// If we didn't get a good HTTP status code, depart

	json_t *element, *input = json_loads(payload.c_str(), 0, &err);
	if (input) {
		element = json_array_get(input, 0);			// grab the first (most recent) command so we can set the time
		if (element) {
			_lastMessageTime = json_string_value(json_object_get(element, "created_at"));	// we just store the raw text because the time comparison only matters on the server side
		}
		for (size_t i = json_array_size(input) - 1; i >= 0; i--) {			// we run this backwards so that the commands end up on the queue in chronological order
			json_t *val;
			string value;
			element = json_array_get(input, i);
			if (!element) continue;											// if we should somehow get a null, skip to the next
			value = json_string_value(json_object_get(element, "value"));	// grab the command string from the element
			value = stripEscape(value);										// strip any escape characters we had to use to push it through Adafruit.IO
			val = json_loads(value.c_str(), 0, &err);						// load in the JSON from the string
			if (val) {
				_me->pushCmd(json_string_value(json_object_get(val, "command")),	// grab the command as a string
								(json_object_get(val, "argument")));				// and the argument as a JSON object
				LOG(DEBUG) << "Pushed command is [" << json_string_value(json_object_get(val, "command")) << "]";
				LOG(DEBUG) << "Pushed argument is [" << json_object_get(val, "argument") << "]";
			} else {
				LOG(ERROR) << "Failed to parse command [" << value << "]";
			}
			json_decref(val);
		}
	} else {
		LOG(ERROR) << "Failed to parse incoming payload [" << payload << "]";
		LOG(ERROR) << "JSON error: " << err.text << " source: " << err.source
					<< " line: " << to_string(err.line) << " column: " << to_string(err.column);
		return 0;
	}
	json_decref(input);
	return payload.length();
}
