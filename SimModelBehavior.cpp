#pragma once
#include <windows.h>
#include <SimConnect.h>

/**
* FTDIS Sim Model Behaviour
*
* Definition of model behaviour for strategy pattern
*
* @author muppetlabs@fswindowseat.com
*/
class SimModelBehavior
{

public:

	float acCntrAlt = 0, acFixedPitch = 0, acFixedTrim = 0, acEngineIdle = 0, acEngineMax = 0;

	/**
	* Sets up a separate SimConnect call for the sim model's specific functions
	* and initializes all of the processing parameters
	*
	* @param	void
	* @return   void
	*/
	virtual void performInitProcessing() = 0;

	/**
	* Closes the separate SimConnect call for the sim model's specific functions
	*
	* @param	void
	* @return   void
	*/
	virtual void performCloseProcessing() = 0;

	/**
	* Calculates and sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @return   void
	*/
	virtual void performSetFuel(float timeStmp, float flightTime) = 0;

	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @return   void
	*/
	virtual void performStartAPUProc(float timeStmp, float startTime) = 0;

	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @return   void
	*/
	virtual void performStartEngineProc(float timeStmp, float startTime) = 0;

	/**
	* Runs through the stop engine procedure
	*
	* @param	void
	* @return   void
	*/
	virtual void performStopEngineProc() = 0;

	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @return   double			Throttle value
	*/
	virtual double performConvertThrottle(double inputThrottle) = 0;

	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @return	void
	*/
	virtual void performSetThrottle(double throttleVal) = 0;

	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	gear			Gear setting
	* @return	void
	*/
	virtual void performSetProp(double throttleVal, int fltPhase, double gear) = 0;

	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser setting 0 = Off / 1 = On
	* @return	void
	*/
	virtual void performSetReverser(int reverserSet) = 0;

	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @return	void
	*/
	virtual void performSetAileron(double degrees) = 0;

	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @return	void
	*/
	virtual void performSetElevator(double degrees) = 0;

	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @return	void
	*/
	virtual void performSetElevatorTrim(double degrees) = 0;

	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @return	void
	*/
	virtual void performSetRudder(double degrees) = 0;

	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps stage setting 1 to 5
	* @return	void
	*/
	virtual void performSetFlaps(double stage) = 0;

	/**
	* Sets the aircraft's nose wheel
	*
	* @param	degrees		Nose wheel deflection
	* @return	void
	*/
	virtual void performSetNoseWheel(double degrees) = 0;

	/**
	* Sets the aircraft's speed brakes / spoilers
	*
	* @param	spoilerSet		Spoiler setting 0 = Down / 1 = Up
	* @return	void
	*/
	virtual void performSetSpoiler(double spoilerSet) = 0;

	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear setting 1 = Down / 0 = Up
	* @return	void
	*/
	virtual void performSetGear(double gearSet) = 0;

	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetBrakes(double brakeSet) = 0;

	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetNavLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetBeaconLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetStrobeLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetTaxiLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's runway turnoff lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetRwyTurnoffLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetWingLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetLandingLights(int lightSwitch) = 0;

	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign setting 1 = On / 0 = Off
	* @return	void
	*/
	virtual void performSetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt) = 0;

	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @return	void
	*/
	virtual void performSetAltimeter(double altitude) = 0;

	/**
	* Performs specific/unique model functions
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	altitude		Aircraft's current altitude
	* @param	fltPhase		FTDIS flight phase
	* @return	void
	*/
	virtual void performModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase) = 0;

	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @return	void
	*/
	virtual void getCamView(float camCoords[], char camSide, char camPos, int camVar) = 0;
};
