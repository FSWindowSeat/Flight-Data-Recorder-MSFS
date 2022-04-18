#ifndef SimBBA332_H
#define SimBBA332_H
#define _USE_MATH_DEFINES
#define _WINSOCKAPI_ 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <cmath> 
#include "SimConnect.h"
#include "Processor.h"
#include "Track.h"
#include "SimModelBehavior.cpp"

/**
* FTDIS Sim Model Behaviour for MS FS2020 BlackBox A332
*
* Definition of sim specific behaviour for strategy pattern
*
* @author muppetlabs@fswindowseat.com
*/
class SimBBA332 : public SimModelBehavior
{
private:
	Processor* cpu = nullptr;
	FDRFile* fdrFile = nullptr;
	FTDISFile* ftdisFile = nullptr;

	bool revEngaged = false, splrEngaged = false, splrArmed = false, navL = false, beaconL = false, strobeL = false, landingL = false, taxiL = false,
		wingL = false, gear = true, lockPosition = false, isAi = false;

	int startAPUProcStep, startEngineProcStep, gearStat, signNoSmk, signSeat, signAtt, trackModelFct, simObjectId = -1, simRequestId = 0;

	float fuelStartL = 0.0, fuelStartC = 0.0, fuelStartR = 0.0, fuelEndL = 0.0, fuelEndC = 0.0, fuelEndR = 0.0,
		acCntrAlt = 0, acFixedPitch = 0, acFixedTrim = 0, acEngineIdle = 0, acEngineMax = 0, elapsedTimeDelta,
		switchInterval, startAPUTime, prevFlexL, prevFlexLIn, prevFlexR, prevFlexRIn, trackAlt, trackModelFctTime;

	float fuel[3];

	struct controls {
		double throttle;
		double ailerons;
		int flaps;
		int spoiler;
	};

	controls flightCtrls = { 0.0, 0.0, 0, 0 };

	struct systems {
		int navLights;
		int beaconLights;
		int landingLights;
	};

	systems flightSyst = { 0,0,0 };

public:

	/**
	* Constructor / destructor
	*/
	SimBBA332(Processor* handler) {

		cpu = handler;
	}

	~SimBBA332() {
		// Unlock position and attitudes
		cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_ALTITUDE_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_ATTITUDE_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

		lockPosition = false;

		// Delete AI model
		this->PerformRemoveAiSimObject();
	}


	/**
	* Sets up a dedicated SimConnect requests for the sim model's specific functions
	* and initializes all of the corresponding parameters for processing
	*
	* @param	simObjectId		DWORD
	* @return   void
	*/
	void PerformInitSimObject(DWORD simObjectId) {
		this->simObjectId = simObjectId;

		// Initialize processing vars
		startAPUProcStep = 0, startAPUTime = 0, startEngineProcStep = 0, gearStat = 1, signNoSmk = 0, signSeat = 0, signAtt = 0,
		elapsedTimeDelta = 0, switchInterval = 0, prevFlexL = 0.0, prevFlexLIn = 0.0, prevFlexR = 0.0, prevFlexRIn = 0.0,
		trackAlt = 0, trackModelFctTime = 0, trackModelFct = 0;

		// Set Aircraft Fuel Parameters
		fuelStartL = 10000.0F;
		fuelStartC = 50000.0F;
		fuelStartR = 10000.0F;
		fuelEndL = 1000.0F;
		fuelEndC = 200.0F;
		fuelEndR = 1000.0F;

		// Set Aircraft geometry and control parameters
		this->acCntrAlt = 4.827F;
		this->acFixedPitch = -0.394F;
		this->acFixedTrim = -0.39F;

		// Set Aircraft propulsion parameters
		this->acEngineIdle = 0.00F;
		this->acEngineMax = 95.0F;

		// Define dedicated SimConnect request ID for this simObject
		this->simRequestId = cpu->reqIndex;
		cpu->reqIndex++;

		// Map client events
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET + this->simRequestId, "FREEZE_LATITUDE_LONGITUDE_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_FREEZE_ALTITUDE_SET + this->simRequestId, "FREEZE_ALTITUDE_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_FREEZE_ATTITUDE_SET + this->simRequestId, "FREEZE_ATTITUDE_SET");

