#include "Processor.h"
#include "ProcessorCom.h"
#include "CoprocessorFDR.h"
#include "CoprocessorFTDIS.h"
#include "GuiMain.h"
#include "Track.h"
#include "FDRFile.h"
#include "FTDISFile.h"

/**
* Constructor / Destructor
*/
Processor::Processor(GuiMain* handler, ProcessorCom* com, vector<Track*> tracks) : wxThread(wxTHREAD_DETACHED) {
	this->guiMainHandler = handler;
	this->cpuCom = com;
	this->tracks = tracks;
	this->headCam = new CamShake(handler);

	// ----------------------------------------------------------------------------------
	// TODO
	// ----------------------------------------------------------------------------------
	// Update flight time in case of continuation of existing recording
	//if (fdrFile->GetSize() > 0)
	//	this->fdrRec.fltTime = fdrFile->fileData[fdrFile->GetSize()].fltTime;
}
// Deprecated
Processor::Processor(GuiMain *handler, ProcessorCom *com, FDRFile *file) : wxThread(wxTHREAD_DETACHED) {
	guiMainHandler = handler;
	cpuCom = com;
	fdrFile = file;
	fileType = 0;

	// Update flight time in case of continuation of existing recording
	if (fdrFile->GetSize() > 0)
		this->fdrRec.fltTime = fdrFile->fileData[fdrFile->GetSize()].fltTime;
}
// Deprecated
Processor::Processor(GuiMain *handler, ProcessorCom *com, FTDISFile *file) : wxThread(wxTHREAD_DETACHED) {
	guiMainHandler = handler;
	cpuCom = com;
	ftdisFile = file;
	fileType = 1;
}


