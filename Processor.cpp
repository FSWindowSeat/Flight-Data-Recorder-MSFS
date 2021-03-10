#include <Processor.h>
#include <ProcessorCom.h>
#include <CoprocessorFDR.h>
#include <CoprocessorFTDIS.h>
#include <GuiMain.h>
#include <FTDISFile.h>

/**
* Constructor / Destructor
*/
Processor::Processor(GuiMain *handler, ProcessorCom *com, FDRFile *file) : wxThread(wxTHREAD_DETACHED) {
	guiMainHandler = handler;
	cpuCom = com;
	fdrFile = file;
	fileType = 0;

	// Update flight time in case of continuation of existing recording
	if (fdrFile->GetSize() > 0)
		this->fdrRec.fltTime = fdrFile->fileData[fdrFile->GetSize()].fltTime;
}

Processor::Processor(GuiMain *handler, ProcessorCom *com, FTDISFile *file) : wxThread(wxTHREAD_DETACHED) {
	guiMainHandler = handler;
	cpuCom = com;
	ftdisFile = file;
	fileType = 1;
}


Processor::~Processor() {
	
	// Close coprocessors
	if (cpuFDR)
		delete cpuFDR;

	if (cpuFTDIS)
		delete cpuFTDIS;
	
	wxCriticalSectionLocker enter(guiMainHandler->cpuThreadCS);
	// Close thread
	guiMainHandler->cpuThread = NULL;
}


/**
* Thread entry
*/
wxThread::ExitCode Processor::Entry() {
	
	// Initialize connection to sim via SimConnect API
	InitSimConnect();
	
	return (wxThread::ExitCode)0;     // success
}


/**
* Thread exit
*/
wxThread::ExitCode Processor::Exit() {
	
	// Signal the Com handler that this thread is going to be destroyed and update status
	cpuCom->SetSimAPI(false);
	cpuCom->SetProcessorState(ProcessorCom::ProcessorState::stop);

	return (wxThread::ExitCode)0;     // success
}


/**
* Handle the processing of SimConnect SIMCONNECT_RECV_ID_SIMOBJECT_DATA return messages
*/
double Processor::SimCoToDouble(SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData) {
	DWORD ObjectID = pObjData->dwObjectID;
	returnDouble* rD = (returnDouble*)&pObjData->dwData;
	return rD->returnVar;
}


/**
* Handles SimConnect server responses (Static method)
*/
void CALLBACK Processor::MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {

	// Cast pContext pointer (void) to MainProcessor type and pass pData and cbData to instance method for further processing
	Processor *procThis = reinterpret_cast<Processor*>(pContext);
	procThis->DispatchProc(pData, cbData);
}


/**
* Handles SimConnect server responses (Instance method)
*/
void Processor::DispatchProc(SIMCONNECT_RECV* pData, DWORD cbData) {
	switch (pData->dwID) {
	// ------------------------------------------------------------------------
	// Handle specific responses for data requests
	case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: {
		SIMCONNECT_RECV_SIMOBJECT_DATA* pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA*)pData;
		switch (pObjData->dwRequestID)
		{
		// Flight Data Recorder vars
		case REQ_POS_DATA: {
			DWORD ObjectID = pObjData->dwObjectID;
			posSet = (posDataSet*)&pObjData->dwData;

			this->fdrRec.lat = posSet->lat;
			this->fdrRec.lon = posSet->lon;
			this->fdrRec.alt = posSet->alt;
			this->fdrRec.hdg = posSet->hdg;
			this->fdrRec.pitch = posSet->pitch;
			this->fdrRec.bank = posSet->bank;
			this->fdrRec.spd = posSet->spd;
			break;
		}
		
		case REQ_SYS_DATA: {
			DWORD ObjectID = pObjData->dwObjectID;
			sysSet = (sysDataSet*)&pObjData->dwData;

			this->fdrRec.thr = sysSet->thr1;
			this->fdrRec.n1 = sysSet->engN1;
			this->fdrRec.alr = sysSet->alr;
			this->fdrRec.elev = sysSet->elev;
			this->fdrRec.rudder = sysSet->rudder;
			this->fdrRec.flaps = sysSet->flaps;
			this->fdrRec.splr = sysSet->splr;
			this->fdrRec.gear = sysSet->gearPos;
			break;
		}

		// FTDIS vars
		case REQ_GROUND_ALT: {
			this->altGround = SimCoToDouble(pObjData);
			break;
		}
		
		case REQ_LEADING_FLAPS: {
			this->flapsLeadingPrc = SimCoToDouble(pObjData);
			break;
		}
		case REQ_TRAILING_FLAPS: {
			this->flapsTrailingPrc = SimCoToDouble(pObjData);
		}
		default:
			break;
		}
		break;
	}
	// ------------------------------------------------------------------------
	// Handle event processing
	case SIMCONNECT_RECV_ID_EVENT_FRAME:
	{
		SIMCONNECT_RECV_EVENT_FRAME* evt = (SIMCONNECT_RECV_EVENT_FRAME*)pData;

		switch (evt->uEventID)
		{
		// Handle per frame processing
		case EVENT_RECUR_FRAME: {
				if (cpuCom->GetSimAPI()) {
					switch (cpuCom->GetProcessorState()) {
						// ------------------------------------------------------------------------
						// Record
						case ProcessorCom::ProcessorState::record: {
							// Calc frame duration (in seconds) and round to two decimal places
							simFrameTimeSec = (1.0f / evt->fFrameRate);
							fdrRec.fltTime = std::floor(((fdrRec.fltTime + simFrameTimeSec) * 100) + .5) / 100;
							
							// Add record to .fdr file
							fdrFile->AddRecord(fdrRec);
							
							cpuCom->SetCursorPos(fdrFile->GetSize());
							break;
						}
						// ------------------------------------------------------------------------
						// Replay
						case ProcessorCom::ProcessorState::replay: {
							if (cpuCom->GetSimRate() != 0) {
								// Calc frame duration (in seconds)
								simFrameTimeSec = (1.0f / evt->fFrameRate);
								simRate = evt->fSimSpeed; //TODO: Align coprocessor sim rate, with MSFS2020 sim rate

								// Initialize coprocessor for corresponding file type
								if (fileType == 0) {
									if (!cpuFDR)
										cpuFDR = new CoprocessorFDR(this);

									// Set aircraft vars
									cpuFDR->SetAircraftPos();
									cpuFDR->SetAircraftSystems();
								}
								else if (fileType == 1) {
									if (!cpuFTDIS)
										cpuFTDIS = new CoprocessorFTDIS(this);

									// Set aircraft vars
									cpuFTDIS->SetAircraftPos();
									cpuFTDIS->SetAircraftSystems();
									cpuFTDIS->SetSoundStage();
								}
							}
							break;
						}
					}
				} 
			}
			break;

		default:
			break;
		}
		break;
	}
	// ------------------------------------------------------------------------
	// Handle close connection request
	case SIMCONNECT_RECV_ID_QUIT:
	{
		hr = SimConnect_Close(hSimConnect);
		cpuCom->SetSimAPI(false);
		break;
	}

	default:
		break;
	}
}


