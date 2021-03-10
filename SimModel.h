#pragma once

#include <windows.h>
#include <SimConnect.h>
#include <SimModelBehavior.cpp>

/**
* Sim Model Strategy Pattern
*
* Definition of strategy pattern to be referenced for various sim models
*
* @author muppetlabs@fswindowseat.com
*/
class SimModel
{

public:
	SimModelBehavior* behavior;

	/**
	* Assigns the sim model's behaviour
	* 
	* @param	simModelBehaviour	Type SimModelBehaviour
	*/
	void setSimModelBehavior(SimModelBehavior* behavior);

	/**
	* Initialize all of the processing parameters
	*
	* @param	void
	* @return   void
	*/
	void initProcessing();

	/**
	* Closes the separate SimConnect call for the sim model's specific functions
	*
	* @param	void
	* @return   void
	*/
	void closeProcessing();

	/**
	* Return the sim model's const vars
	*
	* @param	void
	* @return	float
	*/
	float getAcCntrAlt();

	float getAcFixedPitch();

	float getAcFixedTrim();

	float getAcEngineIdle();

	/**
	* Calculates and sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @return   void
	*/
	void setFuel(float timeStmp, float flightTime);

	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @return   void
	*/
	void startAPUProc(float timeStmp, float startTime);

	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @return   void
	*/
	void startEngineProc(float timeStmp, float startTime);

	/**
	* Runs through the stop engine procedure
	*
	* @param	void
	* @return   void
	*/
	void stopEngineProc();

	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @return   double			Throttle value
	*/
	double convertThrottle(double inputThrottle);

	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @return	void
	*/
	void setThrottle(double throttleVal);

	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	gear			Gear setting
	* @return	void
	*/
	void performSetProp(double throttleVal, int fltPhase, double gear);

	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser setting 0 = Off / 1 = On
	* @return	void
	*/
	void setReverser(int reverserSet);

	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @return	void
	*/
	void setAileron(double degrees);

	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @return	void
	*/
	void setElevator(double degrees);

	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @return	void
	*/
	void setElevatorTrim(double degrees);

	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @return	void
	*/
	void setRudder(double degrees);

	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps stage setting 1 to 5
	* @return	void
	*/
	void setFlaps(double stage);

	/**
	* Sets the aircraft's speed brakes / spoilers
	*
	* @param	spoilerSet		Spoiler setting 0 = Down / 1 = Up
	* @return	void
	*/
	void setSpoiler(double spoilerSet);

	/**
	* Sets the aircraft's nose wheel deflection
	*
	* @param	noseWheelDegrees	Nose wheel deflection in degrees
	* @return	void
	*/
	void setNoseWheel(double noseWheelDegrees);

	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear setting 1 = Down / 0 = Up
	* @return	void
	*/
	void setGear(double gearSet);

	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break pressure
	* @return	void
	*/
	void setBrakes(double brakeSet);

	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setNavLights(int lightSwitch);

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setBeaconLights(int lightSwitch);

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setStrobeLights(int lightSwitch);

	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setTaxiLights(int lightSwitch);

	/**
	* Sets the aircraft's runway turnoff lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setRwyTurnoffLights(int lightSwitch);

	/**
	* This method sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setWingLights(int lightSwitch);

	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void setLandingLights(int lightSwitch);

	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign setting 1 = On / 0 = Off
	* @return	void
	*/
	void setCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt);

	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @return	void
	*/
	void setAltimeter(double altitude);

	/**
	* Performs specific/unique model functions
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	altitude		Aircraft's current altitude
	* @param	fltPhase		FTDIS flight phase
	* @return	void
	*/
	void performModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase);

	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @return	void
	*/
	void getCamView(float camCoords[], char camSide, char camPos, int camVar);
};