		// Assign flight postion and attitude vars to data definitions
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE LATITUDE", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE LONGITUDE", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE ALTITUDE", "meter");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE HEADING DEGREES TRUE", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE PITCH DEGREES", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "PLANE BANK DEGREES", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, "AIRSPEED INDICATED", "knot");

		// Assign simulation vars to data definitions
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:3", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:4", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:1", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:2", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:3", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:4", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "AILERON POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "ELEVATOR POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "RUDDER POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "FLAPS HANDLE INDEX", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "SPOILERS HANDLE POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GEAR HANDLE POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "GEAR CENTER STEER ANGLE", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "FUEL TANK CENTER QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "FUEL TANK LEFT MAIN QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "FUEL TANK RIGHT MAIN QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT NAV", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT LOGO", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT BEACON", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT STROBE", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT TAXI", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT WING", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, "LIGHT LANDING", "number");

		// Assign simulation vars to data definitions
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "VELOCITY BODY Z", "knot");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "VELOCITY BODY Y", "m/s");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "LEADING EDGE FLAPS RIGHT PERCENT", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "TRAILING EDGE FLAPS RIGHT PERCENT", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "ZULU TIME", "seconds");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "GROUND ALTITUDE", "meter");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "PLANE ALT ABOVE GROUND MINUS CG", "meter");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "STATIC CG TO GROUND", "meter");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "STATIC PITCH", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "CAMERA GAMEPLAY PITCH YAW:1", "degree");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, "CAMERA GAMEPLAY PITCH YAW:0", "degree");

		// System Controls: Assign system vars and map client events
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->THROTTLE_1 + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->THROTTLE_2 + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->ENG1_N1 + this->simRequestId, "ENG N1 RPM:1", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->ENG2_N1 + this->simRequestId, "ENG N1 RPM:2", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->ALR + this->simRequestId, "AILERON POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->ELEV + this->simRequestId, "ELEVATOR POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->RUDDER + this->simRequestId, "RUDDER POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->FLAPS + this->simRequestId, "FLAPS HANDLE INDEX", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->SPOILER + this->simRequestId, "SPOILERS HANDLE POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->GEAR_POS + this->simRequestId, "GEAR HANDLE POSITION", "position");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->GEAR_ANGLE + this->simRequestId, "GEAR CENTER STEER ANGLE", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->FUEL_CTR + this->simRequestId, "FUEL TANK CENTER QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->FUEL_LEFT + this->simRequestId, "FUEL TANK LEFT MAIN QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->FUEL_RIGHT + this->simRequestId, "FUEL TANK RIGHT MAIN QUANTITY", "gallons");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->MASTER_BATTERY + this->simRequestId, "ELECTRICAL MASTER BATTERY", "Bool");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_AILERON_SET + this->simRequestId, "AILERON_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_FLAPS_SET + this->simRequestId, "FLAPS_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_SPOILERS_ARM_SET + this->simRequestId, "SPOILERS_ARM_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_SPOILERS_SET + this->simRequestId, "SPOILERS_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_GEAR_SET + this->simRequestId, "GEAR_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_MASTER_BATTERY + this->simRequestId, "TOGGLE_MASTER_BATTERY");

		// Light Controls: Assign light vars and map client events
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_NAV_LIGHTS + this->simRequestId, "TOGGLE_NAV_LIGHTS");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_LOGO_LIGHTS + this->simRequestId, "TOGGLE_LOGO_LIGHTS");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_BEACON_LIGHTS + this->simRequestId, "TOGGLE_BEACON_LIGHTS");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_STROBES_SET + this->simRequestId, "STROBES_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_TAXI_LIGHTS + this->simRequestId, "TOGGLE_TAXI_LIGHTS");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_WING_LIGHTS + this->simRequestId, "TOGGLE_WING_LIGHTS");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_LANDING_LIGHTS_SET + this->simRequestId, "LANDING_LIGHTS_SET");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->NAV_LIGHT + this->simRequestId, "LIGHT NAV", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->LOGO_LIGHT + this->simRequestId, "LIGHT LOGO", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->BEACON_LIGHT + this->simRequestId, "LIGHT BEACON", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->STROBE_LIGHT + this->simRequestId, "LIGHT STROBE", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->TAXI_LIGHT + this->simRequestId, "LIGHT TAXI", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->WING_LIGHT + this->simRequestId, "LIGHT WING", "number");
		cpu->hr = SimConnect_AddToDataDefinition(cpu->hSimConnect, (UINT)cpu->LNDG_LIGHT + this->simRequestId, "LIGHT LANDING", "number");

		// Engine Controls: Map client events to sim events
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_STARTER1 + this->simRequestId, "TOGGLE_STARTER1");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_TOGGLE_STARTER2 + this->simRequestId, "TOGGLE_STARTER2");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_ENGINE_AUTO_START + this->simRequestId, "ENGINE_AUTO_START");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_ENGINE_AUTO_SHUTDOWN + this->simRequestId, "ENGINE_AUTO_SHUTDOWN");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_APU_STARTER + this->simRequestId, "APU_STARTER");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_APU_GENERATOR_SWITCH_SET + this->simRequestId, "APU_GENERATOR_SWITCH_SET");
		cpu->hr = SimConnect_MapClientEventToSimEvent(cpu->hSimConnect, (UINT)cpu->KEY_BLEED_AIR_SOURCE_CONTROL_SET + this->simRequestId, "BLEED_AIR_SOURCE_CONTROL_SET");

		// Add object specific data requests to SimConnect Dispatch
		cpu->hr = SimConnect_RequestDataOnSimObject(cpu->hSimConnect, (UINT)cpu->REQ_POS_DATA + this->simRequestId, (UINT)cpu->POS_DATA_SET + this->simRequestId, this->simObjectId, SIMCONNECT_PERIOD_SIM_FRAME);
		cpu->hr = SimConnect_RequestDataOnSimObject(cpu->hSimConnect, (UINT)cpu->REQ_SYS_DATA + this->simRequestId, (UINT)cpu->SYS_DATA_SET + this->simRequestId, this->simObjectId, SIMCONNECT_PERIOD_SIM_FRAME);
		cpu->hr = SimConnect_RequestDataOnSimObject(cpu->hSimConnect, (UINT)cpu->REQ_SIM_DATA + this->simRequestId, (UINT)cpu->SIM_DATA_SET + this->simRequestId, this->simObjectId, SIMCONNECT_PERIOD_SIM_FRAME);

		//Initialize object for positioning 
		if (!this->lockPosition) {
			// Lock position and attitudes
			cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_ALTITUDE_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, this->simObjectId, (UINT)cpu->KEY_FREEZE_ATTITUDE_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

			this->lockPosition = true;
		}
	}


	/**
	* Creates new Ai object and defines initial position and attitude.
	* Initial position is defined as first record in data track.
	*
	* @param	track	Track
	* @return   void
	*/
	void PerformCreateAiSimObject(Track* track) {
		SIMCONNECT_DATA_INITPOSITION initPos;

		// Initialize position and attitude vars (-> 1st row .track )
		if (track->GetFileType() == Track::FileType::FDR) {
			fdrFile = track->GetFDRFile();

			initPos.Altitude = fdrFile->fileData.front().alt;
			initPos.Latitude = fdrFile->fileData.front().lat;
			initPos.Longitude = fdrFile->fileData.front().lon;
			initPos.Heading = fdrFile->fileData.front().hdg;
		}
		else if (track->GetFileType() == Track::FileType::FTDIS) {
			ftdisFile = track->GetFTDISFile();

			initPos.Altitude = ftdisFile->fileData.front().alt;
			initPos.Latitude = ftdisFile->fileData.front().lat;
			initPos.Longitude = ftdisFile->fileData.front().lon;
			initPos.Heading = ftdisFile->fileData.front().hdg;
		}

		// Default position and attitude vars
		initPos.Pitch = 0.0;
		initPos.Bank = 0.0;
		initPos.OnGround = 1;
		initPos.Airspeed = 1;

		// Request new AI object at initial position
		cpu->hr = SimConnect_AICreateNonATCAircraft(cpu->hSimConnect, track->GetSimCfgUuid().c_str(), "AI999", initPos, (UINT)cpu->REQ_AI_AC + this->simRequestId);
		
		this->isAi = true;
	}


	/**
	* Removes Ai object from sim
	*
	* @param	void
	* @return   void
	*/
	void PerformRemoveAiSimObject() {
		if(this->isAi)
			cpu->hr = SimConnect_AIRemoveObject(cpu->hSimConnect, this->simObjectId, this->simRequestId);
	}


	/**
	* Returns the sim objects's SimConnect ID
	*
	* @param	void
	* @return	simObjectOD		DWORD
	*/
	DWORD PerformGetSimObjectId() {
		return this->simObjectId;
	}


	/**
	* Assigns the sim object's SimConnect request ID
	*
	* @param	request ID		int
	* @return   void
	*/
	void PerformSetSimRequestId(int simRequestId) {
		this->simRequestId = simRequestId;
	}


	/**
	* Returns the sim object's SimConnect request ID
	*
	* @param	void
	* @retun	request ID		int
	*/
	int PerformGetSimRequestId() {
		return this->simRequestId;
	}


	/**
	* Returns the sim model's fixed trim constant
	*
	* @param	void
	* @param	Model fixed trim	float
	*/
	float PerformGetAcFixedTrim() {
		return this->acFixedTrim;
	}


	/**
	* Returns the sim model's fixed trim constant
	*
	* @param	void
	* @param	N1 engine idle		float
	*/
	float PerformGetAcEngineIdle() {
		return this->acEngineIdle;
	}


	/**
	* Set the aircraft's position and attitude
	*
	* @param	posDataSet		Position data set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformSetPosition(PosDataSet posDataSet, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition){
			
			// NOTE	This is to compensate for weird MSFS 2020 behaviour that adds randomly to the altitude of AI aircraft
			//if (this->isAi)
			//	posDataSet.alt -= 0.25;

			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->POS_DATA_SET + this->simRequestId, this->simObjectId, 0, 0, sizeof(posDataSet), &posDataSet);
		}
	}


	/**
	* Set the aircraft's system data vars
	*
	* @param	sysDataSet		Simulation vars data set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformSetSysData(SysDataSet sysDataSet, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition)
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SYS_DATA_SET + this->simRequestId, this->simObjectId, 0, 0, sizeof(sysDataSet), &sysDataSet);
	}


	/**
	* Set the aircraft's simulation data vars
	*
	* @param	simDataSet		Simulation vars data set
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformSetSimData(SimDataSet simDataSet, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition)
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SIM_DATA_SET + this->simRequestId, this->simObjectId, 0, 0, sizeof(simDataSet), &simDataSet);
	}


	/**
	* Calculates and sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformSetFuel(float timeStmp, float flightTime, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			fuel[0] = fuelStartC - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartC - fuelEndC));
			fuel[1] = fuelStartL - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartL - fuelEndL));
			fuel[2] = fuelStartR - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartR - fuelEndR));

			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->FUEL_CTR + this->simRequestId, simObjectId, 0, 0, sizeof(fuel[0]), &fuel[0]);
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->FUEL_LEFT + this->simRequestId, simObjectId, 0, 0, sizeof(fuel[1]), &fuel[1]);
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->FUEL_RIGHT + this->simRequestId, simObjectId, 0, 0, sizeof(fuel[2]), &fuel[2]);
		}
	}


	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Time in seconds after start of replay
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformStartAPUProc(float timeStmp, float startTime, Track::FileType fileType) {
		float apuSwitchDelay = 3, apuGenDelay = 10;
		bool battery = true;

		if (this->simObjectId != -1 && this->lockPosition) {
			// Start APU
			if (startAPUProcStep == 0) {
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_TOGGLE_MASTER_BATTERY + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_APU_STARTER + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				startAPUTime = timeStmp;
				startAPUProcStep = 1;
			}

			// Wait for APU to start up and turn on generators and APU bleed
			if (startAPUProcStep == 1 && (timeStmp - startAPUTime) > apuGenDelay) {
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_APU_GENERATOR_SWITCH_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_BLEED_AIR_SOURCE_CONTROL_SET + this->simRequestId, 2, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				startAPUProcStep = 3;
			}

			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->MASTER_BATTERY + this->simRequestId, simObjectId, 0, 0, sizeof(battery), &battery);
		}
	}


	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformStartEngineProc(float timeStmp, float startTime, Track::FileType fileType) {
		float stepDelay = 3, enginePreStartDelay = 15, enginePostStartDelay = 30;

		if (this->simObjectId != -1 && this->lockPosition) {
			if (timeStmp >= startTime && startEngineProcStep <= 1) {

				// Engine no. 2: Start starter 2 to give engine 2 a head start for auto engine start (the best I can do at the moment)
				if (startEngineProcStep == 0) {
					cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_TOGGLE_STARTER2 + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					startEngineProcStep = 1;
				}

				// Engine no. 1: Run auto engine start procedure
				if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 1) && startEngineProcStep == 1) {
					cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_ENGINE_AUTO_START + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					startEngineProcStep = 2;
				}
			}
		}
	}


	/**
	* Runs through the stop engine procedure
	*
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   void
	*/
	void PerformStopEngineProc(Track::FileType fileType) {
		cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_ENGINE_AUTO_SHUTDOWN + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}


	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return   float			Throttle value
	*/
	double PerformConvertThrottle(double inputThrottle, Track::FileType fileType) {
		double outputThrottle;

		if (inputThrottle <= -1)
			inputThrottle = (inputThrottle + 1) * -1;

		outputThrottle = (inputThrottle * (acEngineMax - acEngineIdle)) + acEngineIdle;

		return outputThrottle;
	}


	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetThrottle(double throttleVal, Track::FileType fileType) {
		double throttle = throttleVal;

		if (this->simObjectId != -1 && this->lockPosition) {
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->THROTTLE_1 + this->simRequestId, simObjectId, 0, 0, sizeof(throttle), &throttle);
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->THROTTLE_2 + this->simRequestId, simObjectId, 0, 0, sizeof(throttle), &throttle);

			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->ENG1_N1 + this->simRequestId, simObjectId, 0, 0, sizeof(throttle), &throttle);
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->ENG2_N1 + this->simRequestId, simObjectId, 0, 0, sizeof(throttle), &throttle);
		}
	}


	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetProp(double throttleVal, int fltPhase, Track::FileType fileType) {

	}


	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser setting 0 = Off / 1 = On
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetReverser(int reverserSet, Track::FileType fileType) {
		if (reverserSet == 1) {

			revEngaged = true;
		}
		else if (reverserSet == 0) {

			revEngaged = false;
		}
	}


	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetAileron(double degrees, Track::FileType fileType) {
		double ailerons = degrees, fsAlrRng = 16383;

		if (this->simObjectId != -1 && this->lockPosition) {

			if (fileType == Track::FileType::FTDIS)
				ailerons *= fsAlrRng;

			cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_AILERON_SET + this->simRequestId, (DWORD)ailerons, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		}
	}


	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetElevator(double degrees, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition)
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->ELEV + this->simRequestId, simObjectId, 0, 0, sizeof(degrees), &degrees);
	}


	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetElevatorTrim(double degrees, Track::FileType fileType) {
		//DEPRECATED
	}


	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetRudder(double degrees, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition)
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->RUDDER + this->simRequestId, simObjectId, 0, 0, sizeof(degrees), &degrees);
	}


	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps setting
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetFlaps(double stage, Track::FileType fileType) {
		double flapIndex = 0.0, fsFlpsRng = 16383;

		if (this->simObjectId != -1 && this->lockPosition) {

			if (fileType == Track::FileType::FTDIS) {
				switch ((int)stage) {
				case 1:
					flapIndex = 1.0;
					break;
				case 5:
					flapIndex = 1.0;
					break;
				case 10:
					flapIndex = 2.0;
					break;
				case 15:
					flapIndex = 3.0;
					break;
				case 25:
					flapIndex = 3.0;
					break;
				case 30:
					flapIndex = 4.0;
					break;
				default:
					flapIndex = 0.0;
				}

				cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->FLAPS + this->simRequestId, simObjectId, 0, 0, sizeof(flapIndex), &flapIndex);
			}
			else if (fileType == Track::FileType::FDR) {
				DWORD flapsDeg = stage * (fsFlpsRng/5);
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_FLAPS_SET + this->simRequestId, flapsDeg, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

			}
		}
	}


	/**
	* Sets the aircraft's speed brakes / spoiler
	*
	* @param	spoilerSet		Spoiler setting 0 = Down / 1 = Mid / 2 = Up
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetSpoiler(double spoilerSet, Track::FileType fileType) {
		double spoiler;

		if (this->simObjectId != -1 && this->lockPosition) {

			if (fileType == Track::FileType::FTDIS) {
				if ((int)spoilerSet == 1 && !splrArmed) {
					// Disengage
					if (splrEngaged) {
						spoiler = 0.0;
						cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SPOILER + this->simRequestId, simObjectId, 0, 0, sizeof(spoiler), &spoiler);
					}
				}
				else if ((int)spoilerSet == 2 && !splrEngaged) {
					spoiler = 1.0;
					// Ensure spoiler are disarmed
					cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_SPOILERS_ARM_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					Sleep(1);
					// Extend spoiler
					cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SPOILER + this->simRequestId, simObjectId, 0, 0, sizeof(spoiler), &spoiler);
					//spoiler = 16383;
					//hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SPOILERS_SET, spoiler, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					splrEngaged = true;
					splrArmed = false;
				}
				else if ((int)spoilerSet == 0 && splrEngaged) {
					spoiler = 0.0;
					cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SPOILER + this->simRequestId, simObjectId, 0, 0, sizeof(spoiler), &spoiler);
					//hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SPOILERS_SET, spoiler, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
					splrEngaged = false;
				}
			}
			else if (fileType == Track::FileType::FDR)
				cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->SPOILER + this->simRequestId, simObjectId, 0, 0, sizeof(spoilerSet), &spoilerSet);

		}
	}


	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear setting 1 = Down / 0 = Up
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetGear(double gearPos, Track::FileType fileType) {
		FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

		if (this->simObjectId != -1 && this->lockPosition) {
			if (gearPos == 1 && gearPos != simData.gearPos)
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_GEAR_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			else if (gearPos == 0 && gearPos != simData.gearPos)
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_GEAR_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);

		}
	}


	/**
	* Sets the aircrafts's nose wheel deflection
	*
	* @param	noseWheelDegrees	Nose wheel deflection in degrees
	* @param	fileType			Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetNoseWheel(double noseWheelDegrees, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition)
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->GEAR_ANGLE + this->simRequestId, simObjectId, 0, 0, sizeof(noseWheelDegrees), &noseWheelDegrees);
	}


	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break setting 1 = On / 0 = Off
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetBrakes(double brakeSet, Track::FileType fileType) {

		/*if (brakeSet == 1)

		else if (brakeSet == 0)*/

	}


	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetNavLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			double state = (double)lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->NAV_LIGHT + this->simRequestId, this->simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's runway logo lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetLogoLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			double state = (double)lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->LOGO_LIGHT + this->simRequestId, this->simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetBeaconLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			double state = (double)lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->BEACON_LIGHT + this->simRequestId, this->simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 2 = On / 1 = Auto / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetStrobeLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			double state = (double)lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->STROBE_LIGHT + this->simRequestId, this->simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetTaxiLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			double state = lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->TAXI_LIGHT + this->simRequestId, simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetWingLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			double state = (double)lightSwitch;
			cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, (UINT)cpu->WING_LIGHT + this->simRequestId, this->simObjectId, 0, 0, sizeof(state), &state);
		}
	}


	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetLandingLights(int lightSwitch, Track::FileType fileType) {
		if (this->simObjectId != -1 && this->lockPosition) {
			FDRFileStruct simData = cpu->GetSimDataRec(this->simRequestId);

			if (lightSwitch == 1 && lightSwitch != simData.lndgL)
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_LANDING_LIGHTS_SET + this->simRequestId, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			else if (lightSwitch == 0 && lightSwitch != simData.lndgL)
				cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, simObjectId, (UINT)cpu->KEY_LANDING_LIGHTS_SET + this->simRequestId, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
		}
	}


	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign setting 1 = On / 0 = Off
	* @param	fileType			Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt, Track::FileType fileType) {
		// Turn on no smoking sign


		// Turn on fasten seatbelt sign

		// Turn off fasten seatbelt sign


		// Trigger cabin attendance sign


	}


	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @param	fileType	Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void PerformSetAltimeter(double altitude, Track::FileType fileType) {
		float baroVal;

		if (altitude < 10000.0)
			baroVal = 0;
		else
			baroVal = 29.92F;

		/*
		XPLMSetDataf(acBaroL, baroVal);
		XPLMSetDataf(acBaroR, baroVal);*/
	}


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
	void PerformModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase, Track::FileType fileType) {


	}


	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @param	fileType		Type of file, i.e. .ftd or .ftd
	* @return	void
	*/
	void GetCamView(float camCoords[], char camSide, char camPos, int camVar) {

	}
};
#endif