#ifndef CONFIG_H
#define CONFIG_H

#include <chrono>

#define GNSS_TTY			"/dev/ttyS4"
#define GNSS_UART			UART4
#define GNSS_BPS			B9600
#define GNSS_TIMEOUT		(180)
#define FRAME_LEN			(100ms)		/// State machine execution frame length
#define UART_TIMEOUT		(100ms)		/// UART contention timeout
#define UART_READ_TIMEOUT	(15ms)		/// UART read timeout
#define SHORE_TIMEOUT		(60s)
#define RETURN_TIMEOUT		(180s)
#define LAUNCH_WAYPOINT		(0)

// New hardware declarations

#define ADC_UPPER_ADDR				(0x1f)
#define ADC_LOWER_ADDR				(0x1d)
#define ADC_I2C_BUS					(2)
#define IMU_I2C_BUS					(1)
#define THROTTLE_MAX				(5)
#define THROTTLE_MIN				(-5)
#define RUDDER_MAX					(100)
#define RUDDER_MIN					(-100)
#define RUDDER_DIRECTION			(0)
#define RUDDER_PERIOD				(100)
#define COURSE_MAX					(360.0)
#define COURSE_MIN					(0.0)
#define LIGHTS_COUNT				(72)
#define AIS_MAX_TIME				(600s)
#define AIS_MAX_DISTANCE			(10)
#define CONFIG_PIN_PATH				"/usr/local/bin/config-pin"
#define SYSTEM_DISARM_INPUT_PORT	(8)
#define SYSTEM_DISARM_INPUT_PIN		(22)
#define SYSTEM_ARM_INPUT_PORT		(8)
#define SYSTEM_ARM_INPUT_PIN		(20)
#define	SYSTEM_SERVO_ENB_PORT		(8)
#define SYSTEM_SERVO_ENB_PIN		(19)
#define	RUDDER_PORT					(9)
#define RUDDER_PIN					(16)
#define PID_KP						(1.0)
#define PID_KI						(0.1)
#define PID_KD						(0.0)

// Scales and offsets
#define IMU_MAG_OFFSET				{{'x',330},{'y',-86},{'z',386}}	
#define IMU_MAG_SCALE				{{'x',0.15835},{'y',0.18519},{'z',0.18149}}			

// RC defines

#define RC_THROTTLE_CH				(0)
#define RC_RUDDER_CH				(3)
#define RC_AUTO_SWITCH				(4)
#define RC_MODE_SWITCH				(5)
#define RC_COURSE_SELECTOR			(6)
#define RC_MIN						(171)
#define RC_MAX						(1811)
#define RC_MIDDLE_POSN				(1028)
#define RC_SERIAL_PATH				"/dev/ttyS5"
#define RC_CHANNEL_COUNT			(18)
#define RC_HORN_SWITCH				(7)

// Autonomous definitions
#define AUTO_DEFAULT_THROTTLE 	(5)
#define AUTO_WAYPOINT_TOL		(50.0)
#define ANCHOR_DEADBAND			(5.0)
#define ANCHOR_THROTTLE_GAIN	(1/20)

// Timeouts

#define GPS_SENSE_TIMEOUT	(5s)
#define RC_SENSE_TIMEOUT	(500ms)
#define IMU_SENSE_TIMEOUT	(500ms)
#define IMU_LOCK_TIMEOUT	(300us)
#define RC_LOCK_TIMEOUT		(300us)
#define SELFTEST_DELAY		(30s)
#define ADC_START_COUNT		(30)  
#define ADC_START_PERIOD	(500us)
#define ADC_LOCK_TIMEOUT	(500us)
#define DISARM_PULSE_LEN	(50ms)
#define ARM_PULSE_LEN		(50ms)
#define HORN_TIME			(2s)

// Sense periods

#define IMU_READ_PERIOD		(10ms)
#define ADC_READ_PERIOD		(10ms)
#define RC_READ_PERIOD		(8ms)
#define GPS_READ_PERIOD		(20ms)

// Voltage/current limits

#define SYSTEM_START_BATTERY_MIN	(12.0)
#define SYSTEM_LOW_BATTERY_CUTOFF	(10.0)

// Log file names
#define MAIN_LOGFILE		"main.log"
#define HARDWARE_LOGFILE	"hardware.log"
#define MQTT_LOGFILE		"mqtt.log"

// Relay map initializer
#define RELAY_MAP_INITIALIZER	{ { "RED", Relay("RED", new Pin(8, 3, true), new Pin(8, 4, false)) }, \
								  { "DIR", Relay("DIR", new Pin(8, 5, true), new Pin(8, 6, false)) }, \
								  { "YLWWHT", Relay("YLWWHT", new Pin(8, 7, true), new Pin(8, 8, false)) }, \
								  { "REDWHT", Relay("REDWHT", new Pin(8, 9, true), new Pin(8, 10, false)) }, \
								  { "YLW", Relay("YLW", new Pin(8, 11, true), new Pin(8, 12, false)) }, \
								  { "WHT", Relay("WHT", new Pin(8, 13, true), new Pin(8, 14, false)) }, \
								  { "DISARM", Relay("DISARM", new Pin(8, 15, true), new Pin(8, 16, false)) }, \
								  { "HORN", Relay("HORN", new Pin(8, 17, true), new Pin(8, 18, false)) }, \
								  { "ENABLE", Relay("ENABLE", new Pin(8, 24, true), new Pin(8, 26, false)) } }

// Analog initializers
#define ADC_UPPER_INITIALIZER	{"RED", "DIR", "YLWWHT", "REDWHT", "YLW", "WHT", "DISARM", "ENABLE"}
#define ADC_LOWER_INITIALIZER	{"HORN", "mot_i", "mot_v", "charge_v", "charge_i", "aux_0", "aux_1", "servo_i"}								  
#define ADC128D818_EXTERNAL_REF	(5.0)
#define ADC_BATMON_PATH			"/sys/devices/platform/ocp/44e0d000.tscadc/TI-am335x-adc/iio:device0/in_voltage1_raw"
#define ADC_BATMON_NAME			"battery_mon"

// Database names
#define DB_DIRECTORY		"/tmp" /* Or override with DB_DIRECTORY environment variable */
#define GPS_DB_FILE			"gps.db"
#define AIS_DB_FILE			"ais.db"
#define STATE_DB_FILE		"state.db"
#define HEALTH_DB_FILE		"health.db"

#endif /* CONFIG_H */


