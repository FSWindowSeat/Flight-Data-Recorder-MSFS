#ifndef SimModel_H
#define SimModel_H
#define _WINSOCKAPI_ 

#include <windows.h>
#include "PosDataSet.h"
#include "SimDataSet.h"
#include "SysDataSet.h"
#include "Track.h"
#include "SimConnect.h"
#include "SimModelBehavior.cpp"

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
	* @param	simModelBehaviour	SimModelBehaviour
	* @return   void
	*/
	void SetSimModelBehavior(SimModelBehavior* behavior);

	/**
	* Sets up a dedicated SimConnect requests for the sim model's specific functions
	* and initializes all of the corresponding parameters for processing
	*
	* @param	simObjectId		DWORD
	* @return   void
	*/
	void InitSimObject(DWORD simObjectId);

	/**
	* Creates new Ai object and defines initial position and attitude
	* Initial position is defined as first record in data track
	*
	* @param	track	Track
	* @return   void
	*/
	void CreateAiSimObject(Track* track);

	/**
	* Removes Ai object from sim
	*
	* @param	void
	* @return   void
	*/
	void RemoveAiSimObject();

	/**
	* Returns the sim object's SimConnect ID
	*
	* @param	void
	* @param	simObject ID	DWORD
	*/
	DWORD GetSimObjectId();

	/**
	* Assigns the sim object's SimConnect request ID
	*
	* @param	request ID		int
	* @return   void
	*/
	void SetSimRequestId(int simRequestId);

	/**
	* Returns the sim object's SimConnect request ID
	*
	* @param	void
	* @retun	request ID		int
	*/
	int GetSimRequestId();

	/**
	* Return the sim model's const vars
	*
	* @param	void
	* @return	float
	*/
	float GetAcFixedTrim();
	float GetAcEngineIdle();


	/**
	* Set the aircraft's position and attitude
	* 
	* @param	posDataSet		Position data Set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void SetPosition(PosDataSet posDataSet, Track::FileType fileType);

	/**
	* Set the aircraft's system data vars
	*
	* @param	sysDataSet		system vars data Set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void SetSysData(SysDataSet sysDataSet, Track::FileType fileType);

	/**
	* Set the aircraft's simulation data vars
	* 
	* @param	simDataSet		simulation vars data Set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void SetSimData(SimDataSet simDataSet, Track::FileType fileType);

	/**
	* Calculates and Sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void SetFuel(float timeStmp, float flightTime, Track::FileType fileType);

	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void StartAPUProc(float timeStmp, float startTime, Track::FileType fileType);

	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void StartEngineProc(float timeStmp, float startTime, Track::FileType fileType);

	/**
	* Runs through the stop engine procedure
	*
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void StopEngineProc(Track::FileType fileType);

	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   double			Throttle value
	*/
	double ConvertThrottle(double inputThrottle, Track::FileType fileType);

	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetThrottle(double throttleVal, Track::FileType fileType);

	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetPropPos(double throttleVal, int fltPhase, Track::FileType fileType);

	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser Setting 0 = Off / 1 = On
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetReverser(int reverserSet, Track::FileType fileType);

	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetAileron(double degrees, Track::FileType fileType);

	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetElevator(double degrees, Track::FileType fileType);

	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetElevatorTrim(double degrees, Track::FileType fileType);

	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetRudder(double degrees, Track::FileType fileType);

	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps stage Setting 1 to 5
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetFlaps(double stage, Track::FileType fileType);

	/**
	* Sets the aircraft's speed brakes / spoilers
	*
	* @param	spoilerSet		Spoiler Setting 0 = Down / 1 = Up
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetSpoiler(double spoilerSet, Track::FileType fileType);

	/**
	* Sets the aircraft's nose wheel deflection
	*
	* @param	noseWheelDegrees	Nose wheel deflection in degrees
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetNoseWheel(double noseWheelDegrees, Track::FileType fileType);

	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear Setting 1 = Down / 0 = Up
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetGear(double gearSet, Track::FileType fileType);

	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break pressure
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetBrakes(double brakeSet, Track::FileType fileType);

	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetNavLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's logo lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetLogoLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetBeaconLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftdis
	* @return	void
	*/
	void SetStrobeLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetTaxiLights(int lightSwitch, Track::FileType fileType);

	/**
	* This method Sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetWingLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetLandingLights(int lightSwitch, Track::FileType fileType);

	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign Setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign Setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign Setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void SetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt, Track::FileType fileType);

	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @param	fileType		Type of file, i.e. .ftd or .ftdis
	* @return	void
	*/
	void SetAltimeter(double altitude, Track::FileType fileType);

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
	void PerformModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase, Track::FileType fileType);

	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @return	void
	*/
	void GetCamView(float camCoords[], char camSide, char camPos, int camVar);
};
#endif