/** 
* Initializes the connection to the sim (using SimConnect API)
*/
bool Processor::InitSimConnect() {

	// Try to connect to sim via Sim Connect API
	if (SUCCEEDED(SimConnect_Open(&hSimConnect, "MainProcessorInterface", NULL, 0, 0, 0)))
	{
		// Subscribe to system events
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_RECUR_FRAME, "frame");

		// Map client / sim events
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_FREEZE_LATITUDE_LONGITUDE_SET, "FREEZE_LATITUDE_LONGITUDE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_FREEZE_ALTITUDE_SET, "FREEZE_ALTITUDE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_FREEZE_ATTITUDE_SET, "FREEZE_ATTITUDE_SET");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SIM_RATE_DECR, "SIM_RATE_DECR");
		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SIM_RATE_INCR, "SIM_RATE_INCR");

		// Assign flight postion and attitude vars to data definitions
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE LATITUDE", "degree");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE LONGITUDE", "degree");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE ALTITUDE", "meter");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE HEADING DEGREES TRUE", "degree");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE PITCH DEGREES", "degree");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "PLANE BANK DEGREES", "degree");
		hr = SimConnect_AddToDataDefinition(hSimConnect, POS_DATA_SET, "AIRSPEED INDICATED", "knot");

		// Assign system vars to data definitions, i.e. propulsion, control surfaces and aircraft systems
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "ENG N1 RPM:1", "number");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "AILERON POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "ELEVATOR POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "RUDDER POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "FLAPS HANDLE INDEX", "number");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "SPOILERS HANDLE POSITION", "position");
		hr = SimConnect_AddToDataDefinition(hSimConnect, SYS_DATA_SET, "GEAR HANDLE POSITION", "position");

		// Assign FTDIS specific vars
		hr = SimConnect_AddToDataDefinition(hSimConnect, GROUND_ALT, "GROUND ALTITUDE", "meter");
		hr = SimConnect_AddToDataDefinition(hSimConnect, LEADING_FLAPS, "LEADING EDGE FLAPS RIGHT PERCENT", "percent");
		hr = SimConnect_AddToDataDefinition(hSimConnect, TRAILING_FLAPS, "TRAILING EDGE FLAPS RIGHT PERCENT", "percent");

		// Assign data requests to dispatch
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQ_POS_DATA, POS_DATA_SET, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQ_SYS_DATA, SYS_DATA_SET, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQ_GROUND_ALT, GROUND_ALT, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQ_LEADING_FLAPS, LEADING_FLAPS, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
		hr = SimConnect_RequestDataOnSimObject(hSimConnect, REQ_TRAILING_FLAPS, TRAILING_FLAPS, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);

		// Set progress flags
		cpuCom->SetSimAPI(true);
		
		// While thread is active / delete request hasn't been sent
		while (!TestDestroy()) {
			// Call dispatch and Sim Connect disptach 
			SimConnect_CallDispatch(hSimConnect, MyDispatchProcRD, this);
			
			// Notify main thread
			wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, wxID_ANY);
			guiMainHandler->GetEventHandler()->AddPendingEvent(event);
			
			Sleep(1);
		}
		
		// Exit thread upon delete request from main thread
		if (TestDestroy()) {
			this->Exit();
		}
		return true;
	}
	// Connectivity to sim via SimConnect not established
	else {
		cpuCom->SetSimAPI(false);
		return false;
	}
}