Processor::~Processor() {
	
	// Close coprocessors
	for (auto& cpu : cpusFDR)
		delete cpu;
	cpusFDR.clear();

	for (auto& cpu : cpusFTDIS)
		delete cpu;
	cpusFTDIS.clear();
	
	// Close thread
	wxCriticalSectionLocker enter(guiMainHandler->cpuThreadCS);
	guiMainHandler->cpuThread = NULL;

	// Close REST API
	if(listener)
		listener->close();
	delete listener;
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
	cpuCom->SetSimConnect(false);
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

		for (UINT n = 0; n < this->reqIndex; n++) {

			if (pObjData->dwRequestID == (UINT)REQ_POS_DATA + n) {
				DWORD ObjectID = pObjData->dwObjectID;
				posSet = (PosDataSet*)&pObjData->dwData;

				this->fdrRecords[n].lat = posSet->lat;
				this->fdrRecords[n].lon = posSet->lon;
				this->fdrRecords[n].alt = posSet->alt - this->staticCGAlt[n];
				this->fdrRecords[n].hdg = posSet->hdg;
				this->fdrRecords[n].pitch = posSet->pitch - this->staticCGPitch[n];
				this->fdrRecords[n].bank = posSet->bank;
				this->fdrRecords[n].spd = posSet->spd;
			}
			else if (pObjData->dwRequestID == (UINT)REQ_SYS_DATA + n) {
				DWORD ObjectID = pObjData->dwObjectID;
				sysSet = (SysDataSet*)&pObjData->dwData;

				this->fdrRecords[n].thr1 = sysSet->thr1;
				this->fdrRecords[n].thr2 = sysSet->thr2;
				this->fdrRecords[n].thr3 = sysSet->thr3;
				this->fdrRecords[n].thr4 = sysSet->thr4;
				this->fdrRecords[n].engN1E1 = sysSet->engN1E1;
				this->fdrRecords[n].engN1E2 = sysSet->engN1E2;
				this->fdrRecords[n].engN1E3 = sysSet->engN1E3;
				this->fdrRecords[n].engN1E4 = sysSet->engN1E4;
				this->fdrRecords[n].alr = sysSet->alr;
				this->fdrRecords[n].elev = sysSet->elev;
				this->fdrRecords[n].rudder = sysSet->rudder;
				this->fdrRecords[n].flaps = sysSet->flaps;
				this->fdrRecords[n].splr = sysSet->splr;
				this->fdrRecords[n].gearPos = sysSet->gearPos;
				this->fdrRecords[n].gearAngle = sysSet->gearAngle;
				this->fdrRecords[n].fuelCtr = sysSet->fuelCtr;
				this->fdrRecords[n].fuelLeft = sysSet->fuelLeft;
				this->fdrRecords[n].fuelRight = sysSet->fuelRight;
				this->fdrRecords[n].navL = sysSet->navL;
				this->fdrRecords[n].logoL = sysSet->logoL;
				this->fdrRecords[n].beacL = sysSet->beacL;
				this->fdrRecords[n].strbL = sysSet->strbL;
				this->fdrRecords[n].taxiL = sysSet->taxiL;
				this->fdrRecords[n].wngL = sysSet->wngL;
				this->fdrRecords[n].lndgL = sysSet->lndgL;
			}
			else if (pObjData->dwRequestID == (UINT)REQ_SIM_DATA + n) {
				DWORD ObjectID = pObjData->dwObjectID;
				simSet = (SimDataSet*)&pObjData->dwData;

				this->altGround[n] = simSet->groundAlt;
				this->absAltGround[n] = simSet->absGroundAlt;
				this->staticCGAlt[n] = simSet->staticCGAlt;
				this->staticCGPitch[n] = simSet->staticCGPitch;
				this->flapsLeadingPrc[n] = simSet->flapsLeadingPrc;
				this->flapsTrailingPrc[n] = simSet->flapsTrailingPrc;
				this->zulu = simSet->zulu;
				this->headCamData.fltTime = simSet->zulu;
				this->headCamData.simCamPosX = simSet->simCamPosX;
				this->headCamData.simCamPosY = simSet->simCamPosY;

				if (zuluStartFrame == 0)
					zuluStartFrame = simSet->zulu;

				this->fdrRecords[n].fltTime = (std::floor(((simSet->zulu - zuluStartFrame) * 100) + .5) / 100);
			}
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
				if (cpuCom->GetSimConnect()) {
					switch (cpuCom->GetProcessorState()) {
						// ------------------------------------------------------------------------
						// Record
						case ProcessorCom::ProcessorState::record: {
							// Initialize data record set and SimConnect for recording
							if (!initRecordDataSet)
								this->InitRecordDataSet();

							// Calc frame duration (in seconds) and round to two decimal places
							//simFrameTimeSec = (1.0f / evt->fFrameRate);
							//this->fdrRecords[this->simRequestId].fltTime = std::floor(((this->fdrRecords[this->simRequestId].fltTime + simFrameTimeSec) * 100) + .5) / 100;
							

							// Add record to .fdr file
							if(this->fdrRecords[this->simRequestId].lat != 0.0)
								fdrFile->AddRecord(this->fdrRecords[this->simRequestId]);
							
							cpuCom->SetCursorPos(fdrFile->GetSize());
							break;
						}
						// ------------------------------------------------------------------------
						// Replay 
						case ProcessorCom::ProcessorState::replay: {
							
							// Determine sim rate
							simFrameTimeSec = (1.0f / evt->fFrameRate);
							simRate = evt->fSimSpeed; 

							// Adjust FS sim rate to selected replay rate
							if (abs(cpuCom->GetReplayRate()) > simRate)
								SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SIM_RATE_INCR, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
							else if(abs(cpuCom->GetReplayRate()) < simRate)
								SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SIM_RATE_DECR, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
							
							// Loop through track(s), create individual co-processors for each track and update sim object information
							Track* track = nullptr;
							int cpusFDRCrsr = 0, cpusFTDISCrsr = 0;
							
							for (vector<int>::size_type tracksCrsr = 0; tracksCrsr != tracks.size(); tracksCrsr++) {
								track = tracks.at(tracksCrsr);
								
								if (track->GetFileType() == Track::FileType::FDR) {
									if (cpusFDRCrsr >= cpusFDR.size())
										// Create new processor
										cpusFDR.push_back(new CoprocessorFDR(this, track));
									else {
										// Run processor
										if(cpusFDR.at(cpusFDRCrsr)->GetSimObjectID() != -1) {
											cpusFDR.at(cpusFDRCrsr)->SetAircraftPos();
											cpusFDR.at(cpusFDRCrsr)->SetAircraftSystems();
											cpusFDR.at(cpusFDRCrsr)->SetSoundStage();
										}
									}
									cpusFDRCrsr++;
								}
								else if (track->GetFileType() == Track::FileType::FTDIS) {
									if (cpusFTDISCrsr >= cpusFTDIS.size())
										// Create new processor
										cpusFTDIS.push_back(new CoprocessorFTDIS(this, track));
									else {
										// Run processor
										if (cpusFTDIS.at(cpusFTDISCrsr)->GetSimObjectID() != -1) {
											cpusFTDIS.at(cpusFTDISCrsr)->SetCursorPos();
											cpusFTDIS.at(cpusFTDISCrsr)->SetAircraftPos();
											cpusFTDIS.at(cpusFTDISCrsr)->SetAircraftSystems();
											cpusFTDIS.at(cpusFTDISCrsr)->SetSoundStage();
										}
									}
									cpusFTDISCrsr++;
								}	
							}
							
							// Camera head shake processing for main/user track
							if(tracks.at(0)->GetTrackType() == Track::TrackType::USER){
								// Prepare camera data set
								this->headCamData.thr = this->fdrRecords[0].thr1;
								this->headCamData.flaps = this->flapsTrailingPrc[0];
								this->headCamData.splr = this->fdrRecords[0].splr;
								this->headCamData.gearPos = this->fdrRecords[0].gearPos;
								this->headCamData.absGroundAlt = this->absAltGround[0];

								// Update camera position
								headCam->SetCameraPos(this->headCamData, camX, camY, camZ);
								
								/* 
								@note As of SDK 0.16.0 Camera Events are not(!) supported
								this->hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_AXIS_PAN_PITCH, (DWORD)camX, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
								this->hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_AXIS_PAN_HEADING, (DWORD)camY, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
								*/
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
	// Receive Object ID for requested AI object and assign to corresponding processor
	case SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID: {
		SIMCONNECT_RECV_ASSIGNED_OBJECT_ID* pObjData = (SIMCONNECT_RECV_ASSIGNED_OBJECT_ID*)pData;
		this->aiID.push_back((DWORD)pObjData->dwObjectID);

		int aiTrackCrsr = 0, aiFDRTrackCrsr = 0, aiFTDISTrackCrsr = 0;
		for (auto& track : tracks) {
			if (track->GetTrackType() == Track::TrackType::AI) {
				if (track->GetFileType() == Track::FileType::FDR) {
					if (this->aiID.size() - 1 == aiTrackCrsr) {
						// Assign object ID
						cpusFDR.at(aiFDRTrackCrsr)->SetSimObjectId((DWORD)pObjData->dwObjectID);
						// Release AI control
						this->hr = SimConnect_AIReleaseControl(this->hSimConnect, pObjData->dwObjectID, AI_RELEASEATC); // (UINT)REQ_AI_AC + cpusFDR.at(aiFDRTrackCrsr)->GetSimRequestID());
					}
					aiFDRTrackCrsr++;
				}
				else if (track->GetFileType() == Track::FileType::FTDIS) {
					if (this->aiID.size() - 1 == aiTrackCrsr) {
						// Assign object ID
						cpusFTDIS.at(aiFTDISTrackCrsr)->SetSimObjectId((DWORD)pObjData->dwObjectID);
						// Release AI control
						this->hr = SimConnect_AIReleaseControl(this->hSimConnect, pObjData->dwObjectID, AI_RELEASEATC); //(UINT)REQ_AI_AC + cpusFDR.at(aiFDRTrackCrsr)->GetSimRequestID());

					}
					aiFTDISTrackCrsr++;
				}
				aiTrackCrsr++;
			}
		}
		break;
	}
	// ------------------------------------------------------------------------
	// Handle close connection request
	case SIMCONNECT_RECV_ID_QUIT:
	{
		hr = SimConnect_Close(hSimConnect);
		cpuCom->SetSimConnect(false);
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
		this->hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_RECUR_FRAME, "frame");

		// Map global client / sim events
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SIM_RATE_DECR, "SIM_RATE_DECR");
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SIM_RATE_INCR, "SIM_RATE_INCR");

		// Map camera controls
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (UINT) KEY_AXIS_PAN_PITCH, "AXIS_PAN_PITCH");
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (UINT) KEY_AXIS_PAN_HEADING, "AXIS_PAN_HEADING");
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (UINT) KEY_AXIS_PAN_TILT, "AXIS_PAN_TILT");
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (UINT) KEY_AXIS_ZOOM_IN_FINE, "ZOOM_IN_FINE");
		this->hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (UINT) KEY_AXIS_ZOOM_OUT_FINE, "ZOOM_OUT_FINE");

		// Set progress flags
		cpuCom->SetSimConnect(true);

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
		cpuCom->SetSimConnect(false);
		return false;
	}
}


