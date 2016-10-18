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
#include "hal/config.h"
#include "private-config.h"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"
#include "MQTTClient.h"
#include "mqtt.hpp"

using namespace std;
using namespace std::placeholders;

MQTT::MQTT (BoatState *me, string host, int port, string username, string key) :
	_state(me), clientID(MQTT_CLIENTID), _name(username), _key(key) {
		conn_opts.keepAliveInterval = MQTT_KEEPALIVE_SEC;
		conn_opts.username = _name.c_str();
		conn_opts.password = _key.c_str();
		uri = "tcp://" + host + ":" + to_string(port);
		MQTTClient_create(&client, uri.c_str(), clientID.c_str(), 
							MQTTCLIENT_PERSISTENCE_NONE, NULL);
		conn_opts.connectTimeout = MQTT_CONNECT_TIMEOUT;
		conn_opts.retryInterval = MQTT_RETRY_INTERVAL;
		conn_opts.cleansession = 1;
		conn_opts.ssl = NULL;
		auto a = bind(&MQTT::msgarrvd, this, _1, _2, _3, _4);
		auto b = bind(&MQTT::delivered, this, _1, _2);
		MQTTClient_setCallbacks(client, NULL, connlost, a, b);
	}
	
int MQTT::connect () {
	int rc = MQTTClient_connect(client, &conn_opts);
    return rc;
}

void MQTT::disconnect () {
	MQTTClient_disconnect(client, MQTT_CONNECT_TIMEOUT);
}

bool MQTT::isConnected() {
	return static_cast<bool>(MQTTClient_isConnected(client));
}

void MQTT::setPubFuncMap (PubFuncMap *pubmap) {
	_pub = pubmap;
	pubit = _pub->begin();
	// this bit of code makes sure the token map matches the function map so we have usable tokens
	token = 0;
}

void MQTT::publishNext() {
	pubit->second(_state, pubit->first, &client);
	if (pubit == _pub->end()) {
		pubit = _pub->begin();
	} else pubit++;
}

int MQTT::publishAll() {
	int cnt = 0;
	for (auto r: *_pub) {
		r.second(_state, r.first, &client);
		cnt++;
	}
	return cnt;
}

bool MQTT::isDelivered(string topic) {
	return (this->token != 0);
}

void MQTT::setSubFuncMap (SubFuncMap *submap) {
	_sub = submap;
}

MQTT::~MQTT() {
    disconnect();
    MQTTClient_destroy(&client);
}

// publisher functions
void MQTT::pub_SpeedLocation(BoatState* state, string topic, MQTTClient* client) {
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	string payload = to_string(state->lastFix.speed) + ",";
	payload += to_string(state->lastFix.fix.lat) + ",";
	payload += to_string(state->lastFix.fix.lon) + ",0.0";
	this->token = 0;
	pubmsg.payload = (void*)(payload.c_str());
	pubmsg.payloadlen = payload.length();
	MQTTClient_publishMessage(client, topic.c_str(), &pubmsg, &token);
}

//void MQTT::pub_Mode(BoatState* state, string topic, MQTTClient* client);				/// Publish the current mode as a CSV list of the form <Boat>,<Nav>,<RC>,<Auto>
//void MQTT::pub_Bearing(BoatState* state, string topic, MQTTClient* client);			/// Publish the current magnetic bearing, true bearing, and GPS course as a CSV list (in that order)
//void MQTT::pub_BatteryVoltage(BoatState* state, string topic, MQTTClient* client);	/// Publish the current battery voltage
//void MQTT::pub_RudderPosition(BoatState* state, string topic, MQTTClient* client);	/// Publish current rudder position
//void MQTT::pub_PID_K(BoatState* state, string topic, MQTTClient* client);				/// Publish current PID values

// subscriber functions
//void MQTT::sub_Command(BoatState*, string topic, string payload);		/// Subscribe to commands from shore
//void MQTT::sub_PID_K(BoatState*, string topic, string payload);		/// Subscribe to PID updates from shore

// callback functions
void MQTT::delivered(void *context, MQTTClient_deliveryToken dt) {
	this->token = dt;
}

int MQTT::msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
	string topic = topicName;
	string msg = static_cast<char*>(message->payload);
	try {
		this->_sub->at(topic)(this->_state, topic, msg);
		return 0;
	} catch (...) {
		return -1;
	}
	return -1;
}

void MQTT::connlost(void *context, char *cause) {
	
}
