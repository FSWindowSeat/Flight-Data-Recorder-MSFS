#include <CoprocessorFDR.h>
#include <Processor.h>
#include <ProcessorCom.h>

/**
* Constructor / Destructor
*/
CoprocessorFDR::CoprocessorFDR(Processor* handler) {
	cpu = handler;
	cpuCom = handler->cpuCom;
	fdrFile = handler->fdrFile;	
	this->InitCoprocessor();
}

CoprocessorFDR::~CoprocessorFDR() {
	// Unlock position and attitudes
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ALTITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ATTITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}


/**
* Initializes the coprocessor
*/
void CoprocessorFDR::InitCoprocessor() {
	// Lock position and attitudes
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ALTITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ATTITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

/**
* Determines the position of the file cursor. The cursor points to the record
* of the flight data file to be read/processed
*/
void CoprocessorFDR::SetCursorPos() {

	// Check if 
	if (this->simElapsedTimeSec == 0 && cpuCom->GetCursorPos() > 0) {
		this->simElapsedTimeSec = fdrFile->GetFlightTime();
		cursorPos = fdrFile->GetSize();
	}
	else {
		// Calculate sim elapsed time
		this->simElapsedTimeSec += cpu->simFrameTimeSec * cpuCom->GetSimRate();
	}
	

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//TODO: Input file has variable frame rate. Improve logic to find correct position / use interpolation
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Determine cursor position
	if (cpuCom->GetSimRate() > 0) {
		while (this->cursorPos < fdrFile->GetSize() && fdrFile->fileData[this->cursorPos].fltTime < simElapsedTimeSec) {
			this->cursorPos += 1;
		}
	}
	else if (cpuCom->GetSimRate() < 0) {
		while (this->cursorPos > 0 && fdrFile->fileData[this->cursorPos].fltTime > simElapsedTimeSec) {
			this->cursorPos -= 1;
		}
	}

	cpuCom->SetCursorPos(cursorPos);
}


/**
* Sets the aircraft's lateral position, vertical position and attitude
* in the sim
*/
void CoprocessorFDR::SetAircraftPos() {
	this->SetCursorPos();

	// Set position vars
	this->posSet.lat = fdrFile->fileData[this->cursorPos].lat;
	this->posSet.lon = fdrFile->fileData[this->cursorPos].lon;
	this->posSet.alt = fdrFile->fileData[this->cursorPos].alt;
	this->posSet.hdg = fdrFile->fileData[this->cursorPos].hdg;
	this->posSet.pitch = fdrFile->fileData[this->cursorPos].pitch;
	this->posSet.bank = fdrFile->fileData[this->cursorPos].bank;
	this->posSet.spd = fdrFile->fileData[this->cursorPos].spd;

	cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, cpu->POS_DATA_SET, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(this->posSet), &this->posSet);
}


/**
* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
*/
void CoprocessorFDR::SetAircraftSystems(){
	// Set system vars
	this->sysSet.thr1 = fdrFile->fileData[this->cursorPos].thr;
	this->sysSet.thr2 = fdrFile->fileData[this->cursorPos].thr;
	this->sysSet.engN1 = fdrFile->fileData[this->cursorPos].n1;
	this->sysSet.alr = fdrFile->fileData[this->cursorPos].alr;
	this->sysSet.elev = fdrFile->fileData[this->cursorPos].elev;
	this->sysSet.rudder = fdrFile->fileData[this->cursorPos].rudder;
	this->sysSet.flaps = fdrFile->fileData[this->cursorPos].flaps;
	this->sysSet.splr = fdrFile->fileData[this->cursorPos].splr;
	this->sysSet.gearPos = fdrFile->fileData[this->cursorPos].gear;

	cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, cpu->SYS_DATA_SET, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(this->sysSet), &this->sysSet);
}
