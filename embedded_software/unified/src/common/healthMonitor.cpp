/******************************************************************************
 * Hackerboat health monitor module
 * healthMonitor.cpp
 * This module monitors the health of the boat
 *
 * See the Hackerboat documentation for more details
 * Written by Pierce Nichols, Sep 2016
 * 
 * Version 0.1: First alpha
 *
 ******************************************************************************/

#include <chrono>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include "hackerboatRoot.hpp"
#include "hal/config.h"
#include "hal/adcInput.hpp"
#include "healthMonitor.hpp"
#include "easylogging++.h"

using namespace rapidjson;
using namespace std;

bool HealthMonitor::readHealth () {
	// attempt to grab a lock over the adc input in order to make sure we're copying consistent data
	if (!_adc->lock.try_lock_for(ADC_LOCK_TIMEOUT)) {
		return false;
	}
	std::map<std::string, double> data = _adc->getScaledValues();	
	this->recordTime = _adc->getLastInputTime();
	_adc->lock.unlock();
	// End lock block
	
	this->valid 		= true;
	this->servoCurrent 	= data["servo_i"];
	this->batteryMon	= data["battery_mon"];
	this->mainVoltage	= data["main_v"];
	this->mainCurrent 	= data["main_i"];
	this->chargeVoltage	= data["charge_v"];
	this->chargeCurrent = data["charge_i"];
	this->motorVoltage	= data["mot_v"];
	this->motorCurrent	= data["mot_i"];
	this->rcRssi		= (int)data["rc_rssi_input"];
	this->cellRssi		= 0;	// Data fetch not yet implemented
	this->wifiRssi		= 0;	// Data fetch not yet implemented
	LOG_EVERY_N(100, DEBUG) << "Pulling health information: " << this;
	return true;
}

bool HealthMonitor::parse (Value& input) {
	string recordTimeIn;
	bool result = true;

	result &= GetVar("recordTime", recordTimeIn, input);
	result &= HackerboatState::parseTime(recordTimeIn, this->recordTime);
	result &= GetVar("servoCurrent", this->servoCurrent, input);
	result &= GetVar("batteryMon", this->batteryMon, input);
	result &= GetVar("mainVoltage", this->mainVoltage, input);
	result &= GetVar("mainCurrent", this->mainCurrent, input);
	result &= GetVar("chargeVoltage", this->chargeVoltage, input);
	result &= GetVar("chargeCurrent", this->chargeVoltage, input);
	result &= GetVar("motorVoltage", this->motorVoltage, input);
	result &= GetVar("motorCurrent", this->motorCurrent, input);
	result &= GetVar("rcRssi", this->rcRssi, input);
	result &= GetVar("cellRssi", this->cellRssi, input);
	result &= GetVar("wifiRssi", this->wifiRssi, input);
	
	valid = result;
	return result;
}

Value HealthMonitor::pack () const {
	Value o;
	int packResult = 0;
	packResult += PutVar("recordTime", HackerboatState::packTime(this->recordTime), o);
	packResult += PutVar("servoCurrent", this->servoCurrent, o);
	packResult += PutVar("batteryMon", this->batteryMon, o);
	packResult += PutVar("mainVoltage", this->mainVoltage, o);
	packResult += PutVar("mainCurrent", this->mainCurrent, o);
	packResult += PutVar("chargeVoltage", this->chargeVoltage, o);
	packResult += PutVar("chargeCurrent", this->chargeCurrent, o);
	packResult += PutVar("motorVoltage", this->motorVoltage, o);
	packResult += PutVar("motorCurrent", this->motorCurrent, o);
	packResult += PutVar("rcRssi", this->rcRssi, o);
	packResult += PutVar("cellRssi", this->cellRssi, o);
	packResult += PutVar("wifiRssi", this->wifiRssi, o);
	
	return o;
	
}

HackerboatStateStorage& HealthMonitor::storage() {
	if (!healthStorage) {
		healthStorage = new HackerboatStateStorage(HackerboatStateStorage::databaseConnection(HEALTH_DB_FILE), 
							"HEALTH_MON",
							{ { "recordTime", "TEXT" },
							  { "servoCurrent", "REAL" },
							  { "batteryMon", "REAL" },
							  { "mainVoltage", "REAL" },
							  { "mainCurrent", "REAL" },
							  { "chargeVoltage", "REAL" },
							  { "chargeCurrent", "REAL" },
							  { "motorVoltage", "REAL" },
							  { "motorCurrent", "REAL" },
							  { "rcRssi", "INTEGER" },
							  { "cellRssi", "INTEGER" },
							  { "wifiRssi", "INTEGER" } });
		healthStorage->createTable();						 
	}
	return *healthStorage;
	
}

bool HealthMonitor::fillRow(SQLiteParameterSlice row) const {
	row.assertWidth(12);

	row.bind(0, HackerboatState::packTime(recordTime));
	row.bind(1, servoCurrent);
	row.bind(2, batteryMon);
	row.bind(3, mainVoltage);
	row.bind(4, mainCurrent);
	row.bind(5, chargeVoltage);
	row.bind(6, chargeCurrent);
	row.bind(7, motorVoltage);
	row.bind(8, motorCurrent);
	row.bind(9, rcRssi);
	row.bind(10, cellRssi);
	row.bind(11, wifiRssi);
	
	return true;
}

bool HealthMonitor::readFromRow(SQLiteRowReference row, sequence seq) {
	bool result = true;
	_sequenceNum = seq;
	row.assertWidth(12);

	std::string str = row.string_field(0);
	result &= HackerboatState::parseTime(str, this->recordTime);
	this->servoCurrent = row.double_field(1);
	this->batteryMon = row.double_field(2);
	this->mainVoltage = row.double_field(3);
	this->mainCurrent = row.double_field(4);
	this->chargeVoltage = row.double_field(5);
	this->chargeCurrent = row.double_field(6);
	this->motorVoltage = row.double_field(7);
	this->motorCurrent = row.double_field(8);
	this->rcRssi = row.int_field(9);
	this->cellRssi = row.int_field(10);
	this->wifiRssi = row.int_field(11);
	
	return result;
}