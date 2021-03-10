#pragma once

#include <fstream>
#include <vector>
#include <FDRFileStruct.h>

class FDRFile {

public:
	FDRFile();
	~FDRFile();

	std::vector<FDRFileStruct> fileData;
	
	enum FileType {
		saveFile,
		tmpFile
	};

private:
	int size;
	double flightTime;

	FileType type = tmpFile;

	/**
	* Reads a vector of type FDRFileStruct to the input stream
	* @param	is			Address of input stream
	* @param	vec			Address of vector of type FDRFileStruct
	*/
	void ReadVecFields(std::istream& is, std::vector<FDRFileStruct>& vec);

public:
	/**
	* Loads the .fdr file from a local directory
	* @param	fileName		Path and name of file
	* @return   bool
	*/
	bool LoadFile(std::string fileName);

	/**
	* Saves the .fdr file to a local directory
	* @param	fileName		Path and name of file
	* @return   bool
	*/
	bool SaveFile(std::string fileName);

	/**
	* Loads the .fdr file from a local directory
	* @param	dataRec		Data record
	* @return   bool
	*/
	bool AddRecord(FDRFileStruct dataRec);

	/**
	* Returns the number of records of the .fdr file
	*/
	int GetSize();

	/**
	* Returns the total time (in s) of the flight tracked in the .fdr file
	*/
	double GetFlightTime();

	/**
	* Returns the file type 
	* (1) saveFile -> .fdr file loaded from disk
	* (2) tmpFile -> tmp file for recording in progress
	*/
	FileType GetFileType();
};