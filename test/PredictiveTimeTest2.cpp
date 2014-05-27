/*
 * PredictiveTimeTest2.cpp
 *
 *  Created on: Feb 3, 2014
 *      Author: stieg
 */

#include "PredictiveTimeTest2.h"

#include <stdlib.h>

#include "dateTime.h"
#include "geopoint.h"
#include "gps.testing.h"
#include "modp_atonum.h"
#include "predictive_timer_2.h"
#include "loggerConfig.h"

#define FILE_PREFIX string("test/")

using std::ifstream;
using std::ios;
using std::istreambuf_iterator;
using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( PredictiveTimeTest2 );

PredictiveTimeTest2::PredictiveTimeTest2() {}

void PredictiveTimeTest2::setUp() {
	initGPS();
	resetPredictiveTimer();
}

void PredictiveTimeTest2::tearDown() {}

vector<string> & PredictiveTimeTest2::split(string &s, char delim, vector<string> &elems) {
    std::stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> PredictiveTimeTest2::split(string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

// And I am totally lazy and just copying because I want this code done.
string PredictiveTimeTest2::readFile(string filename) {
	ifstream t(filename.c_str());
	if (!t.is_open()) {
		t.open(string(FILE_PREFIX + filename).c_str());

	}
	if (!t.is_open()) {
		throw("Can not find file " + filename);
	}
	string str;

	t.seekg(0, ios::end);
	int length = t.tellg();
	str.reserve(length);
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	return str;
}

void PredictiveTimeTest2::testPredictedTimeGpsFeed() {
	string log = readFile("predictive_time_test_lap.log");

	std::istringstream iss(log);


	TrackConfig *trackCfg  = 	&(getWorkingLoggerConfig()->TrackConfigs);
	trackCfg->track.circuit.startFinish.latitude = 47.806934;
	trackCfg->track.circuit.startFinish.longitude = -122.341150;
	trackCfg->radius = 0.0004;
	setGPSQuality (GPS_QUALITY_DIFFERENTIAL);

	int lineNo = 0;
	string line;
    int lapCount = getLapCount();

	while (std::getline(iss, line)) {
		lineNo++;
		vector <string> values = split(line, ',');

		string latitudeRaw = values[5];
		string longitudeRaw = values[6];
		string speedRaw = values[7];
		string timeRaw = values[8];

		timeRaw = "0" + timeRaw;

		if (values[0][0] != '#' && latitudeRaw.size() > 0
				&& longitudeRaw.size() > 0 && speedRaw.size() > 0
				&& timeRaw.size() > 0) {
			//printf("%s", line.c_str());
			float lat = modp_atof(latitudeRaw.c_str());
			float lon = modp_atof(longitudeRaw.c_str());
			float speed = modp_atof(speedRaw.c_str());

         const char *utcTimeStr = timeRaw.c_str();
         float utcTime = modp_atof(utcTimeStr);
         double secondsSinceMidnight = calculateSecondsSinceMidnight(utcTimeStr);

         DateTime dt;
         dt.partialYear = 14;
         dt.month = 5;
         dt.day = 3;
         dt.hour = (int8_t) atoiOffsetLenSafe(utcTimeStr, 0, 2);
         dt.minute = (int8_t) atoiOffsetLenSafe(utcTimeStr, 2, 2);
         dt.second = (int8_t) atoiOffsetLenSafe(utcTimeStr, 4, 2);
         dt.millisecond = (int16_t) atoiOffsetLenSafe(utcTimeStr, 7, 3);
         updateFullDateTime(dt);
         updateMillisSinceEpoch(dt);

         const unsigned long millis = getMillisSinceEpoch();
         printf("---\n");
         printf("DateTime - YY MM DD HH MM SS.mmm : %02d %02d %02d %02d %02d %02d.%03d\n", dt.partialYear,
               dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.millisecond);
         printf("lat = %f : lon = %f : speed = %f : UTC_Str = \"%s\" : time = %f : millis = %lu\n",
               lat, lon, speed, utcTimeStr, utcTime, millis);

         setGPSSpeed(speed);
         setUTCTime(utcTime);
         updatePosition(lat, lon);
         updateSecondsSinceMidnight(secondsSinceMidnight);
         onLocationUpdated();

         GeoPoint gp;
         populateGeoPoint(&gp);
         const float secondsSinceFirstFix = getSecondsSinceFirstFix();
         float predTime = getPredictedTime(gp, secondsSinceFirstFix);
         printf("Lap #%d - Predicted Time: %f\n", getLapCount(), predTime);
		}
	}
}