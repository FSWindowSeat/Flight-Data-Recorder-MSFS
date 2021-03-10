
#include <FTDISFile.h>

FTDISFile::FTDISFile() {

}

FTDISFile::~FTDISFile() {

}


/**
* Reads a vector of type FTDISFileStruct to the input stream
*/
void FTDISFile::ReadVecFields(std::istream& is, std::vector<FTDISFileStruct>& vec)
{
	typename std::vector<FTDISFileStruct>::size_type size = 0;
	is.read((char*)&size, sizeof(size));
	vec.resize((int)size);
	is.read((char*)&vec[0], vec.size() * sizeof(FTDISFileStruct));
}


/**
* Loads the .ftd file (FTDIS format) from a local directory
*/
bool FTDISFile::LoadFile(std::string loadFileName) {

	// Read input file to memory
	std::ifstream in(loadFileName, std::ios::in | std::ios::binary);
	ReadVecFields(in, fileData);
	in.close();

	// Confirm load and initialize process variables
	if (fileData.size() > 0) {

		size = static_cast<int>(fileData.size()) - 1;
		depElev = fileData.front().alt;
		destElev = fileData.back().alt;
		flightTime = fileData.back().fltTime;

		return true;
	}
	else
		return false;
}