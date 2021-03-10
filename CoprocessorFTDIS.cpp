#include <CoprocessorFTDIS.h>
#include <Processor.h>
#include <ProcessorCom.h>

/**
* Constructor / Destructor
*/
CoprocessorFTDIS::CoprocessorFTDIS(Processor* handler) {
	cpu = handler;
	cpuCom = handler->cpuCom;
	ftdisFile = handler->ftdisFile;
	this->InitCoprocessor();
}

CoprocessorFTDIS::~CoprocessorFTDIS() {
	// Unlock position and attitudes
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ALTITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ATTITUDE_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}


/**
* Initializes the coprocessor
*/
void CoprocessorFTDIS::InitCoprocessor() {
	
	// Assign flight model specific vars to data definitions
	//simAircraft.setSimModelBehavior(&simFSA320);
	simAircraft.setSimModelBehavior(&simFSB787);
	//simAircraft.setSimModelBehavior(&simFSB747);
	//simAircraft.setSimModelBehavior(&simPAA388);
	//simAircraft.setSimModelBehavior(&simBBA333);
	simAircraft.initProcessing();

	// Init ground altitudes
	cpu->altGround = ftdisFile->fileData.front().alt;
	prevDepAltGround = ftdisFile->fileData.front().alt;
	prevDestAltGround = ftdisFile->fileData.back().alt;

	// Init sound stage
	// A320 > A320NX.bank A320NX.strings.bank
	// B781 > B781.bank B781.strings.bank
	// A388 > A388.bank A388.strings.bank
	// A332 > A332RR.bank A332RR.strings.bank 
	simSound.initSoundStage(L"B781.bank", L"B781.strings.bank");

	// Init time vars
	cpu->tStart = std::chrono::high_resolution_clock::now();
	cpu->tFrameEnd = std::chrono::high_resolution_clock::now();
	simEngineStartTimeSec = 35;

	// Lock position and attitudes
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_LATITUDE_LONGITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ALTITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	cpu->hr = SimConnect_TransmitClientEvent(cpu->hSimConnect, SIMCONNECT_OBJECT_ID_USER, cpu->KEY_FREEZE_ATTITUDE_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}


/**
* Determines the position of the file cursor. The cursor points to the record
* of the flight data file to be read/processed  
*/
int CoprocessorFTDIS::GetCursorPos() {
	int cursorPos = 0;
	
	// Calculate sim elapsed time
	this->simElapsedTimeSec += cpu->simFrameTimeSec * cpuCom->GetSimRate();

	// Determine cursor position
	cursorPos = (int)std::round(this->simElapsedTimeSec / this->targetPeriod);
	if (cursorPos < 0)
		cursorPos = 0;
	else if (cursorPos > ftdisFile->size)
		cursorPos = ftdisFile->size;

	cpuCom->SetCursorPos(cursorPos);
	
	return cursorPos;
}


/**
* Sets the lateral position, vertical position and attitude of the aircraft
* in the sim
*/
void CoprocessorFTDIS::SetAircraftPos() {
	this->cursorPos = this->GetCursorPos();

	posSet.lat = ftdisFile->fileData[this->cursorPos].lat;
	posSet.lon = ftdisFile->fileData[this->cursorPos].lon;

	// Calculate altitude
	if (ftdisFile->fileData[this->cursorPos].fltTime < 1800) {
		// Determine ground altitude (Departure)
		if (ftdisFile->fileData[this->cursorPos].alt == ftdisFile->depElev) {
			// Check change in ground altitude and "flatten" large changes 
			if (abs(cpu->altGround - this->prevDepAltGround) >= 0.01) {
				this->prevDepAltGround += (cpu->altGround - this->prevDepAltGround) / (1 / (this->targetPeriod * 2));
				posSet.alt = this->prevDepAltGround + simAircraft.getAcCntrAlt();
			}
			else
				posSet.alt = cpu->altGround + simAircraft.getAcCntrAlt();
		}
		else
			// Determine in-flight altitude
			posSet.alt = ftdisFile->fileData[this->cursorPos].alt + simAircraft.getAcCntrAlt();

	}
	else if (ftdisFile->fileData[this->cursorPos].fltTime > ftdisFile->flightTime - 1800) {
		// Determine ground altitude (Destination)
		if (ftdisFile->fileData[this->cursorPos].alt == ftdisFile->destElev) {
			// Check change in ground altitude and "flatten" large changes 
			if (abs(cpu->altGround - this->prevDestAltGround) >= 0.01) {
				this->prevDestAltGround += (cpu->altGround - this->prevDestAltGround) / (1 / (this->targetPeriod * 2));
				posSet.alt = this->prevDestAltGround + simAircraft.getAcCntrAlt();
			}
			else
				posSet.alt = cpu->altGround + simAircraft.getAcCntrAlt();
		}
		else
			// Determine in-flight altitude
			posSet.alt = ftdisFile->fileData[this->cursorPos].alt + simAircraft.getAcCntrAlt();
	}
	else {
		// Determine in-flight altitude
		posSet.alt = ftdisFile->fileData[this->cursorPos].alt + simAircraft.getAcCntrAlt();
	}

	// Update attitude vars
	posSet.hdg = ftdisFile->fileData[this->cursorPos].hdg;
	posSet.pitch = (ftdisFile->fileData[this->cursorPos].pitch + simAircraft.getAcFixedPitch()) * -1;
	posSet.bank = ftdisFile->fileData[this->cursorPos].bank * -1;
	posSet.spd = ftdisFile->fileData[this->cursorPos].spd;

	cpu->hr = SimConnect_SetDataOnSimObject(cpu->hSimConnect, cpu->POS_DATA_SET, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(this->posSet), &this->posSet);
}


/**
* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
*/
void CoprocessorFTDIS::SetAircraftSystems() {
	
	// Set fuel
	simAircraft.setFuel((float)this->simElapsedTimeSec, (float)ftdisFile->flightTime);

	// Start engine procedure during push back
	simAircraft.startEngineProc((float)this->simElapsedTimeSec, (float)this->simEngineStartTimeSec);

	// Stop engines after landing and taxi
	if (ftdisFile->fileData[this->cursorPos].fltPhase == 5 && ftdisFile->fileData[this->cursorPos].thr == -99 && !this->engineStopped) {
		this->thrtPos = -99;
		simAircraft.stopEngineProc();
		this->engineStopped = TRUE;
	}
	else if (!this->engineStopped) {
		// Set aircraft throttle 
		this->n1ThrtVal = simAircraft.convertThrottle((float)ftdisFile->fileData[this->cursorPos].thr);

		if (this->thrtPos < this->n1ThrtVal) {
			this->thrtPos += (100 / 10) * cpu->simFrameTimeSec;

			if (this->thrtPos > this->n1ThrtVal)
				this->thrtPos = this->n1ThrtVal;

		}
		else if (this->thrtPos > this->n1ThrtVal) {
			this->thrtPos -= (100 / 10) * cpu->simFrameTimeSec;

			if (this->thrtPos < this->n1ThrtVal)
				this->thrtPos = this->n1ThrtVal;
		}

		simAircraft.setThrottle((float)this->thrtPos);
	}
	
	// Set flight controls
	simAircraft.setAileron(ftdisFile->fileData[this->cursorPos].alr);
	simAircraft.setFlaps(ftdisFile->fileData[this->cursorPos].flap);

	// Set spoilers
	if ((this->cursorPos > ftdisFile->fileData.size() / 2 && ftdisFile->fileData[this->cursorPos].splr == 0))
		// Arm for landing
		simAircraft.setSpoiler(1);
	else
		simAircraft.setSpoiler(ftdisFile->fileData[this->cursorPos].splr);

	// Set gear			
	simAircraft.setGear(ftdisFile->fileData[this->cursorPos].gear);
	simAircraft.setNoseWheel(ftdisFile->fileData[this->cursorPos].noseWhl);

	// Set lights
	simAircraft.setNavLights(ftdisFile->fileData[this->cursorPos].navL);
	simAircraft.setBeaconLights(ftdisFile->fileData[this->cursorPos].beacL);
	simAircraft.setStrobeLights(ftdisFile->fileData[this->cursorPos].strbL);
	simAircraft.setTaxiLights(ftdisFile->fileData[this->cursorPos].taxiL);
	simAircraft.setLandingLights(ftdisFile->fileData[this->cursorPos].lndgL);
	simAircraft.setWingLights(ftdisFile->fileData[this->cursorPos].wngL);

}

/**
* Adjusts and plays the ambience sound stage (FMOD)
*/
void CoprocessorFTDIS::SetSoundStage() {
	if (soundStage) {
		FTDISFileStruct sndProcRec = ftdisFile->fileData[this->cursorPos];
		sndProcRec.thr = (float)thrtPos;
		sndProcRec.flap = (float)(cpu->flapsLeadingPrc + cpu->flapsTrailingPrc);

		if (simElapsedTimeSec >= this->simEngineStartTimeSec)
			simSound.playEngineSound(sndProcRec);

		simSound.playCabinSound(sndProcRec);
		simSound.playSystemSound(sndProcRec);
		simSound.playControlSound(sndProcRec);
		simSound.systemUpdate();
	}
}