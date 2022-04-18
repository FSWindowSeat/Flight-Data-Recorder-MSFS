#ifndef Processor_H
#define Processor_H
#define _WINSOCKAPI_ 

#include <windows.h>
#include <iostream>
#include <string>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include <chrono>
#include <wx/wx.h>
#include <wx/thread.h>
#include <wx/msgqueue.h>
#include "PosDataSet.h"
#include "SimDataSet.h"
#include "SysDataSet.h"
#include "FDRFileStruct.h"
#include "CamShake.h"
#include "CamShakeStruct.h"
#include "ProcessorUtil.h"
#include "SimConnect.h"

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file
class CoprocessorFDR; // #include "CoprocessorFDR.h" in cpp file
class CoprocessorFTDIS; // #include "CoprocessorFTDIS.h" in cpp file
class Track; // #include "Track.h" in .cpp file
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
	
	Processor(GuiMain* handler, ProcessorCom* com, vector<Track*> tracks);
	// DEPRECATED
	Processor(GuiMain *handler, ProcessorCom *com, FDRFile *file);
	// DEPRECATED
	Processor(GuiMain *handler, ProcessorCom *com, FTDISFile *file);

	~Processor();

private:
	GuiMain *guiMainHandler = nullptr;
	vector<Track*> tracks;
	vector<CoprocessorFDR*> cpusFDR;
	vector<CoprocessorFTDIS*> cpusFTDIS;
	FDRFileStruct fdrRec = {};
	FDRFileStruct fdrRecords[100] = {};
	CamShake* headCam = nullptr;
	CamShakeStruct headCamData; 
	http_listener* listener = nullptr;
	DWORD simObjectId = 0;
	int simRequestId = 0, fileType = 0, zulu = 0, prevZulu = 0, tElapsed = 0, tFlt = 0, travelDist = 0;
	float zuluStartFrame = 0.0f, camX = 0.0f, camY = 0.0f, camZ = 0.0f;
	bool initRecordDataSet = false;

	struct returnDouble {
		double returnVar;
	};

	PosDataSet* posSet;
	SysDataSet* sysSet;
	SimDataSet* simSet;