/**
* Initializes the REST API for the OBS Overlay
* @return   bool
*/
void Processor::InitRestApi() {

	// Listen to and handle GET requests
	try {
		listener = new http_listener(U("http://127.0.0.1:1234"));

		// Open the listener, i.e. start accepting requests. Listener runs asynchronously uses a threadpool to launch the task
		listener->open().wait();
		listener->support(methods::GET, [&](http_request request) {

			// Calc elapsed time (in sim) between requests		
			if (this->prevZulu != 0 && this->zulu != 0 && this->zulu > this->prevZulu) {
				tElapsed = this->zulu - this->prevZulu;
				tFlt += tElapsed;
			}
			this->prevZulu = this->zulu;

			// Separate scope for JSON response (free up memory, when done)
			{
				// Prepare JSON response
				json::value jsonResponse;
				int cursorPos;
				try{
					cursorPos = cpuCom->GetCursorPos();
				}
				catch (...) {
					cursorPos = 1;
				}

				OptionsStruct options = cpuCom->GetOptions();

				// Add reference data
				jsonResponse[L"depHH"] = json::value::number(options.depHH);
				jsonResponse[L"depMM"] = json::value::number(options.depMM);
				jsonResponse[L"depGMTHH"] = json::value::number(options.depGMTHH);
				jsonResponse[L"depGMTMM"] = json::value::number(options.depGMTMM);
				jsonResponse[L"destName"] = json::value::string(utility::conversions::to_string_t(options.destName));
				jsonResponse[L"fltHH"] = json::value::number(options.fltHH);
				jsonResponse[L"fltMM"] = json::value::number(options.fltMM);
				jsonResponse[L"destGMTHH"] = json::value::number(options.destGMTHH);
				jsonResponse[L"destGMTMM"] = json::value::number(options.destGMTMM);
			
				// Add file specific data (.fdr / .ftd)
				if (fileType == 0) {
					jsonResponse[L"lat"] = json::value::number(fdrFile->fileData[cursorPos].lat);
					jsonResponse[L"lon"] = json::value::number(fdrFile->fileData[cursorPos].lon);

					jsonResponse[L"alt"] = json::value::number(fdrFile->fileData[cursorPos].alt);
					jsonResponse[L"hdg"] = json::value::number(fdrFile->fileData[cursorPos].hdg);

					jsonResponse[L"spd"] = json::value::number(fdrFile->fileData[cursorPos].spd);

					jsonResponse[L"zul"] = json::value::number(this->zulu);
					jsonResponse[L"tEl"] = json::value::number(tFlt);

					travelDist += (fdrFile->fileData[cursorPos].spd * 0.000319661) * tElapsed; //Knots to miles per second >> x * 0.000319661
					jsonResponse[L"dst"] = json::value::number(travelDist);
				}
				else if(fileType == 1){
					jsonResponse[L"lat"] = json::value::number(ftdisFile->fileData[cursorPos].lat);
					jsonResponse[L"lon"] = json::value::number(ftdisFile->fileData[cursorPos].lon);

					jsonResponse[L"alt"] = json::value::number(ftdisFile->fileData[cursorPos].alt);
					jsonResponse[L"hdg"] = json::value::number(ftdisFile->fileData[cursorPos].hdg);

					jsonResponse[L"spd"] = json::value::number(ftdisFile->fileData[cursorPos].spd);

					jsonResponse[L"zul"] = json::value::number(this->zulu);
					jsonResponse[L"tEl"] = json::value::number(tFlt);

					travelDist += (ftdisFile->fileData[cursorPos].spd * 0.000319661) * tElapsed; //Knots to miles per second >> x * 0.000319661
					jsonResponse[L"dst"] = json::value::number(travelDist);
				}
	
				// Prepare HTTP response
				http_response response(status_codes::OK);
				response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
				response.set_body(jsonResponse);
				request.reply(response);
			}
			this_thread::sleep_for(chrono::milliseconds(500));
			});
	}
	catch (...) {
		// Ellipsis exception handler to handle http listener / response exceptions
		// TODO: Implement specific handlers for each potential exception!

		// Close listener and clean up

	}

}


