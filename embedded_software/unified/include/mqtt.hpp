/******************************************************************************
 * Hackerboat MQTT interface module
 * mqtt.hpp
 * This module provides a shim for interfacing with the PAHO MQTT client
 * in order to allow the boat to talk to Adafruit.IO and other brokers.
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Oct 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef MQTT_H
#define MQTT_H
 
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

using namespace std;

typedef PubFuncMap map<string, function<void(BoatState*, MQTTClient*)>>;
typedef SubFuncMap map<string, function<void(BoatState*, string topic, string payload)>>;
typedef TokenMap map<string, MQTTClient_deliveryToken>;

class MQTT {
	public:
		MQTT (	BoatState *me						/// The BoatState vector that data is taken from and read to
				string host 	= MQTT_HOST,		 
				int port 		= MQTT_PORT,
				string username	= MQTT_USERNAME,
				string key 		= MQTT_KEY);
		int connect ();
		void disconnect ();
		bool isConnected();
		void setPubFuncMap (PubFuncMap *pubmap);	/// A map of the publish functions to call, by topic
		void publishNext();							/// call the next function in the _pub function list
		int publishAll();							/// call all of the functions in the _pub function list. Returns the number of functions executed
		bool isDelivered(string topic);				/// check whether the last message on the given topic has been delivered yet
		void setSubFuncMap (SubFuncMap *submap);	/// Set a map of the functions to call for each subscribed topic
		MQTTClient* getClient () {return &client;};	/// Get a pointer to the current client
		~MQTT();									/// We need an explicit destructor to make sure we clean up everything
		
		// publisher functions
		static void pub_SpeedLocation(BoatState* state, MQTTClient* client);	/// Publish the current GPS speed and location
		static void pub_Mode(BoatState* state, MQTTClient* client);				/// Publish the current mode as a CSV list of the form <Boat>,<Nav>,<RC>,<Auto>
		static void pub_Bearing(BoatState* state, MQTTClient* client);			/// Publish the current magnetic bearing, true bearing, and GPS course as a CSV list (in that order)
		static void pub_BatteryVoltage(BoatState* state, MQTTClient* client);	/// Publish the current battery voltage
		static void pub_RudderPosition(BoatState* state, MQTTClient* client);	/// Publish current rudder position
		static void pub_PID_K(BoatState* state, MQTTClient* client);			/// Publish current PID values
		
		// subscriber functions
		static void sub_Command(BoatState*, string topic, string payload);		/// Subscribe to commands from shore
		static void sub_PID_K(BoatState*, string topic, string payload);		/// Subscribe to PID updates from shore
		
	private;
		PubFuncMap *_pub;	// A map of the functions to call to publish different outgoing topics
		SubFuncMap *_sub;	// A map of functions to call when different topics are received
		TokenMap tokens;	// Delivery tokens for outbound topics
		int pubptr = 0;
		MQTTClient client;
		MQTTClient_connectOptions conn_opts;
		MQTTClient_SSLOptions ssl_opts;
		bool connected;
		string uri;
		void delivered(void *context, MQTTClient_deliveryToken dt);
		int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
		void connlost(void *context, char *cause);
};



#endif /* MQTT_H */