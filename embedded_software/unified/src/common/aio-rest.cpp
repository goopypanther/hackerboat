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

#include <cstdlib>
#include <inttypes.h>
#include <cstdio>
#include <string>
#include <list>
#include <iomanip>
#include <sstream>
#include <cctype>
#include "hal/config.h"
#include "private-config.h"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"
#include "aio-rest.hpp"
#include "easylogging++.h"
#include "rapidjson/rapidjson.h"
#include "configuration.hpp"
extern "C" {
	#include <curl/curl.h>
}

using namespace std;
using namespace rapidjson;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

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
		if (result == CURLE_OK) cnt++;
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
		//cerr << endl << "Polling " << r.first << endl;
		int result = r.second->poll();
		//cerr << "Result: " << result << endl;
		if (result >= 0) cnt++;
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
	CURLcode ret;
	CURL *hnd;
	struct curl_slist *slist1;
	char errbuf[CURL_ERROR_SIZE];
	string response;

	// assemble header string
	string keyheader = Conf::get()->restConf().at("key_header");
	keyheader += this->_key;

	// assemble URL string
	string url = this->_uri + this->_name + "/feeds/" + feedkey + "/data";
	//cerr << "Transmitting to url: " << url << endl;

	// assemble request
	slist1 = NULL;
	slist1 = curl_slist_append(slist1, keyheader.c_str());
	slist1 = curl_slist_append(slist1, "Content-Type: application/json");

	hnd = curl_easy_init();
	curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, payload.c_str());
	curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)payload.length());
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.38.0");
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_SSH_KNOWNHOSTS, "/home/debian/.ssh/known_hosts");
	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1);			// trigger a failure on a 400-series return code.
	
	// set up data writer callback
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&response);

	// make request
	ret = curl_easy_perform(hnd);

	// clean up request
	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;

	// if we have a good status code, record this as last contact
	if (ret == CURLE_OK) {
		state->lastContact = std::chrono::system_clock::now();
	}

	LOG_IF((ret != CURLE_OK), WARNING) << "AIO REST transmission failed: " << errbuf;
	LOG_EVERY_N(8, INFO) << "Publishing payload [" << payload << "] to feed: [" << feedkey << "] with HTTP code " << to_string(ret);
	return (int)ret;
}

string AIO_Rest::fetch(string feedkey, string specifier, int *httpStatus) {
	CURLcode ret;
	CURL *hnd;
	struct curl_slist *slist1;
	string response;
	char errbuf[CURL_ERROR_SIZE];

	// assemble header string
	string keyheader = Conf::get()->restConf().at("key_header");
	keyheader += this->_key;

	// assemble URL string
	hnd = curl_easy_init();
	string url = this->_uri + this->_name + "/feeds/" + feedkey + "/data";
	if (specifier.length()) {
		url += "?start_time=";
		url += curl_easy_escape(hnd, specifier.c_str(), specifier.size());
	}

	// assemble request
	slist1 = NULL;
	slist1 = curl_slist_append(slist1, keyheader.c_str());

	curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.38.0");
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_SSH_KNOWNHOSTS, "/home/debian/.ssh/known_hosts");
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, errbuf);
	curl_easy_setopt(hnd, CURLOPT_FAILONERROR, 1);			// trigger a failure on a 400-series return code.

	// set up data writer callback
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&response);

	// make request
	ret = curl_easy_perform(hnd);

	// clean up request
	curl_easy_cleanup(hnd);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;

	// if we have a good status code, record this as last contact
	if (ret == CURLE_OK) {
		state->lastContact = std::chrono::system_clock::now();
	}

	*httpStatus = ret;
	LOG_IF((ret != CURLE_OK), WARNING) << "AIO REST request failed" << errbuf;
	return response;
}

