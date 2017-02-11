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
extern "C" {
	#include "MQTTClient.h"
}

#define MQTT_KEEPALIVE_SEC		120
#define MQTT_CONNECT_TIMEOUT	5
#define MQTT_RETRY_INTERVAL		5
#define MQTT_CLIENTID			"TDRoW"

using namespace std;

class MQTT;

typedef function<void(BoatState*, string topic, MQTTClient* client, MQTT* me)> PubFunc;
typedef function<void(BoatState*, string topic, string payload)> SubFunc;
typedef map<string, PubFunc> PubFuncMap;
typedef map<string, SubFunc> SubFuncMap;

#define MQTT_QOS 		(1)
#define MQTT_GROUP 		"hackerboat"
#define MQTT_DATATYPE	"csv"

class MQTT {
	public:
		MQTT (	BoatState *me,						/// The BoatState vector that data is taken from and read to
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
		static void pub_SpeedLocation(BoatState* state, string topic, MQTTClient* client, MQTT* me);		/// Publish the current GPS speed and location
		static void pub_Mode(BoatState* state, string topic, MQTTClient* client, MQTT* me);				/// Publish the current mode as a CSV list of the form <Boat>,<Nav>,<RC>,<Auto>
		static void pub_Bearing(BoatState* state, string topic, MQTTClient* client, MQTT* me);			/// Publish the current magnetic bearing, true bearing, and GPS course as a CSV list (in that order)
		static void pub_BatteryVoltage(BoatState* state, string topic, MQTTClient* client, MQTT* me);	/// Publish the current battery voltage
		static void pub_RudderPosition(BoatState* state, string topic, MQTTClient* client, MQTT* me);	/// Publish current rudder position
		static void pub_ThrottlePosition(BoatState* state, string topic, MQTTClient* client, MQTT* me);	/// Publish current throttle position
		static void pub_PID_K(BoatState* state, string topic, MQTTClient* client, MQTT* me);				/// Publish current PID values
		static void pub_Course(BoatState* state, string topic, MQTTClient* client, MQTT* me);			/// Publish GPS heading
		
		// subscriber functions
		static void sub_Command(BoatState* state, string topic, string payload);		/// Subscribe to commands from shore
		
		
	private:
		// transmission function
		static void transmit(string topic, string payload, MQTTClient* client, MQTT* me);
	
		// callbacks
		static void delivered(void *context, MQTTClient_deliveryToken dt);
		static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
		static void connlost(void *context, char *cause);
		
		static BoatState *_state;
		PubFuncMap *_pub;							/// A map of the functions to call to publish different outgoing topics
		static SubFuncMap *_sub;					/// A map of functions to call when different topics are received
		static MQTTClient_deliveryToken token;		/// Delivery token for the last message
		PubFuncMap::iterator pubit;					// iterator pointed to next item to publish
		MQTTClient client;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		//MQTTClient_SSLOptions ssl_opts;
		bool connected;
		string uri;
		string _name;
		string _key;
		string clientID;
};

BoatState* MQTT::_state;
SubFuncMap* MQTT::_sub;
MQTTClient_deliveryToken MQTT::token;

#endif /* MQTT_H */