#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>

#define ARDUINO_BUF_LEN 	4096
#define LOCAL_BUF_LEN		32768
#define MAX_URI_TOKENS		8
#define REST_ID				254
#define REST_NAME			"BoneHackerBoat"
#define MAX_TOKENS			5
#define MAX_TOKEN_LEN		64
#define HASHSEED			0xdeadbeef
#define GNSS_TTY			"/dev/ttyS4"
#define GNSS_UART			UART4
#define GNSS_BPS			B9600
#define GNSS_BAUD			Baud9600
#define GNSS_TIMEOUT		(180)
#define ARDUINO_TIMEOUT		(60)
#define ARDUINO_REST_TTY	"/dev/ttyS2"		
#define ARDUINO_LOG_TTY		"/dev/ttyS1"
#define ARDUINO_REST_UART	UART2		
#define ARDUINO_LOG_UART	UART1
#define ARDUINO_BPS			B115200	
#define ARDUINO_BAUD		Baud115200	
#define ARDUINO_RESET_PIN	GPIO_48
#define FRAME_LEN_NS		(500000000)		/// State machine execution frame length, in nanoseconds
#define UART_TIMEOUT		(100)		/// UART contention timeout, in milliseconds
#define UART_READ_TIMEOUT	(15)		/// UART read timeout, in milliseconds
#define SELFTEST_DELAY		(30)
#define SHORE_TIMEOUT		(60)
#define HORN_TIME			(2)
#define RETURN_TIMEOUT		(180)
#define LAUNCH_WAYPOINT		(0)

// New hardware declarations

#define ADC_UPPER_ADDR		(0x71)
#define ADC_LOWER_ADDR		(0x70)
#define ADC_I2C_BUS			(2)
#define IMU_I2C_BUS			(1)
#define THROTTLE_RELAY_VECTOR {"RED", "WHITE", "YELLOW", "REDWHT", "YLWWHT"}
#define LIGHTS_COUNT		(144)
#define AIS_MAX_TIME		(600s)
#define AIS_MAX_DISTANCE	(10)

// Mutex timeouts
# define IMU_LOCK_TIMEOUT	(300us)

// File names
#define DB_DIRECTORY		"/tmp" /* Or override with DB_DIRECTORY environment variable */
#define GPS_DB_FILE			"gps.db"
#define AIS_DB_FILE			"ais.db"
#define NAV_DB_FILE			"nav.db"
#define BONE_LOG_DB_FILE	"boatstate.db"
#define ARD_LOG_DB_FILE  	"boatstate.db"
#define WP_DB_FILE			"waypoint.db"

#define REST_LOGFILE		"REST.log"
#define NAV_LOGFILE       	"nav.log"
#define MAIN_LOGFILE		"main.log"

#endif /* CONFIG_H */


