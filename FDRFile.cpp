#include <FDRFile.h>

FDRFile::FDRFile() {
	size = 0;
	flightTime = 0.0;
}

FDRFile::~FDRFile() {

}


/**
* Reads a vector of type FDRFileStruct to the input stream
*/
void FDRFile::ReadVecFields(std::istream& inputStream, std::vector<FDRFileStruct>& vec)
{
	typename std::vector<FDRFileStruct>::size_type size = 0;
	inputStream.read(reinterpret_cast<char*>(&size), sizeof(size));
	vec.resize((int)size);
	inputStream.read(reinterpret_cast<char*>(&vec[0]), vec.size() * sizeof(vec[0]));
}


/**
* Loads the .ftd file (FDR format) from a local directory
*/
bool FDRFile::LoadFile(std::string fileName) {

	// Read input file to memory
	std::ifstream in(fileName, std::ios::in | std::ios::binary);
	ReadVecFields(in, fileData);
	in.close();

	// Confirm load and initialize process variables
	if (fileData.size() > 0) {
		this->type = saveFile;
		return true;
	}
	else
		return false;
}


/**
* Saves the .fdr file to a local directory
*/
bool FDRFile::SaveFile(std::string fileName) {
	try {
		std::ofstream outputStream(fileName, std::ios::out | std::ios::binary);
		size_t sz = fileData.size();
		outputStream.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
		outputStream.write(reinterpret_cast<const char*>(&fileData[0]), fileData.size() * sizeof(fileData[0]));
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}


/**
* Add data record to .fdr file vector
*/
bool FDRFile::AddRecord(FDRFileStruct dataRec) {
	try {
		fileData.push_back(dataRec);
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
}


/**
* Returns the number of records of the .fdr file
*/
int FDRFile::GetSize() {
	if (fileData.size() > 0)
		return static_cast<int>(fileData.size()) - 1;
	else
		return 0;
}

/**
* Returns the total time (in s) of the flight tracked in the .fdr file
*/
double FDRFile::GetFlightTime() {
	if (fileData.size() > 0)
		return fileData.back().fltTime;
	else
		return 0;
}

/**
* Returns the file type
* (1) saveFile -> .fdr file loaded from disk
* (2) tmpFile -> tmp file for recording in progress
*/
FDRFile::FileType FDRFile::GetFileType() {
	return this->type;
}
