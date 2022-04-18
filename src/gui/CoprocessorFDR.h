#ifndef CoprocessorFDR_H
#define CoprocessorFDR_H

// Includes
#include "PosDataSet.h"
#include "SimDataSet.h"
#include "SysDataSet.h"
#include "Track.h"
#include "FDRFile.h"
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
* Flight Data Recorder Coprocessor FDR files
*
* This represents the functionality to record and replay flight data 
* in MS FS2020 using the SimConnect API
*
* @author muppetlabs@fswindowseat.com
*/
class CoprocessorFDR{

public:
	CoprocessorFDR(Processor* handler, Track* track);
	~CoprocessorFDR();

private:
	Processor* cpu = nullptr;
	ProcessorCom* cpuCom = nullptr;
	Track* track = nullptr;
	FDRFile* fdrFile = nullptr;
	SimModel* simAircraft = nullptr;
	SimSoundEngine* simSound = nullptr;

	SimFSA320* simFSA320 = nullptr;
	SimBBA332* simBBA332 = nullptr;
	SimBBA343* simBBA343 = nullptr;
	SimPAA388* simPAA388 = nullptr;
	SimFSB787* simFSB787 = nullptr;

	PosDataSet posSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	SimDataSet simSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	SysDataSet sysSet = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	
	FTDISFileStruct sndProcRec = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0F};
	
	double simElapsedTimeSec = 0;// altOffset = 0;

	int cursorPos = 0;

	bool soundStage = true, initSound = false, activeSimObject = false, timeOffsetAdjust = false;

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
	* Returns the SimConnect object Id of the corresponding Sim Model.
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
	* Determines the position of the file cursor. The cursor points to the record
	* of the flight data file to be read/processed
	*
	* @param	void
	* @return   int	Cursor position flight data file
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