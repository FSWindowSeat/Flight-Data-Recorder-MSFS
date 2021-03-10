#pragma once

#include <fstream>
#include <vector>
#include <FTDISFileStruct.h>

class FTDISFile {

public:
	FTDISFile();
	~FTDISFile();

	int size;
	double depElev, destElev, flightElevAdj, flightTime;
	
	std::vector<FTDISFileStruct> fileData;

private:
	/**
	* Reads a vector of type FTDISFileStruct to the input stream
	* @param	is			Address of input stream
	* @param	vec			Address of vector of type FTDISFileStruct
	*/
	void ReadVecFields(std::istream& is, std::vector<FTDISFileStruct>& vec);

public:
	/**
	* Loads the .ftd file from a local directory
	* @return   bool
	*/
	bool LoadFile(std::string loadFileName);
};
