#ifndef Track_H
#define Track_H

#include "FDRFile.h"
#include "FTDISFile.h"

/**
* Represents a Input File for Replay in the Processor
*
* @author muppetlabs@fswindowseat.com
*/
class Track {

public: 
	int trackId;

	enum TrackType {
		USER,
		AI
	};

	enum FileType {
		NA,
		FTDIS,
		FDR
	};

	Track(TrackType trackType);
	~Track();

private:
	TrackType trackType;
	int cursorPos;
	int simApi;
	double timeOffset;
	std::string simCfgUuid;
	std::string simTypeLivery;
	std::string fileName;
	FileType fileType;
	FDRFile *fdrFile;
	FTDISFile *ftdisFile;
	
public:

	/**
	* Get/Set methods
	*/
	void SetTrackId(int trackId);
	int GetTrackId();
	
	TrackType GetTrackType();

	void SetFileName(std::string fileName);
	std::string GetFileName();

	void SetFileType(FileType fileType);
	FileType GetFileType();

	void SetFile(FDRFile *fdrFile);
	FTDISFile* GetFTDISFile();

	void SetFile(FTDISFile *ftdisFile);
	FDRFile* GetFDRFile();

	void ClearFile();

	void SetSimApi(int simApi);
	int GetSimApi();

	void SetSimCfgUuid(std::string simCfgUuid);
	std::string GetSimCfgUuid();

	void SetSimTypeLivery(std::string simCfgUuid);
	std::string GetSimTypeLivery();

	void SetTimeOffset(double offSet);
	double GetTimeOffset();

	void SetCursorPos(int cursorPos);
	int GetCursorPos();
	
	template<typename T> auto GetFile();
	
};
#endif