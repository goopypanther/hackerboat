#ifndef CONFIG_H
#define CONFIG_H

#define ARDUINO_BUF_LEN 	4096
#define LOCAL_BUF_LEN		32768
#define MAX_URI_TOKENS		8
#define REST_ID				254
#define REST_NAME			"BoneHackerBoat"
#define MAX_TOKENS			5
#define MAX_TOKEN_LEN		64
#define HASHSEED			0xdeadbeef
#define GNSS_TTY			"/dev/ttyO4"
#define GNSS_UART			UART4
#define GNSS_BPS			B57600
#define GNSS_BAUD			Baud57600
#define ARDUINO_REST_TTY	"/dev/ttyO2"		
#define ARDUINO_LOG_TTY		"/dev/ttyO1"
#define ARDUINO_REST_UART	UART2		
#define ARDUINO_LOG_UART	UART1
#define ARDUINO_BPS			B115200	
#define ARDUINO_BAUD		Baud115200	
#define ARDUINO_RESET_PIN
#define FRAME_LEN_NS		(100000000)		/// State machine execution frame length, in nanoseconds
#define UART_TIMEOUT		(100000)		/// UART contention timeout, in microseconds
#define SELFTEST_FRAMES

// File names
#define	GPS_DB_FILE
#define	NAV_DB_FILE
#define	BONE_LOG_DB_FILE
#define ARD_LOG_DB_FILE
#define WP_DB_FILE	
#define REST_LOGFILE
#define NAV_LOGFILE
#define MAIN_LOGFILE

#endif /* CONFIG_H */

	