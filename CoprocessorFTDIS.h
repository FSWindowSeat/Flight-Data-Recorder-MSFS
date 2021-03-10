#pragma once
#include <chrono>
#include <FTDISFile.h>
#include <SimSoundEngine.h>
#include <SimModel.h>
#include <SimFSA320.cpp>
#include <SimFSB787.cpp>

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
	CoprocessorFTDIS(Processor* handler);
	~CoprocessorFTDIS();

private:
	Processor* cpu = nullptr;
	ProcessorCom* cpuCom = nullptr;
	FTDISFile* ftdisFile = nullptr;

	SimFSA320 simFSA320;
	SimFSB787 simFSB787;
	//SimFSB747 simFSB747;
	//SimPAA388 simPAA388;
	//SimBBA333 simBBA333;
	SimModel simAircraft;
	SimSoundEngine simSound;

	struct posDataSet
	{
		double lat;
		double lon;
		double alt;
		double hdg;
		double pitch;
		double bank;
		double spd;
	};
	posDataSet posSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };


	struct sysDataSet
	{
		double thr1;
		double thr2;
		double engN1;
		double alr;
		double elev;
		double rudder;
		double flaps;
		double splr;
		double gearPos;
	};

	sysDataSet sysSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	int cursorPos = 0, quit = 0,  initSnd = 0, loadFileLen = 0, loadRowCount = 0, loopCount = 1, progrT = 0, prevProgrT = -1;

	double	timeStamp = 0, elapsedTimeMil = 0, prevElapsedTimeMil = 0, simElapsedTimeMil = 0, simElapsedTimeSec = 0, simFrameTimeMil = 0,  simEngineStartTimeSec = 0,
		flightTime = 0, frameRate = 1, frameElapsedNow = 0, velBodyX = 0, autoRateWindowDep = 0, autoRateWindowArr = 0, depElev = 0, depElevAdj = 0, destElev = 0,
		destElevAdj = 0, flightElevAdj = 0, prevDepAltGround = 0, prevDestAltGround = 0, inThrtVal = 0, thrtPos = 0, n1ThrtVal = 0, 
		 acceleration = 0, prevVelocity = 0, verticalAccel = 0, verticalSpeed = 0, prevAlt = 0;

	bool engineStopped = FALSE, onGround = TRUE, fileLoaded = FALSE, soundStage = TRUE, runHalfRate = FALSE, runAutoRate = FALSE, enablePhysics = FALSE;

	double targetPeriod = 0.020;

private:
	/**
	* Initializes the coprocessor
	*/
	void InitCoprocessor();

public:
	/**
	* Determines the position of the file cursor. The cursor points to the record
	* of the flight data file to be read/processed
	* 
	* * @return   int	Cursor position flight data file
	*/
	int GetCursorPos();

	/**
	* Sets the aircraft's lateral position, vertical position and attitude 
	* in the sim
	*/
	void SetAircraftPos();


	/**
	* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
	*/
	void SetAircraftSystems();


	/**
	* Adjusts and plays the ambience sound stage (FMOD)
	*/
	void SetSoundStage();
};

