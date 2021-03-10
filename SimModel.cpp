#include <SimModel.h>

/**
* Assigns the sim model's behaviour
*/
void SimModel::setSimModelBehavior(SimModelBehavior* behavior) {
	this->behavior = behavior;
}


/**
* Initialize all of the processing parameters
*/
void SimModel::initProcessing() {
	this->behavior->performInitProcessing();
}


/**
* Closes the separate SimConnect call for the sim model's specific functions
*/
void SimModel::closeProcessing() {
	this->behavior->performCloseProcessing();
}


/**
* Return the sim model's const vars
*/
float SimModel::getAcCntrAlt() {
	return this->behavior->acCntrAlt;
}

float SimModel::getAcFixedPitch() {
	return this->behavior->acFixedPitch;
}

float SimModel::getAcFixedTrim() {
	return this->behavior->acFixedTrim;
}

float SimModel::getAcEngineIdle() {
	return this->behavior->acEngineIdle;
}


/**
* Calculates and sets the aircraft's fuel parameters
*/
void SimModel::setFuel(float timeStmp, float flightTime) {
	this->behavior->performSetFuel(timeStmp, flightTime);
}


/**
* Runs through the aicraft's start APU procedure
*/
void SimModel::startAPUProc(float timeStmp, float startTime) {
	this->behavior->performStartAPUProc(timeStmp, startTime);
}


/**
* Runs through the aircraft's startup procedure
*/
void SimModel::startEngineProc(float timeStmp, float startTime) {
	this->behavior->performStartEngineProc(timeStmp, startTime);
}


/**
* Runs through the stop engine procedure
*/
void SimModel::stopEngineProc() {
	this->behavior->performStopEngineProc();
}


/**
* Converts the throttle value specified in the FTDIS input .fd file to the
* corresponding value within the aircraft's throttle range
*/
double SimModel::convertThrottle(double inputThrottle) {
	return this->behavior->performConvertThrottle(inputThrottle);
}


/**
* Sets the aircraft's throttle
*/
void SimModel::setThrottle(double throttleVal) {
	this->behavior->performSetThrottle(throttleVal);
}


/**
* Sets the prop's feather, based on input throttle, flight phase, etc.
* Used for props only
*/
void SimModel::performSetProp(double throttleVal, int fltPhase, double gear) {
	this->behavior->performSetProp(throttleVal, fltPhase, gear);
}


/**
* Sets the aircraft's reversers
*/
void SimModel::setReverser(int reverserSet) {
	this->behavior->performSetReverser(reverserSet);
}


/**
* Sets the aircraft's ailerons
*/
void SimModel::setAileron(double degrees) {
	this->behavior->performSetAileron(degrees);
}


/**
* Sets the aircraft's elevator
*/
void SimModel::setElevator(double degrees) {
	this->behavior->performSetElevator(degrees);
}


/**
* Sets the aircraft's elevator trim
*/
void SimModel::setElevatorTrim(double degrees) {
	this->behavior->performSetElevatorTrim(degrees);
}


/**
* Sets the aircraft's rudder
*/
void SimModel::setRudder(double degrees) {
	this->behavior->performSetRudder(degrees);
}


/**
* Sets the aircrafts's flaps
*/
void SimModel::setFlaps(double stage) {
	this->behavior->performSetFlaps(stage);
}


/**
* Sets the aircraft's speed brakes / spoilers
*/
void SimModel::setSpoiler(double spoilerSet) {
	this->behavior->performSetSpoiler(spoilerSet);
}


/**
* Sets the aircraft's nose wheel deflection
*/
void SimModel::setNoseWheel(double noseWheelDegrees) {
	this->behavior->performSetNoseWheel(noseWheelDegrees);
}


/**
* Sets the aircrafts's gear
*/
void SimModel::setGear(double gearSet) {
	this->behavior->performSetGear(gearSet);
}


/**
* Sets the aircraft's brakes
*/
void SimModel::setBrakes(double brakeSet) {
	this->behavior->performSetBrakes(brakeSet);
}


/**
* Sets the aircraft's navigation lights
*/
void SimModel::setNavLights(int lightSwitch) {
	this->behavior->performSetNavLights(lightSwitch);
}


/**
* Sets the aircraft's beacon lights
*/
void SimModel::setBeaconLights(int lightSwitch) {
	this->behavior->performSetBeaconLights(lightSwitch);
}


/**
* Sets the aircraft's beacon lights
*/
void SimModel::setStrobeLights(int lightSwitch) {
	this->behavior->performSetStrobeLights(lightSwitch);
}


/**
* Sets the aircraft's taxi lights
*/
void SimModel::setTaxiLights(int lightSwitch) {
	this->behavior->performSetTaxiLights(lightSwitch);
}


/**
* Sets the aircraft's runway turnoff lights
*/
void SimModel::setRwyTurnoffLights(int lightSwitch) {
	this->behavior->performSetRwyTurnoffLights(lightSwitch);
}


/**
* Sets the aircraft's wing lights
*/
void SimModel::setWingLights(int lightSwitch) {
	this->behavior->performSetWingLights(lightSwitch);
}


/**
* Sets the aircraft's landing lights
*/
void SimModel::setLandingLights(int lightSwitch) {
	this->behavior->performSetLandingLights(lightSwitch);
}


/**
* Sets the aircraft's cabin signs, as per the phase of flight
*/
void SimModel::setCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt) {
	this->behavior->performSetCabinSigns(timeStmp, signSeatbelt, signNoSmoke, signCabinAtt);
}


/**
* Sets the aircraft's altitude to the current barometric pressure
*/
void SimModel::setAltimeter(double altitude) {
	this->behavior->performSetAltimeter(altitude);
}


/**
* Performs specific/unique model functions
*/
void SimModel::performModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase) {
	this->behavior->performModelFunction(timeStmp, flightTime, altitude, fltPhase);
}


/**
* Defines and returns the model's camera view coordinates
*/
void SimModel::getCamView(float camCoords[], char camSide, char camPos, int camVar) {
	this->behavior->getCamView(camCoords, camSide, camPos, camVar);
}