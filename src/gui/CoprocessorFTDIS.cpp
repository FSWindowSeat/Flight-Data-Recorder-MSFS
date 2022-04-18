#include "CoprocessorFTDIS.h"
#include "Processor.h"
#include "ProcessorCom.h"

/**
* Constructor / Destructor
*/
CoprocessorFTDIS::CoprocessorFTDIS(Processor* handler, Track* track) {
	this->cpu = handler;
	this->cpuCom = handler->cpuCom;
	this->track = track;
	this->ftdisFile = track->GetFTDISFile();
	this->simAircraft = new SimModel();
	this->InitCoprocessor();
}

CoprocessorFTDIS::~CoprocessorFTDIS() {	
	// Close sim models
	delete simAircraft;
	delete simFSA320;
	delete simBBA332;
	delete simBBA343;
	delete simPAA388;
	delete simFSB787;

	// Close sound engine
	if(initSound)
		delete simSound;

	activeSimObject = false;
}


/**
* Initializes the coprocessor
*/
void CoprocessorFTDIS::InitCoprocessor() {
	
	// Assign flight model specific vars to data definitions
	switch (this->track->GetSimApi()) {
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
			//simBBA332 = new SimBBA332(cpu);
			//simAircraft->SetSimModelBehavior(simBBA332);
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
	
	// Init ground altitudes
	cpu->altGround[simAircraft->GetSimRequestId()] = ftdisFile->fileData.front().alt;
	prevDepAltGround = ftdisFile->fileData.front().alt;
	prevDestAltGround = ftdisFile->fileData.back().alt;

	// Init simulation vars
	simEngineStartTimeSec = 35;
}


/**
* Assigns the SimConnect object Id to corresponding Sim Model.
* Required for newly requested AI objects only.
*/
void CoprocessorFTDIS::SetSimObjectId(DWORD simObjectId) {
	simAircraft->InitSimObject(simObjectId);
	activeSimObject = true;
}


/**
* Returns the SimConnect object Id of tje corresponding Sim Model.
* Required for newly requested AI objects only.
*/
DWORD CoprocessorFTDIS::GetSimObjectID() {
	return simAircraft->GetSimObjectId();
}


/**
* Returns the models's SimConnect request ID
* Required for newly requested AI objects only.
*/
int CoprocessorFTDIS::GetSimRequestID() {
	return simAircraft->GetSimRequestId();
}


/**
* Assigns a track to the coprocessor
*/
void CoprocessorFTDIS::SetTrack(Track* track) {
	this->track = track;
}


/**
* Returns track assigned to the coprocessor
*/
Track* CoprocessorFTDIS::GetTrack() {
	return this->track;
}


/**
* Determines the position of the file cursor. The cursor points to the record
* of the flight data file to be read/processed  
*/
void CoprocessorFTDIS::SetCursorPos() {
		
	// Calc processor turnaround duration (in miliseconds)
	if (tFrameStart == std::chrono::steady_clock::time_point())
		tFrameStart = std::chrono::high_resolution_clock::now();

	tNow = std::chrono::high_resolution_clock::now();
	tFrameDeltaMs += std::chrono::duration<double, std::milli>(tNow - tFrameStart).count(); //tNow - tFrameStart;
	tFrameStart = std::chrono::high_resolution_clock::now();

	// Adjust for time offset
	if (this->track->GetTimeOffset() != 0 && this->simElapsedTimeSec == 0 && this->cursorPos == 0 && !timeOffsetAdjust) {
		this->simElapsedTimeSec += this->track->GetTimeOffset();
		timeOffsetAdjust = true;
	}
		
	// Update cursor position and aircraft object at ~100FPS
	if(tFrameDeltaMs >= 5 && cpuCom->GetReplayRate() != 0.0) {
		// Calculate sim elapsed time and determine cursor position
		this->simFrameTimeSec = tFrameDeltaMs / 1000;
		this->simElapsedTimeSec += simFrameTimeSec * cpuCom->GetReplayRate();
		
		this->cursorPos = (int)std::round(this->simElapsedTimeSec / this->targetPeriod);
	
		// Check for edge cases 
		if (this->cursorPos < 0)
			this->cursorPos = 0;
		else if (cursorPos > ftdisFile->size) {
			this->cursorPos = ftdisFile->size;

			if (this->track->GetTrackType() == Track::TrackType::AI) {
				// Remove object if at end of track and velocity, i.e. moving
				if (ftdisFile->fileData[this->cursorPos].spd >= 1)
					simAircraft->RemoveAiSimObject();

				activeSimObject = false;
			}
		}

		// Only run update if new cursor position is different from the previous one
		if (this->cursorPos != prevCursorPos) {
			track->SetCursorPos(this->cursorPos);
			
			if (track->GetTrackType() == Track::TrackType::USER)
				cpuCom->SetCursorPos(this->cursorPos);
			
			runUpdate = true;
		}
		else
			runUpdate = false;

		tFrameDeltaMs = 0; //std::chrono::milliseconds::zero();
		prevCursorPos = this->cursorPos;
	}
	// Pause
	else if(cpuCom->GetReplayRate() == 0.0)
		tFrameDeltaMs = 0; // std::chrono::milliseconds::zero();
	// Replay active but no update required
	else
		runUpdate = false;

	// Adjust sim rate at start/end of recording	
	if (cpuCom->GetHalfRate() && (ftdisFile->fileData[this->cursorPos].fltTime < runHalfRateSec || ftdisFile->fileData[this->cursorPos].fltTime >(ftdisFile->flightTime - runHalfRateSec))) {
		if (cpuCom->GetReplayRate() != 0.5F)
			cpuCom->SetReplayRate(0.5F);
	}
	else if (cpuCom->GetHalfRate()) {
		if (cpuCom->GetReplayRate() != 1.0F)
			cpuCom->SetReplayRate(1.0F);
	}
}


/**
* Sets the lateral position, vertical position and attitude of the aircraft
* in the sim
*/
void CoprocessorFTDIS::SetAircraftPos() {

	if(activeSimObject && runUpdate) {
		// Set position vars
		posSet.lat = ftdisFile->fileData[this->cursorPos].lat;
		posSet.lon = ftdisFile->fileData[this->cursorPos].lon;

		// Determine altitude
		if (ftdisFile->fileData[this->cursorPos].fltTime < 1800) {
			// Determine ground altitude (Departure)
			if (ftdisFile->fileData[this->cursorPos].alt == ftdisFile->depElev) {
				
				// ----------------------------------------------------------------------------------
				// TODO DEPRECATED -> Remove after testing period (10-21)
				// ----------------------------------------------------------------------------------
				// Check change in ground altitude and smooth large/sudden changes 
				//if (abs(cpu->altGround - this->prevDepAltGround) >= 0.01) {
				//	this->prevDepAltGround += (cpu->altGround - this->prevDepAltGround) / (1 / (this->targetPeriod * 2));
				//	posSet.alt = this->prevDepAltGround + simAircraft.getAcCntrAlt();
				//}
				//else
				posSet.alt = cpu->altGround[simAircraft->GetSimRequestId()] + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
				absGroundAlt = 0;
			}
			else {
				// Determine in-flight altitude
				posSet.alt = ftdisFile->fileData[this->cursorPos].alt + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
				absGroundAlt = posSet.alt - cpu->altGround[simAircraft->GetSimRequestId()];
			}

		}
		else if (ftdisFile->fileData[this->cursorPos].fltTime > ftdisFile->flightTime - 1800) {
			// Determine ground altitude (Destination)
			if (ftdisFile->fileData[this->cursorPos].alt == ftdisFile->destElev) {
				// ----------------------------------------------------------------------------------
				// TODO DEPRECATED -> Remove after testing period (10-21)
				// ----------------------------------------------------------------------------------
				// Check change in ground altitude and smooth large/sudden changes 
				//if (abs(cpu->altGround - this->prevDestAltGround) >= 0.01) {
				//	this->prevDestAltGround += (cpu->altGround - this->prevDestAltGround) / (1 / (this->targetPeriod * 2));
				//	posSet.alt = this->prevDestAltGround + simAircraft.getAcCntrAlt();
				//}
				//else
					posSet.alt = cpu->altGround[simAircraft->GetSimRequestId()] + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
					absGroundAlt = 0;
			}
			else{
				// Determine in-flight altitude
				posSet.alt = ftdisFile->fileData[this->cursorPos].alt + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
				absGroundAlt = posSet.alt - cpu->altGround[simAircraft->GetSimRequestId()];
			}
		}
		else {
			// Determine in-flight altitude
			posSet.alt = ftdisFile->fileData[this->cursorPos].alt + cpu->staticCGAlt[simAircraft->GetSimRequestId()];
			absGroundAlt = posSet.alt - cpu->altGround[simAircraft->GetSimRequestId()];
		}

		// Set attitude vars
		posSet.spd = ftdisFile->fileData[this->cursorPos].spd;
		posSet.hdg = ftdisFile->fileData[this->cursorPos].hdg;
		posSet.pitch = ftdisFile->fileData[this->cursorPos].pitch * -1 + cpu->staticCGPitch[simAircraft->GetSimRequestId()];
		posSet.bank = ftdisFile->fileData[this->cursorPos].bank * -1;
		
		simAircraft->SetPosition(posSet, Track::FileType::FTDIS);
		
		// Set simulation vars
		simSet.velZ = ftdisFile->fileData[this->cursorPos].spd;
		cursorOffSet = (int)std::round(1 / this->targetPeriod);
		if (this->cursorPos >= cursorOffSet)
			simSet.velY = (ftdisFile->fileData[this->cursorPos].alt - ftdisFile->fileData[this->cursorPos - cursorOffSet + 1].alt);
		
		simAircraft->SetSimData(simSet, Track::FileType::FTDIS);
	}
}


/**
* Sets the aircraft's flight systems, such as flight controls, propulsion and gear
*/
void CoprocessorFTDIS::SetAircraftSystems() {
	
	if (activeSimObject && runUpdate) {
		// Set fuel
		simAircraft->SetFuel((float)this->simElapsedTimeSec, (float)ftdisFile->flightTime, Track::FileType::FTDIS);

		// Start engine procedure during push back
		simAircraft->StartEngineProc((float)this->simElapsedTimeSec, (float)this->simEngineStartTimeSec, Track::FileType::FTDIS);

		// Stop engines after landing and taxi
		if (ftdisFile->fileData[this->cursorPos].fltPhase == 5 && ftdisFile->fileData[this->cursorPos].thr == -99 && !this->engineStopped) {
			this->thrtPos = -99;
			simAircraft->StopEngineProc(Track::FileType::FTDIS);
			this->engineStopped = TRUE;
		}
		else if (!this->engineStopped) {
			// Set aircraft throttle 
			this->n1ThrtVal = simAircraft->ConvertThrottle((float)ftdisFile->fileData[this->cursorPos].thr, Track::FileType::FTDIS);

			if (this->thrtPos < this->n1ThrtVal) {
				this->thrtPos += (100 / 10) * simFrameTimeSec;

				if (this->thrtPos > this->n1ThrtVal)
					this->thrtPos = this->n1ThrtVal;

			}
			else if (this->thrtPos > this->n1ThrtVal) {
				this->thrtPos -= (100 / 10) * simFrameTimeSec;

				if (this->thrtPos < this->n1ThrtVal)
					this->thrtPos = this->n1ThrtVal;
			}

			simAircraft->SetThrottle((float)this->thrtPos, Track::FileType::FTDIS);
		}

		// Set flight controls
		simAircraft->SetAileron(ftdisFile->fileData[this->cursorPos].alr, Track::FileType::FTDIS);
		simAircraft->SetFlaps(ftdisFile->fileData[this->cursorPos].flap, Track::FileType::FTDIS);

		// Set spoilers
		if ((this->cursorPos > ftdisFile->fileData.size() / 2 && ftdisFile->fileData[this->cursorPos].splr == 0))
			// Arm for landing
			simAircraft->SetSpoiler(1, Track::FileType::FTDIS);
		else
			simAircraft->SetSpoiler(ftdisFile->fileData[this->cursorPos].splr, Track::FileType::FTDIS);

		// Set gear			
		simAircraft->SetGear(ftdisFile->fileData[this->cursorPos].gear, Track::FileType::FTDIS);
		simAircraft->SetNoseWheel(ftdisFile->fileData[this->cursorPos].noseWhl, Track::FileType::FTDIS);

		// Set lights
		simAircraft->SetNavLights(ftdisFile->fileData[this->cursorPos].navL, Track::FileType::FTDIS);
		simAircraft->SetLogoLights(ftdisFile->fileData[this->cursorPos].logoL, Track::FileType::FTDIS);
		simAircraft->SetBeaconLights(ftdisFile->fileData[this->cursorPos].beacL, Track::FileType::FTDIS);
		simAircraft->SetStrobeLights(ftdisFile->fileData[this->cursorPos].strbL, Track::FileType::FTDIS);
		simAircraft->SetTaxiLights(ftdisFile->fileData[this->cursorPos].taxiL, Track::FileType::FTDIS);
		simAircraft->SetWingLights(ftdisFile->fileData[this->cursorPos].wngL, Track::FileType::FTDIS);
		simAircraft->SetLandingLights(ftdisFile->fileData[this->cursorPos].lndgL, Track::FileType::FTDIS);
		
	}
}

/**
* Adjusts and plays the ambience sound stage (FMOD)
*/
void CoprocessorFTDIS::SetSoundStage() {
	
	if (activeSimObject && runUpdate && cpuCom->GetCustomSound() && track->GetTrackType() == Track::TrackType::USER) {
		if (!initSound){
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
			
			if(cpuCom->GetBeepStartEnd())
				simSound->PlayStartReplaySound();
			
			initSound = TRUE;
		}
		
		FTDISFileStruct sndProcRec = ftdisFile->fileData[this->cursorPos];
		sndProcRec.thr = (float)thrtPos;
		sndProcRec.flap = (float)(cpu->flapsLeadingPrc[0] + cpu->flapsTrailingPrc[0]);
		sndProcRec.alt = absGroundAlt;

		if (simElapsedTimeSec >= this->simEngineStartTimeSec)
			simSound->PlayEngineSound(sndProcRec);

		simSound->PlayCabinSound(sndProcRec);
		simSound->PlaySystemSound(sndProcRec);
		simSound->PlayControlSound(sndProcRec);
		simSound->SystemUpdate();
	} else if (runUpdate && !cpuCom->GetCustomSound() && initSound) {
		delete simSound;
		initSound = FALSE;
	}
}