// AIO_Subscriber class functions

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
	string payload = "{\"value\":\"" + to_string(_me->lastFix.speed) + "\",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_Mode::pub() {
	string payload = "{\"value\":\"";
	payload += _me->boatModeNames.get(_me->getBoatMode());
	payload += "," + _me->navModeNames.get(_me->getNavMode());
	payload += "," + _me->autoModeNames.get(_me->getAutoMode());
	payload += "," + _me->rcModeNames.get(_me->getRCMode());
	payload += "\",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_MagHeading::pub() {
	if (!_me->orient) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->orient->getOrientation()->heading) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_GPSCourse::pub() {
	string payload = "{\"value\":";
	payload += to_string(_me->lastFix.track) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_BatteryVoltage::pub() {
	if (!_me->health) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->health->batteryMon) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_RudderPosition::pub() {
	if (!_me->rudder) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->rudder->read()) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_ThrottlePosition::pub() {
	if (!_me->throttle) return -1;
	string payload = "{\"value\":";
	payload += to_string(_me->throttle->getThrottle()) + ",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

int pub_FaultString::pub() {
	string payload = "{\"value\":\"";
	if(_me->getFaultString().length()) {
		payload += _me->getFaultString();
	} else {
		payload += "None";
	}
	payload += "\",";
	payload += "\"lat\":" + to_string(_me->lastFix.fix.lat) + ",";
	payload += "\"lon\":" + to_string(_me->lastFix.fix.lon) + ",";
	payload += "\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	if (_me->getFaultString().length()) {
		_last = true;
		return this->_rest->transmit(this->_key, payload);
	} else if (this->_last) {
		_last = false;
		return this->_rest->transmit(this->_key, payload);
	} else return CURLE_OK;
}

int pub_Waypoint::pub() {
	string payload = "{\"value\":\"";
	payload += _me->printCurrentWaypointNum();
	payload += "\",\"lat\":";
	payload += to_string(_me->getCurrentTarget().lat);
	payload += ",\"lon\":";
	payload += to_string(_me->getCurrentTarget().lon);
	payload += ",\"ele\":0.0}";
	//cerr << "Sending payload: " << payload << " to key: " << this->_key << endl;
	return this->_rest->transmit(this->_key, payload);
}

// Subscriber functors

int sub_Command::poll() {
	string payload, mostRecent;
	try {
		payload = _rest->fetch(_key, _lastMessageTime, &httpStatus);		// Fetch the desired feed, excluding anything that arrived before the last item processed.
	} catch (...) {
		LOG(ERROR) << "Subscription poll failed for unknown reasons" << endl;
		//cerr << "Subscription poll failed for unknown reasons" << endl;
		return -1;
	}
	if (!payload.length()) return 0;					// If no payload string, depart
	if (httpStatus != CURLE_OK) { 				// If we didn't get a good HTTP status code, depart
		//cerr << "HTTP failure on poll" << endl;
		return -1;
	} 
	cerr << "Received command payload is: " << payload << endl;
	Document input, val, element;
	input.Parse(payload.c_str());
	if (!input.HasParseError() && input.IsArray()) {
		for (int i = input.Size() - 1; i >= 0; i--) {			// we run this backwards so that the commands end up on the queue in chronological order													// if we should somehow get a null, skip to the next
			if (input[i].HasMember("created_at") && 
				input[i]["created_at"].IsString() &&
				input[i].HasMember("value") &&
				input[i]["value"].IsString()) {
					mostRecent = input[i]["created_at"].GetString();		// grab the time this was created at...
					if (mostRecent.compare(_lastMessageTime) == 0) {							// if we've seen this before, skip it
						continue;
					}
					string value = input[i]["value"].GetString();
					value = stripEscape(value);
					val.Parse(value.c_str());								// load in the JSON from the string
					if (!val.HasParseError() && 
						val.HasMember("command")) {
							if (val.HasMember("argument")) {
								_me->pushCmd(val["command"].GetString(), 
											val["argument"].GetObject());
							} else {
								_me->pushCmd(val["command"].GetString());
							}
					} else {
						LOG(ERROR) << "Failed to parse command [" << value << "]";
					}
			}
		} 
	} else {
		LOG(ERROR) << "Failed to parse incoming payload [" << payload << "]";
		LOG(ERROR) << "JSON error code: " << input.GetParseError() << " offset: " << input.GetErrorOffset();
		cerr << "Poll command JSON failure" << endl;
		return -1;
	}
	// We haven't really validated this input, so it's possible that it's garbage and that could cause us to re-execute old commands
	// This needs to be fixed b/c it's a potential security hole
	if (mostRecent != "") _lastMessageTime = mostRecent;
	return payload.length();
}

// This is the write callback for libcurl
// userp is expected to be of type std::string; if it is not, bad strange things will happen
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	string *target = (string *)userp;
	char *ptr = (char *)contents;

	//target->clear();
	size *= nmemb;
	target->reserve(size);
	for (size_t i = 0; i < size; i++) {
		*target += ptr[i];
	}
	return size;
}
