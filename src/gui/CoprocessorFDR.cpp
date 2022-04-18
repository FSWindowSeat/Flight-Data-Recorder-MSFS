#include "CoprocessorFDR.h"
#include "Processor.h"
#include "ProcessorCom.h"

/**
* Constructor / Destructor
*/
CoprocessorFDR::CoprocessorFDR(Processor* handler, Track* track) {
	cpu = handler;
	cpuCom = handler->cpuCom;
	this->track = track;
	fdrFile = track->GetFDRFile();
	simAircraft = new SimModel();
	this->InitCoprocessor();
}

CoprocessorFDR::~CoprocessorFDR() {
	// Close sim models
	delete simAircraft;
	delete simFSA320;
	delete simBBA332;
	delete simBBA343;
	delete simPAA388;
	delete simFSB787;
	//delete simAIA320;

	// Close sound engine
	if (initSound)
		delete simSound;

	activeSimObject = false;
}


/**
* Initializes the coprocessor
*/
void CoprocessorFDR::InitCoprocessor() {
	// Assign flight model specific vars to data definitions
	switch (track->GetSimApi()) {
	case 320:
		simFSA320 = new SimFSA320(cpu);
		simAircraft->SetSimModelBehavior(simFSA320);
		break;
	case 332:
		simBBA332 = new SimBBA332(cpu);
		simAircraft->SetSimModelBehavior(simBBA332);
		break;
	case 343:
		simBBA343 = new SimBBA343(cpu);
		simAircraft->SetSimModelBehavior(simBBA343);
		break;
	case 388:
		simPAA388 = new SimPAA388(cpu);
		simAircraft->SetSimModelBehavior(simPAA388);
		break;
	case 787:
		simFSB787 = new SimFSB787(cpu);
		simAircraft->SetSimModelBehavior(simFSB787);
		break;
	case 999:
		simFSA320 = new SimFSA320(cpu);
		simAircraft->SetSimModelBehavior(simFSA320);
		break;
	default:
		simFSA320 = new SimFSA320(cpu);
		simAircraft->SetSimModelBehavior(simFSA320);
	}
	
	if (track->GetTrackType() == Track::TrackType::USER) {
		this->SetSimObjectId(SIMCONNECT_OBJECT_ID_USER);
		activeSimObject = true;
	}	
	else if (track->GetTrackType() == Track::TrackType::AI)
		simAircraft->CreateAiSimObject(this->track);
}


/**
* Assigns the SimConnect object Id to corresponding Sim Model.
* Triggers intialization of the model as soon as objectID has been set
*/
void CoprocessorFDR::SetSimObjectId(DWORD simObjectId) {
	simAircraft->InitSimObject(simObjectId);
	activeSimObject = true;
}


/**
* Returns the SimConnect object Id of tje corresponding Sim Model.
* Required for newly requested AI objects only.
*/
DWORD CoprocessorFDR::GetSimObjectID() {
	return simAircraft->GetSimObjectId();
}


/**
* Returns the models's SimConnect request ID
* Required for newly requested AI objects only.
*/
int CoprocessorFDR::GetSimRequestID() {
	return simAircraft->GetSimRequestId();
}


/**
* Assigns a track to the coprocessor
*/
void CoprocessorFDR::SetTrack(Track* track) {
	this->track = track;
}


/**
* Returns track assigned to the coprocessor
*/
Track* CoprocessorFDR::GetTrack() {
	return this->track;
}


/**
* Determines the position of the file cursor. The cursor points to the record
* of the flight data file to be read/processed
*/
void CoprocessorFDR::SetCursorPos() {
	// Adjust for time offset
	if (this->track->GetTimeOffset() != 0 && this->simElapsedTimeSec == 0 && this->cursorPos == 0 && !timeOffsetAdjust) {
		this->simElapsedTimeSec += this->track->GetTimeOffset();
		timeOffsetAdjust = true;
	}
		
	// Check for end of main/user track 
	if (this->track->GetTrackType() == Track::TrackType::USER && this->simElapsedTimeSec == 0 && cpuCom->GetCursorPos() > 0) {
		this->simElapsedTimeSec = fdrFile->GetFlightTime();
		cursorPos = fdrFile->GetSize();
	}
	// Check for end of AI/traffic track
	else if (this->track->GetTrackType() == Track::TrackType::AI && this->cursorPos == fdrFile->GetSize()) {
		// Remove object if at end of track and velocity, i.e. moving
		if (fdrFile->fileData[this->cursorPos].spd >= 1)
			simAircraft->RemoveAiSimObject();
		
		activeSimObject = false;
	}
	// Calculate sim elapsed time
	else {
		this->simElapsedTimeSec += cpu->simFrameTimeSec * cpuCom->GetReplayRate();
	}

	// Determine cursor position
	if (cpuCom->GetReplayRate() > 0 && this->simElapsedTimeSec > 0) {
		while (this->cursorPos < fdrFile->GetSize() && fdrFile->fileData[this->cursorPos].fltTime < this->simElapsedTimeSec) {
			this->cursorPos += 1;
		}
	}
	else if (cpuCom->GetReplayRate() < 0 && this->simElapsedTimeSec > 0) {
		while (this->cursorPos > 0 && fdrFile->fileData[this->cursorPos].fltTime > this->simElapsedTimeSec) {
			this->cursorPos -= 1;
		}
	}

	track->SetCursorPos(cursorPos);
	
	if (this->track->GetTrackType() == Track::TrackType::USER)
		cpuCom->SetCursorPos(this->cursorPos);
	
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
	this->posSet.alt = fdrFile->fileData[this->cursorPos].alt + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
	this->posSet.spd = fdrFile->fileData[this->cursorPos].spd;
	this->posSet.hdg = fdrFile->fileData[this->cursorPos].hdg;
	this->posSet.pitch = fdrFile->fileData[this->cursorPos].pitch + cpu->staticCGPitch[simAircraft->GetSimRequestId()];
	this->posSet.bank = fdrFile->fileData[this->cursorPos].bank;
	
	if(activeSimObject)
		simAircraft->SetPosition(this->posSet, Track::FileType::FDR);
}


