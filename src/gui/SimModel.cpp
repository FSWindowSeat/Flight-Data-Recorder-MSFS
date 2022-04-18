#include "SimModel.h"

/**
* Assigns the sim model's behaviour
*/
void SimModel::SetSimModelBehavior(SimModelBehavior* behavior) {
	this->behavior = behavior;
}


/**
* Sets up a dedicated SimConnect requests for the sim model's specific functions
* and initializes all of the corresponding parameters for processing
*/
void SimModel::InitSimObject(DWORD simObjectId) {
	this->behavior->PerformInitSimObject(simObjectId);
}


/**
* Creates new Ai object and defines initial position and attitude
* Initial position is defined as first record in data track
*/
void SimModel::CreateAiSimObject(Track* track) {
	this->behavior->PerformCreateAiSimObject(track);
}

/**
* Removes Ai object from sim
*/
void SimModel::RemoveAiSimObject() {
	this->behavior->PerformRemoveAiSimObject();
}

/**
* Returns the sim objects's SimConnect ID
*/
DWORD SimModel::GetSimObjectId() {
	return this->behavior->PerformGetSimObjectId();
}


/**
* Assigns the sim object's SimConnect request ID
*/
void SimModel::SetSimRequestId(int simRequestId) {
	this->behavior->PerformSetSimRequestId(simRequestId);
}


/**
* Returns the sim object's SimConnect request ID
*/
int SimModel::GetSimRequestId() {
	return this->behavior->PerformGetSimRequestId();
}


/**
* Returns the sim model's fixed trim constant
*/
float SimModel::GetAcFixedTrim() {
	return this->behavior->PerformGetAcFixedTrim();
}


/**
* Returns the sim model's engine idel trim constant
*/
float SimModel::GetAcEngineIdle() {
	return this->behavior->PerformGetAcEngineIdle();
}


/**
* Set the aircraft's position and attitude
*/
void SimModel::SetPosition(PosDataSet posDataSet, Track::FileType fileType) {
	return this->behavior->PerformSetPosition(posDataSet, fileType);
}


/**
* Set the aircraft's system
*/
void SimModel::SetSysData(SysDataSet sysDataSet, Track::FileType fileType) {
	return this->behavior->PerformSetSysData(sysDataSet, fileType);
}


/**
* Set the aircraft's simulation vars
*/
void SimModel::SetSimData(SimDataSet simDataSet, Track::FileType fileType) {
	return this->behavior->PerformSetSimData(simDataSet, fileType);
}


/**
* Calculates and sets the aircraft's fuel parameters
*/
void SimModel::SetFuel(float timeStmp, float flightTime, Track::FileType fileType) {
	this->behavior->PerformSetFuel(timeStmp, flightTime, fileType);
}


/**
* Runs through the aicraft's start APU procedure
*/
void SimModel::StartAPUProc(float timeStmp, float startTime, Track::FileType fileType) {
	this->behavior->PerformStartAPUProc(timeStmp, startTime, fileType);
}


/**
* Runs through the aircraft's startup procedure
*/
void SimModel::StartEngineProc(float timeStmp, float startTime, Track::FileType fileType) {
	this->behavior->PerformStartEngineProc(timeStmp, startTime, fileType);
}


/**
* Runs through the stop engine procedure
*/
void SimModel::StopEngineProc(Track::FileType fileType) {
	this->behavior->PerformStopEngineProc(fileType);
}


/**
* Converts the throttle value specified in the FTDIS input .fd file to the
* corresponding value within the aircraft's throttle range
*/
double SimModel::ConvertThrottle(double inputThrottle, Track::FileType fileType) {
	return this->behavior->PerformConvertThrottle(inputThrottle, fileType);
}


/**
* Sets the aircraft's throttle
*/
void SimModel::SetThrottle(double throttleVal, Track::FileType fileType) {
	this->behavior->PerformSetThrottle(throttleVal, fileType);
}


