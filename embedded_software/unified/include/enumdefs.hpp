/******************************************************************************
 * Hackerboat Enums
 * enumdef.hpp
 * This module defines the enums for various subsystems
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Aug 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#ifndef ENUMDEFS_H
#define ENUMDEFS_H

/**
 * @brief Beaglebone controller mode, representing the overall system mode of the boat
 */
enum class BoatModeEnum : int {
	START		= 0,  		/**< Initial starting state         	*/
	SELFTEST	= 1,  		/**< Initial self-test            		*/
	DISARMED	= 2,  		/**< Disarmed wait state          		*/  
	FAULT		= 3,		/**< Beaglebone faulted           		*/ 
	NAVIGATION	= 4,		/**< Beaglebone armed & navigating		*/ 
	ARMEDTEST	= 5,		/**< Beaglebone accepts all commands that would be valid in any unsafe state */
	LOWBATTERY	= 6,		/**< Boat's main battery voltage is too low for operations */
	NONE		= 7			/**< State of the Beaglebone is currently unknown	*/
};

/**
 * @brief Navigation mode, representing the state of the nav subsystem
 */
enum class NavModeEnum : int  {
	IDLE		= 0,  		/**< Navigation idle state (or system not in nav mode)	*/
	FAULT		= 1,  		/**< Navigation faulted (boat goes to fault state)		*/
	RC			= 2,  		/**< Boat is under R/C control							*/  
	AUTONOMOUS	= 3,		/**< Boat is operating autonomously			       		*/ 
	NONE		= 4			/**< State of the nav system is currently unknown		*/
};

/**
 * @brief Autonomous mode, representing the state of the nav subsystem
 */
enum class AutoModeEnum : int  {
	IDLE		= 0,		/**< Nav system is not in autonomous mode 			*/
	WAYPOINT	= 1,  		/**< Boat is navigating along a list of waypoints.	*/
	RETURN		= 2,  		/**< Return to launch site.							*/
	ANCHOR		= 3,  		/**< Boat is holding position.						*/  
	NONE		= 4			/**< State of the nav system is currently unknown	*/
};

/**
 * @brief RC mode, representing the state of the nav subsystem
 */
enum class RCModeEnum : int  {
	IDLE		= 0,		/**< RC system is deactivated 						*/
	RUDDER		= 1,  		/**< Manual rudder and throttle						*/
	COURSE		= 2,  		/**< Steer the course selected on the controller.	*/
	FAILSAFE	= 3,		/**< RC is in failsafe mode							*/
	NONE		= 4			/**< State of the RC system is currently unknown	*/
};

/**
 * @brief Waypoint action, representing the action to take at the end of the waypoint list
 */

enum class WaypointActionEnum : int {
	IDLE,		/**< Do nothing -- navigation and autonomous mode both idle */
	ANCHOR,		/**< Anchor at the last waypoint. If triggered from RC Failsafe mode, anchor at current location. */
	RETURN,		/**< Return to launch site. */
	REPEAT,		/**< Go back to the first waypoint. If triggered from RC Failsafe mode, proceed with waypoints */
	NONE		/**< No action specified */
};

#endif
