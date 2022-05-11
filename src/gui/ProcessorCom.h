#ifndef ProcessorCom_H
#define ProcessorCom_H

#include <mutex>
#include <string>
#include "OptionsStruct.h"

/**
* Communication Interface between GUI and Processor
*
* Allows to exchange information between main thread (GUI) and detached thread (Processor).
* Includes safe guards to share properties between the two threads safely 
* 
* @author muppetlabs@fswindowseat.com
*/
class ProcessorCom {

public:
	ProcessorCom();
	~ProcessorCom();
	
	enum ProcessorState {
		stop,
		replay,
		record
	};

private:
	ProcessorState processorState = stop;
	int cursorPos = 0;
	bool simConnect = false;
	float replayRate = 0.0F;
	std::mutex m;
	OptionsStruct options = {320,false,false,false,false,0,0,0,0,"",0,0,0,0};

public:
	
	/**
	* Get/Set methods global vars
	*/
	void SetProcessorState(ProcessorState state);
	ProcessorState GetProcessorState();

	void SetCursorPos(int num);
	int GetCursorPos();
	
	void SetReplayRate(float num);
	float GetReplayRate();
	
	void SetSimConnect(bool state);
	bool GetSimConnect();

	void SetOptions(OptionsStruct options);
	OptionsStruct GetOptions();
	
	int GetSimApi();

	bool GetHalfRate();

	bool GetCustomSound();
	
	bool GetBeepStartEnd();
	
	bool GetCamShake();

	int GetDepHH();
	
	int GetDepMM();
	
	int GetDepGMTHH();
	
	int GetDepGMTMM();

	std::string GetDestName();
	
	int GetFltHH();
	
	int GetFltMM();
	
	int GetDestGMTHH();

	int GetDestGMTMM();
};
#endif