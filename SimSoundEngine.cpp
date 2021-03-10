#include <SimSoundEngine.h>

/**
* Constructor / Destructor
*/
SimSoundEngine::SimSoundEngine() {
}


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
* Initializes the sound stage
*/
void SimSoundEngine::initSoundStage(wstring bankFile, wstring stringFile) {
	
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
	
}


/**
* Plays the engine sound
*/
void SimSoundEngine::playEngineSound(FTDISFileStruct ftdisRec) {
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
	}
}


/**
* Plays the cabin sound
*/
void SimSoundEngine::playCabinSound(FTDISFileStruct ftdisRec) {
	// Initialize
	if (!initCabinSnd) {
		ERRCHECK(cabinBackgInst->start());
		initCabinSnd = TRUE;
	}

	// Update
	ERRCHECK(cabinBackgInst->setParameterByName("kts", (float)ftdisRec.spd));
}


/**
* Plays the aircraft system sounds, e.g. gear
*/
void SimSoundEngine::playSystemSound(FTDISFileStruct ftdisRec) {
	if (!initSystemSnd) {
		ERRCHECK(gearRollInst->start());
		ERRCHECK(gearUpInst->start());
		initSystemSnd = TRUE;
	}

	// Update
	ERRCHECK(gearRollInst->setParameterByName("kts", (float)ftdisRec.spd));
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


/**
* Plays the aircraft control sounds, e.g. flaps and spoilers
*/
void SimSoundEngine::playControlSound(FTDISFileStruct ftdisRec) {
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
		flapsTransitInst->start();
		flapsInTransit = 1;
	}
	else if (flapsPos == ftdisRec.flap && flapsInTransit == 1) {
		// Stop
		flapsTransitInst->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
		flapsInTransit = 0;
	}
	else
		// Continue
		flapsPos = (float)ftdisRec.flap;
}


/**
* Updates the FMOD sound system. Required to run each frame.
*/
void SimSoundEngine::systemUpdate() {
	ERRCHECK(system->update());
}

