
#include "json_utilities.hpp"
#include "stateStructTypes.hpp"
#include "arduinoState.hpp"
#include "test_utilities.hpp"
#include <gtest/gtest.h>

TEST(Serialization, ArduinoState) {


	const char *s =
	"{ "
	" \"popStatus\": true, "
	" \"uTime\": { \"tv_sec\": 17652, \"tv_nsec\": 823456 }, "
	" \"mode\": \"ArmedTest\", "
	" \"command\": \"SelfTest\", "
	" \"originMode\": \"SelfTest\", "
	" \"throttle\": 42, "
	" \"boat\": \"WaypointNavigation\", "
	" \"orientation\": { \"roll\": 0, \"pitch\": -1.5, \"yaw\": 27.75 }, "
	" \"faultString\": \"\", "

	" \"headingTarget\": 230, "
	" \"internalVoltage\": 11.75, "
	" \"batteryVoltage\": 11.5, "
	" \"motorVoltage\": 10.0, "

	" \"enbButton\": false, "
	" \"stopButton\": false, "

	" \"timeSinceLastPacket\": 98345701, "
	" \"timeOfLastPacket\": 843675, "
	" \"timeOfLastBoneHB\": 9348765, "
	" \"timeOfLastShoreHB\": 987435, "

	" \"rudder\": 0.125, "
	" \"rudderRaw\": 122, "
	" \"internalVoltageRaw\": 254, "
	" \"motorVoltageRaw\": 200, "
	" \"motorCurrent\": 0.0625, "
	" \"motorCurrentRaw\": 12, "

	" \"Kp\": 0.75, \"Ki\": 0.03125, \"Kd\": -0.5, "

	" \"magX\": 1, \"magY\": 2, \"magZ\": 3, "

	" \"accX\": -4.5, \"accY\": -5, \"accZ\": -0.5, "

	" \"gyroX\": 4.5, \"gyroY\": 5.5, \"gyroZ\": 6.5, "

	" \"horn\": true, "

	" \"motorDirRly\": false, "
	" \"motorWhtRly\": true, "
	" \"motorYlwRly\": false, "
	" \"motorRedRly\": false, "
	" \"motorRedWhtRly\": true, "
	" \"motorRedYlwRly\": false, "

	" \"servoPower\": true, "

	" \"startStopTime\": 983457, "
	" \"startModeTime\": 834759 "
	" }";

	json_t *sample = json_loads(s, 0, NULL);
	ASSERT_TRUE(sample);

	arduinoStateClass a;
	EXPECT_TRUE(a.parse(sample, false));

	json_t *roundtrip = a.pack(false);
	EXPECT_JSON_EQ(sample, roundtrip);
	json_decref(roundtrip);

	json_decref(sample);
}