public:
	FDRFile* fdrFile = nullptr;
	FTDISFile* ftdisFile = nullptr;
	ProcessorCom* cpuCom = nullptr;
	vector<DWORD> aiID;
	HANDLE  hSimConnect = NULL;
	HRESULT hr;

	enum EVENT_ID {
		KEY_FREEZE_LATITUDE_LONGITUDE_SET = 0x00001000,
		KEY_FREEZE_ALTITUDE_SET = 0x00002000,
		KEY_FREEZE_ATTITUDE_SET = 0x00003000,
		KEY_SIM_RATE_DECR = 0x00004000,
		KEY_SIM_RATE_INCR = 0x00005000,
		EVENT_RECUR_FRAME = 0x00006000,
		EVENT_MESSAGE_1 = 0x00007000,
		KEY_AILERON_SET = 0x00008000,
		KEY_SPOILERS_ARM_SET = 0x00009000,
		KEY_SPOILERS_SET = 0x0000A000,
		KEY_TOGGLE_NAV_LIGHTS = 0x0000B000,
		KEY_TOGGLE_LOGO_LIGHTS = 0x0000C000,
		KEY_TOGGLE_BEACON_LIGHTS = 0x0000D000,
		KEY_STROBES_SET = 0x0000E000,
		KEY_LANDING_LIGHTS_SET = 0x0000F000,
		KEY_TOGGLE_TAXI_LIGHTS = 0x00010000,
		KEY_TOGGLE_WING_LIGHTS = 0x00011000,
		KEY_GEAR_SET = 0x00001200,
		KEY_TURBINE_IGNITION_SWITCH_TOGGLE = 0x00013000,
		KEY_TOGGLE_STARTER1 = 0x00014000,
		KEY_TOGGLE_STARTER2 = 0x00015000,
		KEY_TOGGLE_STARTER3 = 0x00016000,
		KEY_TOGGLE_STARTER4 = 0x00017000,
		KEY_ENGINE_AUTO_START = 0x00018000,
		KEY_ENGINE_AUTO_SHUTDOWN = 0x00019000,
		KEY_APU_STARTER = 0x0001A000,
		KEY_APU_GENERATOR_SWITCH_SET = 0x0001B000,
		KEY_BLEED_AIR_SOURCE_CONTROL_SET = 0x0001C000,
		KEY_TOGGLE_MASTER_BATTERY = 0x0001D000,
		KEY_FLAPS_SET = 0x0001E000,
		AI_RELEASEATC = 0x0001F000,
		KEY_AXIS_PAN_PITCH = 0x00100000,
		KEY_AXIS_PAN_HEADING = 0x00200000,
		KEY_AXIS_PAN_TILT = 0x00300000,
		KEY_AXIS_ZOOM_IN_FINE = 0x00400000,
		KEY_AXIS_ZOOM_OUT_FINE = 0x00500000
	};

	enum DATA_DEFINE_ID {
		POS_DATA_SET = 0x00001000,
		SYS_DATA_SET = 0x00002000,
		SIM_DATA_SET = 0x00003000,
		THROTTLE_1 = 0x00004000,
		THROTTLE_2 = 0x00005000,
		THROTTLE_3 = 0x00006000,
		THROTTLE_4 = 0x00007000,
		ENG1_N1 = 0x00008000,
		ENG2_N1 = 0x00009000,
		ENG3_N1 = 0x0000A000,
		ENG4_N1 = 0x0000B000,
		ALR = 0x0000C000,
		ELEV = 0x0000D000,
		RUDDER = 0x0000E000,
		FLAPS = 0x0000F000,
		SPOILER = 0x00010000,
		GEAR_POS = 0x00011000,
		GEAR_ANGLE = 0x00012000,
		FUEL_CTR = 0x00013000,
		FUEL_LEFT = 0x00014000,
		FUEL_RIGHT = 0x00015000,
		NAV_LIGHT = 0x00016000,
		LOGO_LIGHT = 0x00017000,
		BEACON_LIGHT = 0x00018000,
		STROBE_LIGHT = 0x00019000,
		TAXI_LIGHT = 0x0001A000,
		WING_LIGHT = 0x0001B000,
		LNDG_LIGHT = 0x0001C000,
		MASTER_BATTERY = 0x0001D000
	};

	enum DATA_REQUEST_ID {
		REQ_POS_DATA = 0x00001000,
		REQ_SYS_DATA = 0x00002000,
		REQ_SIM_DATA = 0x00003000,
		REQ_AI_AC = 0x00004000
	};

	UINT reqIndex = 0;

	double altGround[100] = { 0.0 }, absAltGround[100] = { 0.0 }, staticCGAlt[100] = { 0.0 }, staticCGPitch[100] = { 0.0 }, flapsLeadingPrc[100] = { 0.0 }, flapsTrailingPrc[100] = { 0.0 };;

	double simFrameTimeSec = 0, simRate = 0, engN1 = 0; 

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
	* @return   void
	*/
	void static CALLBACK MyDispatchProcRD(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);

	/**
	* Handles SimConnect server responses (Instance method)
	*
	* @param	pData		Pointer to a data buffer, to be treated initially as a SIMCONNECT_RECV structure.
	* @param	cbData		The size of the data buffer, in bytes.
	* @return   void
	*/
	void DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData);

	/**
	* Handles the processing of SimConnect SIMCONNECT_RECV_ID_SIMOBJECT_DATA return messages
	* and converts them to a basic type 
	*
	* @param	pObjData	Pointer to SimConnect response object
	* @return	simCo Val	double
	*/
	double SimCoToDouble(SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData);
	double SimCoToInt(SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData);

public:
	/**
	* Initializes the connection to the sim (using SimConnect API)
	* 
	* @param	void
	* @return   bool
	*/
	bool InitSimConnect();

	/**
	* Initializes the REST API for the OBS.
	* 
	* @param	void
	* @return   void
	*/
	void InitRestApi();

	/**
	* Initializes the data record set and SimConnect request dispatch for flight recording
	*
	* @param	void
	* @return   void
	*/
	void InitRecordDataSet();

	/*
	* Returns simulation data variables set
	* 
	* @param	pos				int
	* @return	FDRFileStruct
	*/
	FDRFileStruct GetSimDataRec(int pos);
};
#endif