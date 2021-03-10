#pragma once
#include <FDRFile.h>

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
	CoprocessorFDR(Processor* handler);
	~CoprocessorFDR();

private:
	Processor* cpu = nullptr;
	ProcessorCom* cpuCom = nullptr;
	FDRFile* fdrFile = nullptr;

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

	double simElapsedTimeSec = 0;

	int cursorPos = 0;

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
	* @return   int	Cursor position flight data file
	*/
	void SetCursorPos();

	/**
	* Sets the aircraft's lateral position, vertical position and attitude
	* in the sim
	*/
	void SetAircraftPos();

	/**
	* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
	*/
	void SetAircraftSystems();

};

