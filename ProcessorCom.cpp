#include "ProcessorCom.h"


/**
* Constructor / Destructor
*/
ProcessorCom::ProcessorCom() {

}

ProcessorCom::~ProcessorCom() {

}


/**
* Get/Set: cursorPos
*/

void ProcessorCom::SetProcessorState(ProcessorState state) {
	std::lock_guard<std::mutex> lockGuard(m);
	this->processorState = state;
}

ProcessorCom::ProcessorState ProcessorCom::GetProcessorState() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->processorState;
}


/**
* Get/Set: cursorPos
*/
void ProcessorCom::SetCursorPos(int num) {
	std::lock_guard<std::mutex> lockGuard(m);
	this->cursorPos = num;
}

int ProcessorCom::GetCursorPos() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->cursorPos;
}


/**
* Get/Set: simRate
*/
void ProcessorCom::SetSimRate(float num) {
	std::lock_guard<std::mutex> lockGuard(m);
	simRate = num;
}

float ProcessorCom::GetSimRate() {
	std::lock_guard<std::mutex> lockGuard(m);
	return simRate;
}


/**
* Get/Set: simApi
*/
void ProcessorCom::SetSimAPI(bool state) {
	std::lock_guard<std::mutex> lockGuard(m);
	simAPI = state;
}

bool ProcessorCom::GetSimAPI() {
	std::lock_guard<std::mutex> lockGuard(m);
	return simAPI;
}

