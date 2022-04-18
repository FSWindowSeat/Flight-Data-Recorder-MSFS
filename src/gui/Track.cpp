#include "Track.h"

/**
* Constructor / Destructor
*/
Track::Track(Track::TrackType trackType) {
	this->trackType = trackType;
	this->fileType = Track::FileType::NA;
	this->timeOffset = 0;
}

Track::~Track() {

}

/**
* Get/Set methods
*/
void Track::SetTrackId(int trackId) {
	this->trackId = trackId;
}

int Track::GetTrackId() {
	return this->trackId;
}


Track::TrackType Track::GetTrackType() {
	return this->trackType;
}


void Track::SetFileName(std::string fileName) {
	this->fileName = fileName;
}

std::string Track::GetFileName() {
	return this->fileName;
}


void Track::SetFileType(FileType fileType) {
	this->fileType = fileType;
}

Track::FileType Track::GetFileType() {
	return this->fileType;
}


void Track::SetSimApi(int simApi) {
	this->simApi = simApi;
}

int Track::GetSimApi() {
	return this->simApi;
}


void Track::SetSimCfgUuid(std::string simCfgUuid) {
	this->simCfgUuid = simCfgUuid;
}

std::string Track::GetSimCfgUuid() {
	return this->simCfgUuid;
}


void Track::SetSimTypeLivery(std::string simTypeLivery) {
	this->simTypeLivery = simTypeLivery;
}

std::string Track::GetSimTypeLivery() {
	return this->simTypeLivery;
}


void Track::SetTimeOffset(double offset) {
	this->timeOffset = offset;
}

double Track::GetTimeOffset() {
	return this->timeOffset;
}


void Track::SetCursorPos(int cursorPos) {
	this->cursorPos = cursorPos;
}

int Track::GetCursorPos() {
	return this->cursorPos;
}


void Track::SetFile(FDRFile *fdrFile) {
	this->fdrFile = fdrFile;
}

FDRFile* Track::GetFDRFile() {
	return this->fdrFile;
}

void Track::SetFile(FTDISFile *ftdisFile) {
	this->ftdisFile = ftdisFile;
}

FTDISFile* Track::GetFTDISFile() {
	return this->ftdisFile;
}

void Track::ClearFile() {
	if (ftdisFile) {
		delete this->ftdisFile;
		this->ftdisFile = NULL;
	}
	else if(fdrFile) {
		delete this->fdrFile;
		this->fdrFile = NULL;
	}
}

template<typename T> auto Track::GetFile() {

	if (fileType == Track::FileType::FDR)
		return this->fdrFile;
	else if (fileType == Track::FileType::FTDIS)
		return this->ftdisFile;

}