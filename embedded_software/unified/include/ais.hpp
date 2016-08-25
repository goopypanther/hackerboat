/******************************************************************************
 * Hackerboat Beaglebone AIS module
 * ais.hpp
 * This module stores AIS data 
 * see the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef AIS_H
#define AIS_H
 
#include <jansson.h>
#include "hal/config.h"
#include <math.h>
#include <string>
#include <chrono>
#include "hackerboatRoot.hpp"
#include "location.hpp"

enum class AISMsgType : int {
	UNDEFINED		= 0,
	POSN_REPORT_A1	= 1,
	POSN_REPORT_A2	= 2,
	POSN_REPORT_A3	= 3,
	BASE_STATION	= 4,
	STATIC_DATA_A	= 5,
	POSN_REPORT_B	= 18,
	STATIC_DATA_B	= 24
};

enum class AISNavStatus : int {
	ENGINE				= 0,
	ANCHORED			= 1,
	NOT_UNDER_CMD		= 2,
	RESTRICTED_MANEUVER	= 3,
	CONSTRAINED_DRAUGHT	= 4,
	MOORED				= 5,
	AGROUND				= 6,
	FISHING				= 7,
	SAILING				= 8,
	HSC_NAV				= 9,
	WIG_NAV				= 10,
	AIS_SART			= 14,
	UNDEFINED			= 15
};

enum class AISShipType : int {
	UNAVAILABLE			= 0,	/**< Default value */
	WIG_ALL				= 20,	/**< Ekranoplans */
	WIG_HAZ_A			= 21,	/**< And another ekranoplan... */
	WIG_HAZ_B			= 22,	/**< Yet another ekranoplan... */
	WIG_HAZ_C			= 23,	/**< And another... */
	WIG_HAZ_D			= 24,	/**< Five ought to be enough for all the ekranoplans... */
	FISHING				= 30,	/**< But we only need one number for the oldest sort of vessel, amirite? */
	TOWING				= 31,	/**< An ordinary tow combination */
	TOWING_LARGE		= 32,	/**< A tow longer than 200m and/or wider than 25m */
	DREDGING			= 33,
	DIVING_OPS			= 34,
	MILITARY_OPS		= 35,
	SAILING				= 36,
	PLEASURE			= 37,
	HSC					= 40,
	HSC_HAZ_A			= 41,
	HSC_HAZ_B			= 42,
	HSC_HAZ_C			= 43,
	HSC_HAZ_D			= 44,
	HSC_NO_INFO			= 49,
	PILOT				= 50,
	SAR					= 51,
	TUG					= 52,
	PORT_TENDER			= 53,
	ANTI_POLLUTION		= 54,
	LAW_ENFORCEMENT		= 55,
	MEDICAL				= 58,
	NONCOMBATANT		= 59,
	PASSENGER			= 60,
	PASSENGER_HAZ_A		= 61,
	PASSENGER_HAZ_B		= 62,
	PASSENGER_HAZ_C		= 63,
	PASSENGER_HAZ_D		= 64,
	PASSENGER_NO_INFO	= 69,
	CARGO				= 70,
	CARGO_HAZ_A			= 71,
	CARGO_HAZ_B			= 72,
	CARGO_HAZ_C			= 73,
	CARGO_HAZ_D			= 74,
	CARGO_NO_INFO		= 79,
	TANKER				= 80,
	TANKER_HAZ_A		= 81,
	TANKER_HAZ_B		= 82,
	TANKER_HAZ_C		= 83,
	TANKER_HAZ_D		= 84,
	TANKER_NO_INFO		= 89,
	OTHER				= 90,
	OTHER_HAZ_A			= 91,
	OTHER_HAZ_B			= 92,
	OTHER_HAZ_C			= 93,
	OTHER_HAZ_D			= 94,
	OTHER_NO_INFO		= 99,
};

enum class AISEPFDType : int {
	UNDEFINED	= 0,
	GPS			= 1,
	GLONASS		= 2,
	GPS_GLONASS	= 3,
	LORAN_C		= 4,
	CHAYKA		= 5,
	INS			= 6,
	SURVEYED	= 7,
	GALILEO		= 8
};

class AISBase : public HackerboatStateStorable {
	public:
		AISBase () = default;
		
		virtual bool prune (Location& current);	/**< Test if this contact should be pruned. If true, it deletes itself from the database and should be deleted upon return. */
		
		sysclock 		lastContact;		/**< Time of last contact */
		sysclock 		lastTimestamp;		/**< Time of last time stamp from the target transmitter. */
		int 			mmsi = -1;			/**< MMSI of transmitter */
		Location		fix;				/**< Location of last position transmission */
		const std::string msgClass = "AIS";	/**< Message class from gpsd */
		std::string		device;				/**< Name of the device */
};

class AISShip : AISBase {
	public:
		AISShip () = default;
		AISShip (json_t *packet);				/**< Create a ship object from the given packet. */
		bool parseGpsdPacket (json_t *packet);	/**< Parse an incoming AIS packet. Return true if successful. Will fail is packet is bad or MMSIs do not match. */
		bool project ();						/**< Project the position of the current contact now. */
		bool project (sysclock time);			/**< Project the position of this contact at time_point. */
		bool prune (Location& current);
		bool parse (json_t *input);
		json_t *pack () const;
		bool isValid () const;
		HackerboatStateStorage& storage();
		bool fillRow(SQLiteParameterSlice) const USE_RESULT;
		bool readFromRow(SQLiteRowReference, sequence) USE_RESULT;
		
		AISNavStatus	status;					/**< Navigation status of target */
		double			turn;					/**< Rate of turn, degrees per minute. */
		double			speed;					/**< Speed in knots. */
		double			course;					/**< True course, degrees. */
		double			heading;				/**< Magnet heading, degrees. */
		int				imo;					/**< IMO number */
		std::string		callsign;				/**< Ship's callsign. */
		std::string		shipname;				/**< Name of ship. */
		AISShipType		shiptype;				/**< Type of ship. */
		int				to_bow;					/**< Distance from the GNSS receiver to the bow, in meters. */
		int				to_stern;				/**< Distance from the GNSS receiver to the stern, in meters. */
		int				to_port;				/**< Distance from the GNSS receiver to the port, in meters. */
		int				to_starboard;			/**< Distance from the GNSS receiver to the starboard, in meters. */
		AISEPFDType		epfd;					/**< Type of position locating device. */
		
	private:
		bool coreParse (json_t* input);			/**< Pieces of the parsing task shared between parse() and gpsdInputParse() */
		bool removeEntry ();					/**< Remove this entry from the database. Called only from prune() */
		HackerboatStateStorage *aisShipStorage = NULL;
	
};


#endif
