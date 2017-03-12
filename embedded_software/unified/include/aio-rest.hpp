/******************************************************************************
 * Hackerboat Adafruit.IO REST interface module
 * aio-rest.hpp
 * This module provides an interface to curl in order to
 * allow the boat to talk to Adafruit.IO and other brokers.
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Oct 2016
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#ifndef AIO_REST_H
#define AIO_REST_H

#include <jansson.h>
#include <cstdlib>
#include <inttypes.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <list>
#include "pstream.h"
#include "hal/inputThread.hpp"
#include "hal/config.h"
#include "private-config.h"
#include "hackerboatRoot.hpp"
#include "boatState.hpp"

#define REST_GROUP 		""
#define REST_DATATYPE	"json"

using namespace std;

class AIO_Rest;

class AIO_Subscriber {
	public:
		AIO_Subscriber (BoatState *me, AIO_Rest *rest, string feedkey) :
			_me(me), _rest(rest), _key(feedkey), _lastMessageTime(""), httpStatus(0) {};
		virtual int poll() = 0;									// returns number of bytes received
		int getStatus() {return httpStatus;};
	protected:
//		char toHex(char code);
//		string urlEncode(const string &value);		// url encode a string
		string stripEscape(const string &value);	// strip backslash escapes out of a string
		string 			_key;
		BoatState		*_me;
		AIO_Rest		*_rest;
		string 			_lastMessageTime;
		int				httpStatus;
};

class AIO_Publisher {
	public:
		AIO_Publisher (BoatState *me, AIO_Rest *rest, string feedkey) :
			_me(me), _rest(rest), _key(feedkey) {};
		virtual int pub() = 0;									// returns http status code
	protected:
		string 		_key;
		BoatState	*_me;
		AIO_Rest	*_rest;
};

typedef map<string, AIO_Publisher*> PubFuncMap;
typedef map<string, AIO_Subscriber*> SubFuncMap;

class AIO_Rest : public InputThread  {
	public:
		AIO_Rest (BoatState *me,						/// The BoatState vector that data is taken from and read to
				string host 	= REST_HOST,
				string username	= REST_USERNAME,
				string key 		= REST_KEY,
				string group	= REST_GROUP,
				string datatype	= REST_DATATYPE,
				std::chrono::system_clock::duration subper = REST_SUBSCRIPTION_PERIOD,
				std::chrono::system_clock::duration pubper = REST_PUBLISH_PERIOD);
		bool begin();								/// Start the
		bool execute();								/// Get the next subscription
		void setPubFuncMap (PubFuncMap *pubmap);	/// A map of the publish functions to call, by topic
		int publishNext();							/// call the next function in the _pub function list. Returns the HTTP response code
		int publishAll();							/// call all of the functions in the _pub function list. Returns the number of functions successfully executed (i.e. 200 series response code)
		void setSubFuncMap (SubFuncMap *submap);	/// Set a map of the functions to call for each subscribed topic
		int pollSubs();								/// Polls all subscribed channels. Returns the number of channels with new data

		// transmission functions
		int transmit(string feedkey, string payload);						/// Attempts to add the given payload to the given feed. Returns HTTP response code.
		string fetch(string feedkey, string specifier, int *httpStatus);	/// Fetches the last data from the given feed with the given specifier, which must be URL encoded. Returns the response string.

	private:

		BoatState *state;
		PubFuncMap *_pub;				/// A map of the functions to call to publish different outgoing topics
		SubFuncMap *_sub;				/// A map of functions to call when different topics are received
		PubFuncMap::iterator pubit;		/// Iterator pointed to next item to publish
		std::chrono::system_clock::duration 	_subper;	/// Frequency of subscription polling
		std::chrono::system_clock::duration 	_pubper;	/// Frequency of publishing
		std::chrono::system_clock::time_point	lastsub;	/// Time of the last subscription poll
		std::chrono::system_clock::time_point	lastpub;	/// Time of the last publication

		bool 			_autopub;		/// If this is true, the running thread attempts to publish one output every thread call.
		string 			_uri;
		string 			_name;
		string 			_key;
		string 			_group;
		string 			_datatype;
		std::thread 	*myThread;
};

// Publisher classes
// Note that all published data includes a location

/// Publish the boat's current GPS speed and location
class pub_SpeedLocation : public AIO_Publisher {
	public:
		pub_SpeedLocation(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "speedlocation") {};
		int pub();
};

/// Publish the boat's current modes as a colon-separated list
class pub_Mode : public AIO_Publisher {
	public:
		pub_Mode(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "mode") {};
		int pub();
};

/// Publish the boat's current magnetic heading
class pub_MagHeading : public AIO_Publisher {
	public:
		pub_MagHeading(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "magheading") {};
		int pub();
};

/// Publish the boat's current GPS course
class pub_GPSCourse : public AIO_Publisher {
	public:
		pub_GPSCourse(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "gpscourse") {};
		int pub();
};

/// Publish the boat's main battery voltage as a float
class pub_BatteryVoltage : public AIO_Publisher {
	public:
		pub_BatteryVoltage(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "batteryvoltage") {};
		int pub();
};

/// Publish the current rudder position as an integer
class pub_RudderPosition : public AIO_Publisher {
	public:
		pub_RudderPosition(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "rudderposn") {};
		int pub();
};

/// Publish the current throttle position as an integer
class pub_ThrottlePosition : public AIO_Publisher {
	public:
		pub_ThrottlePosition(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "throttleposn") {};
		int pub();
};

/// Publish the current fault string, if present.
class pub_FaultString : public AIO_Publisher {
	public:
		pub_FaultString(BoatState *me, AIO_Rest *rest) :
			AIO_Publisher(me, rest, "faultstring"), _last(false) {};
		int pub();
	private:
		bool _last;		/// If the last time this object was called there was a fault string, this will be true.
};

// Subscriber classes
// Note that all subscriber classes will manipulate the given BoatState object, as appropriate

/// Subscribe to shore commands
class sub_Command : public AIO_Subscriber {
	public:
		sub_Command(BoatState *me, AIO_Rest *rest) :
			AIO_Subscriber(me, rest, "command") {};
		int poll();
};

#endif /* MQTT_H */
