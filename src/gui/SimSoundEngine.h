#ifndef SimSoundEngine_H
#define SimSoundEngine_H
#define _WINSOCKAPI_ 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>
#include <locale.h>
#include "ProcessorUtil.h"
#include "FTDISFileStruct.h"
#include "fmod_studio.hpp"
#include "fmod.hpp"


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
	float flapsPos = 0, alt = 0;
	bool initEngineSnd = FALSE, initCabinSnd = FALSE, initSystemSnd = FALSE, initControlSnd = FALSE;
	FMOD::Studio::System* system;
	FMOD::Studio::Bank* masterBank, * stringsBank;
	FMOD::Studio::EventDescription* engineEvt, * cabinBackgEvt, * flapsWindEvt, * flapsTransitEvt, * gearRollEvt, * gearDownEvt, * gearUpEvt, * beepEvt;
	FMOD::Studio::EventInstance* engineInst, * cabinBackgInst, * flapsWindInst, * flapsTransitInst, * gearRollInst, * gearDownInst, * gearUpInst, * beepInst;

public:

	/**
	* Constructor / Destructor
	*/
	SimSoundEngine();
	~SimSoundEngine();

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
	* @param	bankFile, stringFile
	* @return   void
	*/
	void InitSoundStage(wstring bankFile, wstring stringFile);

	/**
	* Closes the sound stage
	*
	* @param	void
	* @return   void
	*/
	void CloseSoundStage();

	/**
	* Plays / stops the engine sound
	*
	* @param	void
	* @return   void
	*/
	void PlayEngineSound(FTDISFileStruct ftdisRec);
	void StopEngineSound();

	/**
	* Plays / stops the cabin sound
	*
	* @param	void
	* @return   void
	*/
	void PlayCabinSound(FTDISFileStruct ftdisRec);
	void StopCabinSound();

	/**
	* Plays / stops the aircraft system sounds, e.g. gear
	*
	* @param	void
	* @return   void
	*/
	void PlaySystemSound(FTDISFileStruct ftdisRec);
	void StopSystemSound();

	/**
	* Plays / stops the aircraft control surface sounds, e.g. flaps and spoilers
	*
	* @param	void
	* @return   void
	*/
	void PlayControlSound(FTDISFileStruct ftdisRec);
	void StopControlSound();

	/**
	* Plays start replay sound to signal start of replay
	*
	* @param	void
	* @return   void
	*/
	void PlayStartReplaySound();

	/**
	* This method updates the FMOD sound system. Required to run each frame.
	*
	* @param	void
	* @return   void
	*/
	void SystemUpdate();
};
#endif