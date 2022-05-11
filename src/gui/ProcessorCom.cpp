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
* Get/Set: replayRate
*/
void ProcessorCom::SetReplayRate(float num) {
	std::lock_guard<std::mutex> lockGuard(m);
	replayRate = num;
}

float ProcessorCom::GetReplayRate() {
	std::lock_guard<std::mutex> lockGuard(m);
	return replayRate;
}


/**
* Get/Set: simApi
*/
void ProcessorCom::SetSimConnect(bool state) {
	std::lock_guard<std::mutex> lockGuard(m);
	simConnect = state;
}

bool ProcessorCom::GetSimConnect() {
	std::lock_guard<std::mutex> lockGuard(m);
	return simConnect;
}


/**
* Get/Set: Options
*/
void ProcessorCom::SetOptions(OptionsStruct newOptions) {
	std::lock_guard<std::mutex> lockGuard(m);
	this->options.simApi = newOptions.simApi;
	this->options.halfRate = newOptions.halfRate;
	this->options.customSound = newOptions.customSound;
	this->options.beepStartEnd = newOptions.beepStartEnd;
	this->options.camShake = newOptions.camShake;
	this->options.depHH = newOptions.depHH;
	this->options.depMM = newOptions.depMM;
	this->options.depGMTHH = newOptions.depGMTHH;
	this->options.depGMTMM = newOptions.depGMTMM;
	this->options.destName = newOptions.destName;
	this->options.fltHH = newOptions.fltHH;
	this->options.fltMM = newOptions.fltMM;
	this->options.destGMTHH = newOptions.destGMTHH;
	this->options.destGMTMM = newOptions.destGMTMM;
}

OptionsStruct ProcessorCom::GetOptions() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options;
}

int ProcessorCom::GetSimApi() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.simApi;
}

bool ProcessorCom::GetHalfRate() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.halfRate;
}

bool ProcessorCom::GetCustomSound() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.customSound;
}

bool ProcessorCom::GetBeepStartEnd() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.beepStartEnd;
}

bool ProcessorCom::GetCamShake() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.camShake;
}

int ProcessorCom::GetDepHH() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.depHH;
}

int ProcessorCom::GetDepMM() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.depMM;
}

int ProcessorCom::GetDepGMTHH() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.depGMTHH;
}

int ProcessorCom::GetDepGMTMM() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.depGMTMM;
}

std::string ProcessorCom::GetDestName() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.destName;
}

int ProcessorCom::GetFltHH() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.fltHH;
}

int ProcessorCom::GetFltMM() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.fltMM;
}

int ProcessorCom::GetDestGMTHH() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.destGMTHH;
}

int ProcessorCom::GetDestGMTMM() {
	std::lock_guard<std::mutex> lockGuard(m);
	return this->options.destGMTMM;
}
