#ifndef SimModelBehavior_H
#define SimModelBehavior_H
#define _WINSOCKAPI_ 

#include <windows.h>
#include "PosDataSet.h"
#include "SimDataSet.h"
#include "SysDataSet.h"
#include "Track.h"
#include "SimConnect.h"

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
	
	/**
	* Sets up a dedicated SimConnect requests for the sim model's specific functions
	* and initializes all of the corresponding parameters for processing
	*
	* @param	simObjectId		DWORD
	* @return   void
	*/
	virtual void PerformInitSimObject(DWORD simObjectId) = 0;

	/**
	* Creates new Ai object and defines initial position and attitude
	* Initial position is defined as first record in data track
	*
	* @param	track	Track
	* @return   void
	*/
	virtual void PerformCreateAiSimObject(Track* track) = 0;

	/**
	* Removes Ai object from sim
	*
	* @param	void
	* @return   void
	*/
	virtual void PerformRemoveAiSimObject() = 0;

	/**
	* Returns the sim objects's SimConnect ID
	*
	* @param	void
	* @return	simObjectOD		DWORD
	*/
	virtual DWORD PerformGetSimObjectId() = 0;

	/**
	* Assigns the sim object's SimConnect request ID
	*
	* @param	request ID		int
	* @return   void
	*/
	virtual void PerformSetSimRequestId(int simRequestId) = 0;

	/**
	* Returns the sim object's SimConnect request ID
	*
	* @param	void
	* @retun	request ID		int
	*/
	virtual int PerformGetSimRequestId() = 0;

	/**
	* Returns the sim model's fixed trim constant
	*
	* @param	void
	* @param	Model fixed trim	float
	*/
	virtual float PerformGetAcFixedTrim() = 0;

	/**
	* Returns the sim model's fixed trim constant
	*
	* @param	void
	* @param	N1 engine idle		float
	*/
	virtual float PerformGetAcEngineIdle() = 0;

	/**
	* Set the aircraft's position and attitude
	*
	* @param	posDataSet		Position data Set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformSetPosition(PosDataSet posDataSet, Track::FileType fileType) = 0;

	/**
	* Set the aircraft's system vars
	*
	* @param	sysDataSet		system vars data set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformSetSysData(SysDataSet sysDataSet, Track::FileType fileType) = 0;

	/**
	* Set the aircraft's simulation data vars
	*
	* @param	simDataSet		simulation vars data set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformSetSimData(SimDataSet simDataSet, Track::FileType fileType) = 0;

	/**
	* Calculates and sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformSetFuel(float timeStmp, float flightTime, Track::FileType fileType) = 0;

	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformStartAPUProc(float timeStmp, float startTime, Track::FileType fileType) = 0;

	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformStartEngineProc(float timeStmp, float startTime, Track::FileType fileType) = 0;

	/**
	* Runs through the stop engine procedure
	*
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	virtual void PerformStopEngineProc(Track::FileType fileType) = 0;

	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   double			Throttle value
	*/
	virtual double PerformConvertThrottle(double inputThrottle, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetThrottle(double throttleVal, Track::FileType fileType) = 0;

	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetProp(double throttleVal, int fltPhase, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser setting 0 = Off / 1 = On
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetReverser(int reverserSet, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetAileron(double degrees, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetElevator(double degrees, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetElevatorTrim(double degrees, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetRudder(double degrees, Track::FileType fileType) = 0;

	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps stage setting 1 to 5
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetFlaps(double stage, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's nose wheel
	*
	* @param	degrees		Nose wheel deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetNoseWheel(double degrees, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's speed brakes / spoilers
	*
	* @param	spoilerSet		Spoiler setting 0 = Down / 1 = Up
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetSpoiler(double spoilerSet, Track::FileType fileType) = 0;

	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear setting 1 = Down / 0 = Up
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetGear(double gearSet, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break setting 1 = On / 0 = Off
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetBrakes(double brakeSet, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetNavLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's logolights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetLogoLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetBeaconLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetStrobeLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetTaxiLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetWingLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetLandingLights(int lightSwitch, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign setting 1 = On / 0 = Off
	* @param	fileType			Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformSetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt, Track::FileType fileType) = 0;

	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @return	void
	*/
	virtual void PerformSetAltimeter(double altitude, Track::FileType fileType) = 0;

	/**
	* Performs specific/unique model functions
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	altitude		Aircraft's current altitude
	* @param	fltPhase		FTDIS flight phase
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	virtual void PerformModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase, Track::FileType fileType) = 0;

	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @return	void
	*/
	virtual void GetCamView(float camCoords[], char camSide, char camPos, int camVar) = 0;
};
#endif