/**
* Sets the prop's feather, based on input throttle, flight phase, etc.
* Used for props only
*/
void SimModel::SetPropPos(double throttleVal, int fltPhase, Track::FileType fileType) {
	this->behavior->PerformSetProp(throttleVal, fltPhase, fileType);
}


/**
* Sets the aircraft's reversers
*/
void SimModel::SetReverser(int reverserSet, Track::FileType fileType) {
	this->behavior->PerformSetReverser(reverserSet, fileType);
}


/**
* Sets the aircraft's ailerons
*/
void SimModel::SetAileron(double degrees, Track::FileType fileType) {
	this->behavior->PerformSetAileron(degrees, fileType);
}


/**
* Sets the aircraft's elevator
*/
void SimModel::SetElevator(double degrees, Track::FileType fileType) {
	this->behavior->PerformSetElevator(degrees, fileType);
}


/**
* Sets the aircraft's elevator trim
*/
void SimModel::SetElevatorTrim(double degrees, Track::FileType fileType) {
	this->behavior->PerformSetElevatorTrim(degrees, fileType);
}


/**
* Sets the aircraft's rudder
*/
void SimModel::SetRudder(double degrees, Track::FileType fileType) {
	this->behavior->PerformSetRudder(degrees, fileType);
}


/**
* Sets the aircrafts's flaps
*/
void SimModel::SetFlaps(double stage, Track::FileType fileType) {
	this->behavior->PerformSetFlaps(stage, fileType);
}


/**
* Sets the aircraft's speed brakes / spoilers
*/
void SimModel::SetSpoiler(double spoilerSet, Track::FileType fileType) {
	this->behavior->PerformSetSpoiler(spoilerSet, fileType);
}


/**
* Sets the aircraft's nose wheel deflection
*/
void SimModel::SetNoseWheel(double noseWheelDegrees, Track::FileType fileType) {
	this->behavior->PerformSetNoseWheel(noseWheelDegrees, fileType);
}


/**
* Sets the aircrafts's gear
*/
void SimModel::SetGear(double gearSet, Track::FileType fileType) {
	this->behavior->PerformSetGear(gearSet, fileType);
}


/**
* Sets the aircraft's brakes
*/
void SimModel::SetBrakes(double brakeSet, Track::FileType fileType) {
	this->behavior->PerformSetBrakes(brakeSet, fileType);
}


/**
* Sets the aircraft's navigation lights
*/
void SimModel::SetNavLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetNavLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's logo lights
*/
void SimModel::SetLogoLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetLogoLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's beacon lights
*/
void SimModel::SetBeaconLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetBeaconLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's beacon lights
*/
void SimModel::SetStrobeLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetStrobeLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's taxi lights
*/
void SimModel::SetTaxiLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetTaxiLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's wing lights
*/
void SimModel::SetWingLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetWingLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's landing lights
*/
void SimModel::SetLandingLights(int lightSwitch, Track::FileType fileType) {
	this->behavior->PerformSetLandingLights(lightSwitch, fileType);
}


/**
* Sets the aircraft's cabin signs, as per the phase of flight
*/
void SimModel::SetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt, Track::FileType fileType) {
	this->behavior->PerformSetCabinSigns(timeStmp, signSeatbelt, signNoSmoke, signCabinAtt, fileType);
}


/**
* Sets the aircraft's altitude to the current barometric pressure
*/
void SimModel::SetAltimeter(double altitude, Track::FileType fileType) {
	this->behavior->PerformSetAltimeter(altitude, fileType);
}


/**
* Performs specific/unique model functions
*/
void SimModel::PerformModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase, Track::FileType fileType) {
	this->behavior->PerformModelFunction(timeStmp, flightTime, altitude, fltPhase, fileType);
}


/**
* Defines and returns the model's camera view coordinates
*/
void SimModel::GetCamView(float camCoords[], char camSide, char camPos, int camVar) {
	this->behavior->GetCamView(camCoords, camSide, camPos, camVar);
}