/**
* Initializes the data record set and SimConnect request dispatch for flight recording
*
* @param	void
* @return   void
*/
void Processor::InitRecordDataSet() {
	// Define dedicated SimConnect request ID for this simObject
	this->simRequestId = this->reqIndex;
	this->reqIndex++;

	// Assign flight postion and attitude vars to data definitions
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE LATITUDE", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE LONGITUDE", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE ALTITUDE", "meter");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE HEADING DEGREES TRUE", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE PITCH DEGREES", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "PLANE BANK DEGREES", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->POS_DATA_SET + this->simRequestId, "AIRSPEED INDICATED", "knot");

	// Assign simulation vars to data definitions
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:3", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GENERAL ENG THROTTLE LEVER POSITION:4", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:1", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:2", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:3", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "ENG N1 RPM:4", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "AILERON POSITION", "position");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "ELEVATOR POSITION", "position");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "RUDDER POSITION", "position");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "FLAPS HANDLE INDEX", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "SPOILERS HANDLE POSITION", "position");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GEAR HANDLE POSITION", "position");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "GEAR CENTER STEER ANGLE", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "FUEL TANK CENTER QUANTITY", "gallons");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "FUEL TANK LEFT MAIN QUANTITY", "gallons");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "FUEL TANK RIGHT MAIN QUANTITY", "gallons");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT NAV", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT LOGO", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT BEACON", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT STROBE", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT TAXI", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT WING", "number");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SYS_DATA_SET + this->simRequestId, "LIGHT LANDING", "number");

	// Assign simulation vars to data definitions
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "VELOCITY BODY Z", "knot");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "VELOCITY BODY Y", "m/s");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "LEADING EDGE FLAPS RIGHT PERCENT", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "TRAILING EDGE FLAPS RIGHT PERCENT", "percent");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "ZULU TIME", "seconds");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "GROUND ALTITUDE", "meter");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "PLANE ALT ABOVE GROUND MINUS CG", "meter");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "STATIC CG TO GROUND", "meter");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "STATIC PITCH", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "CAMERA GAMEPLAY PITCH YAW:1", "degree");
	this->hr = SimConnect_AddToDataDefinition(this->hSimConnect, (UINT)this->SIM_DATA_SET + this->simRequestId, "CAMERA GAMEPLAY PITCH YAW:0", "degree");
	
	// Add object specific data requests to SimConnect Dispatch
	this->hr = SimConnect_RequestDataOnSimObject(this->hSimConnect, (UINT)this->REQ_POS_DATA + this->simRequestId, (UINT)this->POS_DATA_SET + this->simRequestId, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
	this->hr = SimConnect_RequestDataOnSimObject(this->hSimConnect, (UINT)this->REQ_SYS_DATA + this->simRequestId, (UINT)this->SYS_DATA_SET + this->simRequestId, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);
	this->hr = SimConnect_RequestDataOnSimObject(this->hSimConnect, (UINT)this->REQ_SIM_DATA + this->simRequestId, (UINT)this->SIM_DATA_SET + this->simRequestId, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);

	initRecordDataSet = true;
}


/*
* Returns simulation data variables set
* @return	FDRFileStruct
*/
FDRFileStruct Processor::GetSimDataRec(int pos) {
	return this->fdrRecords[pos];
}