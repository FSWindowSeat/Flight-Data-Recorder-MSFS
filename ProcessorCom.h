#pragma once

#include <mutex>

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
	bool simAPI = false;
	float simRate = 0.0F;
	std::mutex m;

public:
	
	/**
	* Get/Set methods global vars
	*/
	void SetProcessorState(ProcessorState state);
	ProcessorState GetProcessorState();

	void SetCursorPos(int num);
	int GetCursorPos();
	
	void SetSimRate(float num);
	float GetSimRate();

	void SetSimAPI(bool state);
	bool GetSimAPI();
};

