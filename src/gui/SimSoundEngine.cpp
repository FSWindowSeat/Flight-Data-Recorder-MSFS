#include "SimSoundEngine.h"


/**
* Encapsulate the FMOD result type check
*/
static bool ERRCHECK(const FMOD_RESULT result)
{
	if (result != FMOD_OK)
		return false;

	return true;
}


/**
* Constructor / Destructor
*/
SimSoundEngine::SimSoundEngine() {
}

 SimSoundEngine::~SimSoundEngine() {
	ERRCHECK(cabinBackgInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(cabinBackgInst->release());
	 
	ERRCHECK(engineInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(engineInst->release());

	ERRCHECK(flapsWindInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(flapsWindInst->release());

	ERRCHECK(flapsTransitInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(flapsTransitInst->release());

	ERRCHECK(gearRollInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(gearRollInst->release());

	ERRCHECK(gearDownInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(gearDownInst->release());

	ERRCHECK(gearUpInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	ERRCHECK(gearUpInst->release());

	ERRCHECK(masterBank->unload());

	ERRCHECK(system->release());
}


/**
* Initializes the sound stage
*/
void SimSoundEngine::InitSoundStage(wstring bankFile, wstring stringFile) {
	
	// Create and initialize FMOD system
	ERRCHECK(FMOD::Studio::System::create(&system));
	ERRCHECK(system->initialize(64, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL));

	// Define path and file references
	xpPath = ProcessorUtil::getExePath();
	std::replace(xpPath.begin(), xpPath.end(), '\\', '/');
	bankFilePath = xpPath + L"/" + bankFile;
	stringFilePath = xpPath + L"/" + stringFile;

	// Load banks
	std::wcstombs(loadFileName, bankFilePath.c_str(), sizeof loadFileName);
	ERRCHECK(system->loadBankFile(loadFileName, FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank));

	std::wcstombs(loadFileName, stringFilePath.c_str(), sizeof loadFileName);
	ERRCHECK(system->loadBankFile(loadFileName, FMOD_STUDIO_LOAD_BANK_NORMAL, &stringsBank));

	// Initialize sounds
	ERRCHECK(system->getEvent("event:/engineFull", &engineEvt));
	ERRCHECK(engineEvt->createInstance(&engineInst));

	ERRCHECK(system->getEvent("event:/cabinBckg", &cabinBackgEvt));
	ERRCHECK(cabinBackgEvt->createInstance(&cabinBackgInst));

	ERRCHECK(system->getEvent("event:/flapsWind", &flapsWindEvt));
	ERRCHECK(flapsWindEvt->createInstance(&flapsWindInst));

	ERRCHECK(system->getEvent("event:/flapsTransit", &flapsTransitEvt));
	ERRCHECK(flapsTransitEvt->createInstance(&flapsTransitInst));

	ERRCHECK(system->getEvent("event:/gearRoll", &gearRollEvt));
	ERRCHECK(gearRollEvt->createInstance(&gearRollInst));

	ERRCHECK(system->getEvent("event:/gearDown", &gearDownEvt));
	ERRCHECK(gearDownEvt->createInstance(&gearDownInst));

	ERRCHECK(system->getEvent("event:/gearUp", &gearUpEvt));
	ERRCHECK(gearUpEvt->createInstance(&gearUpInst));

	ERRCHECK(system->getEvent("event:/beep", &beepEvt));
	ERRCHECK(beepEvt->createInstance(&beepInst));
	
}



/**
* Plays / stops the engine sound
*/
void SimSoundEngine::PlayEngineSound(FTDISFileStruct ftdisRec) {
	if (!initEngineSnd) {
		ERRCHECK(engineInst->start());
		initEngineSnd = TRUE;
	}

	// Stop
	if (ftdisRec.thr == -99) {
		engineInst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		// Update
	}
	else {
		ERRCHECK(engineInst->setParameterByName("n1", (float)ftdisRec.thr));
		ERRCHECK(engineInst->setParameterByName("kts", (float)ftdisRec.spd));
		ERRCHECK(engineInst->setParameterByName("alt", (float)ftdisRec.alt));
	}
}


/**
* Plays / stops the cabin sound
*/
void SimSoundEngine::PlayCabinSound(FTDISFileStruct ftdisRec) {
	// Initialize
	if (!initCabinSnd) {
		ERRCHECK(cabinBackgInst->start());
		initCabinSnd = TRUE;
	}

	// Update
	ERRCHECK(cabinBackgInst->setParameterByName("kts", (float)ftdisRec.spd));
}

void SimSoundEngine::StopCabinSound() {
	// Stop
	if (initCabinSnd) {
		ERRCHECK(cabinBackgInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		initCabinSnd = FALSE;
	}
}


/**
* Plays / stops the aircraft system sounds, e.g. gear
*/
void SimSoundEngine::PlaySystemSound(FTDISFileStruct ftdisRec) {
	if (!initSystemSnd) {
		ERRCHECK(gearRollInst->start());
		ERRCHECK(gearUpInst->start());
		initSystemSnd = TRUE;
	}

	// Update
	ERRCHECK(gearRollInst->setParameterByName("kts", (float)ftdisRec.spd));
	ERRCHECK(gearRollInst->setParameterByName("alt", (float)ftdisRec.alt));
	ERRCHECK(gearUpInst->setParameterByName("kts", (float)ftdisRec.spd));
	ERRCHECK(gearUpInst->setParameterByName("n1", (float)ftdisRec.thr));

	// Gear: Trigger start / end events
	if (gearPos == 1 && gearPos != ftdisRec.gear) {
		ERRCHECK(gearUpInst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
		ERRCHECK(gearRollInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		gearPos = 0;
	}
	else if (gearPos == 0 && gearPos != ftdisRec.gear) {
		ERRCHECK(gearDownInst->start());
		ERRCHECK(gearRollInst->start());
		gearPos = 1;
	}
}

void SimSoundEngine::StopSystemSound() {
	// Stop
	if (initSystemSnd) {
		ERRCHECK(gearRollInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		ERRCHECK(gearUpInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		initSystemSnd = FALSE;
	}
}


/**
* Plays / stops  the aircraft control sounds, e.g. flaps and spoilers
*/
void SimSoundEngine::PlayControlSound(FTDISFileStruct ftdisRec) {
	if (!initControlSnd) {
		ERRCHECK(flapsWindInst->start());
		initControlSnd = TRUE;
	}

	// Update
	ERRCHECK(flapsWindInst->setParameterByName("kts", (float)ftdisRec.spd));
	ERRCHECK(flapsWindInst->setParameterByName("flapsPrc", (float)ftdisRec.flap * 0.5F));

	// Flaps movement: Trigger start / end events
	if (flapsPos != ftdisRec.flap && flapsInTransit == 0) {
		// Start
		ERRCHECK(flapsTransitInst->start());
		flapsInTransit = 1;
	}
	else if (flapsPos == ftdisRec.flap && flapsInTransit == 1) {
		// Stop
		ERRCHECK(flapsTransitInst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
		flapsInTransit = 0;
	}
	else
		// Continue
		flapsPos = (float)ftdisRec.flap;
}

void SimSoundEngine::StopControlSound() {
	if (initControlSnd) {
		ERRCHECK(flapsWindInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		ERRCHECK(flapsTransitInst->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		initControlSnd = FALSE;
	}
}


/**
* Plays start replay sound to signal start of replay
*/
void SimSoundEngine::PlayStartReplaySound() {
	ERRCHECK(beepInst->start());
	ERRCHECK(beepInst->release());
}


/**
* Updates the FMOD sound system. Required to run each frame.
*/
void SimSoundEngine::SystemUpdate() {
	ERRCHECK(system->update());
}

