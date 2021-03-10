#pragma warning(disable : 4996)
#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include <ProcessorUtil.h>
#include <SimConnect.h>
#include <FDRFileStruct.h>

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file
class CoprocessorFDR; // #include "CoprocessorFDR.h" in cpp file
class CoprocessorFTDIS; // #include "CoprocessorFTDIS.h" in cpp file
class FDRFile; // #include "FDRfile.h" in .cpp file
class FTDISFile; // #include "FTDISfile.h" in .cpp file


/** 
* Flight Data Recorder Replay Processor
*
* Represents the functionality to establish a connection to MS FS2020 via the SimConnect API,
* to handle the API's requests/responses and to orchestrate all of the request/responses to 
* record and replay MS FS2020 flight data 
*
* @author muppetlabs@fswindowseat.com
*/
class Processor : public wxThread {
public:
	Processor(GuiMain *handler, ProcessorCom *com, FDRFile *file);
	Processor(GuiMain *handler, ProcessorCom *com, FTDISFile *file);

	~Processor();

private:
	GuiMain *guiMainHandler = nullptr;
	CoprocessorFDR *cpuFDR = nullptr;
	CoprocessorFTDIS *cpuFTDIS = nullptr;
	FDRFileStruct fdrRec = {};
	int fileType = 0;

	struct returnDouble {
		double returnVar;
	};

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
	posDataSet* posSet;
	
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

	sysDataSet* sysSet;

public:
	FDRFile* fdrFile = nullptr;
	FTDISFile* ftdisFile = nullptr;
	ProcessorCom* cpuCom = nullptr;
	HANDLE  hSimConnect = NULL;
	HRESULT hr;

	enum EVENT_ID {
		KEY_FREEZE_LATITUDE_LONGITUDE_SET,
		KEY_FREEZE_ALTITUDE_SET,
		KEY_FREEZE_ATTITUDE_SET,
		KEY_SIM_RATE_DECR,
		KEY_SIM_RATE_INCR,
		EVENT_RECUR_FRAME,
		EVENT_MESSAGE_1,
		KEY_AILERON_SET,
		KEY_SPOILERS_ARM_SET,
		KEY_SPOILERS_SET,
		KEY_TOGGLE_NAV_LIGHTS,
		KEY_TOGGLE_BEACON_LIGHTS,
		KEY_STROBES_SET,
		KEY_LANDING_LIGHTS_SET,
		KEY_TOGGLE_TAXI_LIGHTS,
		KEY_TOGGLE_WING_LIGHTS,
		KEY_GEAR_SET,
		KEY_TOGGLE_STARTER1,
		KEY_TOGGLE_STARTER2,
		KEY_ENGINE_AUTO_START,
		KEY_ENGINE_AUTO_SHUTDOWN
	};

	enum DATA_DEFINE_ID {
		POS_DATA_SET,
		SYS_DATA_SET,
		GROUND_ALT,
		LEADING_FLAPS,
		TRAILING_FLAPS
	};

	enum DATA_REQUEST_ID {
		REQ_POS_DATA,
		REQ_SYS_DATA,
		REQ_GROUND_ALT,
		REQ_LEADING_FLAPS,
		REQ_TRAILING_FLAPS
	};

	std::chrono::high_resolution_clock::time_point tStart, tFrameEnd, tFrameStart;

	double simFrameTimeSec = 0, simRate = 0, engN1 = 0, altGround = 0, flapsLeadingPrc = 0, flapsTrailingPrc = 0;

	//bool initAPI = FALSE;

private:

	/**
	* Thread entry and exit
	*
	* @param -
	*/
	virtual ExitCode Entry();
	virtual ExitCode Exit();

	/**
	* Handles SimConnect server responses (Static method)
	*
	* @param	pData		Pointer to a data buffer, to be treated initially as a SIMCONNECT_RECV structure.
	* @param	cbData		The size of the data buffer, in bytes.
	* @param	pContext	Pointer specified by the client in a SimConnect_CallDispatch function call
	*/
	void static CALLBACK MyDispatchProcRD(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);

	/**
	* Handles SimConnect server responses (Instance method)
	*
	* @param	pData		Pointer to a data buffer, to be treated initially as a SIMCONNECT_RECV structure.
	* @param	cbData		The size of the data buffer, in bytes.
	*/
	void DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData);

	/**
	* Handle the processing of SimConnect SIMCONNECT_RECV_ID_SIMOBJECT_DATA return messages
	* and converts them to a basic type 
	*
	* @param	pObjData	Pointer to SimConnect response object
	*/
	double SimCoToDouble(SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData);
	double SimCoToInt(SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData);

public:
	/**
	* Initializes the connection to the sim (using SimConnect API)
	* @return   bool
	*/
	bool InitSimConnect();

};