/**
* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
*/
void CoprocessorFDR::SetAircraftSystems(){
	
	// Set system vars: Propulsion
	simAircraft->StartAPUProc(fdrFile->fileData[this->cursorPos].fltTime, 10, Track::FileType::FDR);
	simAircraft->StartEngineProc(fdrFile->fileData[this->cursorPos].fltTime, 70, Track::FileType::FDR);
	simAircraft->SetThrottle(fdrFile->fileData[this->cursorPos].thr1, Track::FileType::FDR);
	
	// Set system vars: Flight Controls & Gear
	simAircraft->SetAileron(fdrFile->fileData[this->cursorPos].alr, Track::FileType::FDR);
	simAircraft->SetElevator(fdrFile->fileData[this->cursorPos].elev, Track::FileType::FDR);
	simAircraft->SetRudder(fdrFile->fileData[this->cursorPos].rudder, Track::FileType::FDR);
	simAircraft->SetFlaps(fdrFile->fileData[this->cursorPos].flaps, Track::FileType::FDR);
	simAircraft->SetSpoiler(fdrFile->fileData[this->cursorPos].splr, Track::FileType::FDR);
	simAircraft->SetGear(fdrFile->fileData[this->cursorPos].gearPos, Track::FileType::FDR);
	simAircraft->SetNoseWheel(fdrFile->fileData[this->cursorPos].gearAngle, Track::FileType::FDR);
	
	// Set system vars: Fuel (Distributes set amount of fuel over flight time)
	simAircraft->SetFuel(fdrFile->fileData[this->cursorPos].fltTime, fdrFile->fileData.back().fltTime, Track::FileType::FDR);

	// Set system vars: Lights
	simAircraft->SetNavLights(fdrFile->fileData[this->cursorPos].navL, Track::FileType::FDR);
	simAircraft->SetLogoLights(fdrFile->fileData[this->cursorPos].logoL, Track::FileType::FDR);
	simAircraft->SetBeaconLights(fdrFile->fileData[this->cursorPos].beacL, Track::FileType::FDR);
	simAircraft->SetStrobeLights(fdrFile->fileData[this->cursorPos].strbL, Track::FileType::FDR);
	simAircraft->SetTaxiLights(fdrFile->fileData[this->cursorPos].taxiL, Track::FileType::FDR);
	simAircraft->SetWingLights(fdrFile->fileData[this->cursorPos].wngL, Track::FileType::FDR);
	simAircraft->SetLandingLights(fdrFile->fileData[this->cursorPos].lndgL, Track::FileType::FDR);
}


/**
* Adjusts and plays the ambience sound stage (FMOD)
*/
void CoprocessorFDR::SetSoundStage() {

	if (activeSimObject && cpuCom->GetCustomSound() && track->GetTrackType() == Track::TrackType::USER) {
		if (!initSound) {
			simSound = new SimSoundEngine();

			// Assign flight model specific vars to data definitions
			switch (track->GetSimApi()) {
			case 320:
				simSound->InitSoundStage(L"A320NX.bank", L"A320NX.strings.bank");
				break;
			case 332:
				simSound->InitSoundStage(L"A332RR.bank", L"A332RR.strings.bank");
				break;
			case 343:
				simSound->InitSoundStage(L"A343.bank", L"A343.strings.bank");
				break;
			case 388:
				simSound->InitSoundStage(L"A388.bank", L"A388.strings.bank");
				break;
			case 787:
				simSound->InitSoundStage(L"B781.bank", L"B781.strings.bank");
				break;
			default:
				simSound->InitSoundStage(L"A320NX.bank", L"A320NX.strings.bank");
			}
			
			if (cpuCom->GetBeepStartEnd())
				simSound->PlayStartReplaySound();
			
			initSound = TRUE;
		}

		sndProcRec.alt = fdrFile->fileData[this->cursorPos].elev;
		sndProcRec.spd = fdrFile->fileData[this->cursorPos].spd;
		sndProcRec.flap = (float)(cpu->flapsLeadingPrc[simAircraft->GetSimRequestId()] + cpu->flapsTrailingPrc[simAircraft->GetSimRequestId()]);
		sndProcRec.splr = fdrFile->fileData[this->cursorPos].splr;
		sndProcRec.gear = fdrFile->fileData[this->cursorPos].gearPos;
		sndProcRec.thr = fdrFile->fileData[this->cursorPos].thr1;

		simSound->PlayCabinSound(sndProcRec);
		simSound->PlaySystemSound(sndProcRec);
		simSound->PlayControlSound(sndProcRec);
		simSound->SystemUpdate();
	}
	else if (!cpuCom->GetCustomSound() && initSound) {
		delete simSound;
		initSound = FALSE;
	}
}