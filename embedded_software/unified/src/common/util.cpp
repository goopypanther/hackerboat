/******************************************************************************
 * Hackerboat Beaglebone utilities
 * util.cpp
 * This is a set of generic utilites
 * see the Hackerboat documentation for more details
 *
 * Written by Pierce Nichols, Apr 2017
 *
 * Version 0.1: First alpha
 *
 ******************************************************************************/
 
#include "util.hpp"
#include "hackerboatRoot.hpp"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

using namespace rapidjson;
 
 // initialization of static class members
 Args* Args::_instance = new Args();
 Document HackerboatState::root;
