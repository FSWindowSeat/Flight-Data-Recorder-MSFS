
#pragma warning(disable : 4996)
#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include <locale.h>
#include <ProcessorUtil.h>
#include <FTDISFileStruct.h>
#include <fmod_studio.hpp>
#include <fmod.hpp>


/**
* Flight Data Recorder Ambient Sound Engine
*
* Ambient sound engine generates sounds during replay to improve the cabin imersion
*
* @author muppetlabs@fswindowseat.com
*/
class SimSoundEngine {

private:
	wstring xpPath, bankFilePath, stringFilePath;
	char loadFileName[1000];
	int gearPos = 1, flapsInTransit = 0;
	float flapsPos = 0;
	bool initEngineSnd = FALSE, initCabinSnd = FALSE, initSystemSnd = FALSE, initControlSnd = FALSE;
	FMOD::Studio::System* system;
	FMOD::Studio::Bank* masterBank, * stringsBank;
	FMOD::Studio::EventDescription* engineEvt, * cabinBackgEvt, * flapsWindEvt, * flapsTransitEvt, * gearRollEvt, * gearDownEvt, * gearUpEvt;
	FMOD::Studio::EventInstance* engineInst, * cabinBackgInst, * flapsWindInst, * flapsTransitInst, * gearRollInst, * gearDownInst, * gearUpInst;

public:

	/**
	* Constructor / Destructor
	*/
	SimSoundEngine();

	/**
	* Encapsulate the FMOD result type check
	*
	* @param	result	FMOD_Result type
	* @return   bool	FMOD result check
	*/
	//static bool ERRCHECK(const FMOD_RESULT result);

	/**
	* Initializes the sound stage
	*
	* @param	void
	* @return   void
	*/
	void initSoundStage(wstring bankFile, wstring stringFile);

	/**
	* Plays the engine sound
	*
	* @param	void
	* @return   void
	*/
	void playEngineSound(FTDISFileStruct ftdisRec);

	/**
	* Plays the cabin sound
	*
	* @param	void
	* @return   void
	*/
	void playCabinSound(FTDISFileStruct ftdisRec);

	/**
	* Plays the aircraft system sounds, e.g. gear
	*
	* @param	void
	* @return   void
	*/
	void playSystemSound(FTDISFileStruct ftdisRec);

	/**
	* Plays the aircraft control surface sounds, e.g. flaps and spoilers
	*
	* @param	void
	* @return   void
	*/
	void playControlSound(FTDISFileStruct ftdisRec);

	/**
	* This method updates the FMOD sound system. Required to run each frame.
	*
	* @param	void
	* @return   void
	*/
	void systemUpdate();
};

