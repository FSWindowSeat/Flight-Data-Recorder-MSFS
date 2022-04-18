#ifndef CoprocessorFTDIS_H
#define CoprocessorFTDIS_H

// Includes
#include <chrono>
#include "PosDataSet.h"
#include "SimDataSet.h"
#include "SysDataSet.h"
#include "Track.h"
#include "FTDISFile.h"
#include "SimSoundEngine.h"
#include "SimModel.h"
#include "SimFSA320.cpp"
#include "SimFSB787.cpp"
#include "SimBBA332.cpp"
#include "SimBBA343.cpp"
#include "SimPAA388.cpp"

// Forward declarations to avoid circular include.
class Processor;  // #include "Processor.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file

/**
* Flight Data Recorder Coprocessor FTDIS files
*
* This represents the functionality to replay the flight data prepared by the 
* Flight Tracking Data Integration System (FTDIS) in MS FS2020 using the SimConnect API 
*
* @author muppetlabs@fswindowseat.com
*/
class CoprocessorFTDIS {

public:
	CoprocessorFTDIS(Processor* handler, Track* track);
	~CoprocessorFTDIS();

private:
	Processor *cpu = nullptr;
	ProcessorCom *cpuCom = nullptr;
	Track *track = nullptr;
	FTDISFile *ftdisFile = nullptr;
	SimModel *simAircraft = nullptr;
	SimSoundEngine *simSound = nullptr;

	SimFSA320 *simFSA320 = nullptr;
	SimBBA332 *simBBA332 = nullptr;
	SimBBA343 *simBBA343 = nullptr;
	SimPAA388 *simPAA388 = nullptr;
	SimFSB787 *simFSB787 = nullptr;
	
	PosDataSet posSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	SimDataSet simSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	SysDataSet sysSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };


	int runHalfRateSec = 1200, cursorPos = 0, prevCursorPos = 0, cursorOffSet = 0, quit = 0,  initSnd = 0, loadFileLen = 0, loadRowCount = 0, loopCount = 1, progrT = 0, prevProgrT = -1;

	double timeStamp = 0, simElapsedTimeSec = 0, simFrameTimeSec = 0, simEngineStartTimeSec = 0, flightTime = 0, tFrameDeltaMs = 0, 
	    frameRate = 1, velBodyX = 0, autoRateWindowDep = 0, autoRateWindowArr = 0, depElev = 0, depElevAdj = 0, destElev = 0,
		destElevAdj = 0, flightElevAdj = 0, prevDepAltGround = 0, prevDestAltGround = 0, inThrtVal = 0, thrtPos = 0, n1ThrtVal = 0, 
		acceleration = 0, prevVelocity = 0, verticalAccel = 0, verticalSpeed = 0, prevAlt = 0, absGroundAlt = 0;

	bool engineStopped = FALSE, onGround = TRUE, fileLoaded = FALSE, soundStage = TRUE, runUpdate = FALSE, runHalfRate = TRUE, runAutoRate = FALSE, enablePhysics = FALSE, initSound = FALSE, activeSimObject = false, timeOffsetAdjust = false;

	double targetPeriod = 0.01;

	std::chrono::high_resolution_clock::time_point tReplayStart, tFrameStart, tNow;

private:
	/**
	* Initializes the coprocessor
	*/
	void InitCoprocessor();

public:
	
	/**
	* Assigns the SimConnect object Id to corresponding Sim Model.
	* Required for newly requested AI objects only.
	*
	* @param	Sim Object ID	DWORD
	* @return	void
	*/
	void SetSimObjectId(DWORD simObjectId);
	
	/**
	* Returns the SimConnect object Id of tje corresponding Sim Model.
	* Required for newly requested AI objects only.
	*
	* @param	void
	* @return	Sim Object ID	DWORD
	*/
	DWORD GetSimObjectID();

	/**
	* Returns the models's SimConnect request ID
	* Required for newly requested AI objects only.
	*
	* @param	void
	* @return	Sim Object Request ID	int
	*/
	int GetSimRequestID();

	/**
	* Assigns a track to the coprocessor
	*
	* @param	track	Track (pointer)
	* @return	void
	*/
	void SetTrack(Track* track);

	/**
	* Returns track assigned to the coprocessor
	*
	* @param	void
	* @return	track	Track (pointer)
	*/
	Track* GetTrack();

	/**
	* Sets the position of the file cursor. The cursor points to the record
	* of the flight data file to be read/processed 
	*
	* @param	void
	* @return   void
	*/
	void SetCursorPos();

	/**
	* Sets the aircraft's lateral position, vertical position and attitude 
	* in the sim
	*
	* @param	void
	* @return   void
	*/
	void SetAircraftPos();


	/**
	* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
	*
	* @param	void
	* @return   void
	*/
	void SetAircraftSystems();


	/**
	* Adjusts and plays the ambience sound stage (FMOD)
	*
	* @param	void
	* @return   void
	*/
	void SetSoundStage();
};
#endif
