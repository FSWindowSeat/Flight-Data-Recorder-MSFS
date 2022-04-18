#include "ProcessorUtil.h"

namespace ProcessorUtil
{

	/**
	* Converts values in Radians to Degrees
	*/
	double ProcessorUtil::toDegrees(double radians) {
		return radians * (180.0F / M_PI);
	}


	/**
	* Converts values in Degress to Radians
	*/
	double ProcessorUtil::toRadians(double degrees) {
		return degrees * (M_PI / 180.0F);
	}


	/**
	* Converts metres to feet
	*/
	double ProcessorUtil::toFeet(double metres) {
		return metres * 3.28084F;
	}


	/**
	* Converts feet to metres
	*/
	double ProcessorUtil::toMetres(double feet) {
		return feet / 3.28084F;
	}


	/**
	* Returns a file's number of lines
	*/
	int ProcessorUtil::fileLength(fs::path filePathName) {
		int rowCount = 0;
		string line;

		ifstream checkFile(filePathName);

		while (getline(checkFile, line))
			++rowCount;

		return rowCount;
	}


	/**
	* Returns the path to the executable
	*/
	wstring ProcessorUtil::getExePath() {
		TCHAR buffer[MAX_PATH] = {0};

		GetModuleFileName(NULL, buffer, MAX_PATH);

		wstring::size_type pos = wstring(buffer).find_last_of(L"\\/");

		return wstring(buffer).substr(0, pos);
	}


	/**
	* Returns a random number within a given range
	*/
	double ProcessorUtil::randRange(double lo, double hi) {
		return lo + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (hi - lo)));
	}


	/**
	* Returns a sin value, based upon the corresponding wave length and amplitude
	*/
	float ProcessorUtil::getSinVal(float timeStmp, float waveLength, float amplitude) {
		return amplitude * (float)sin(timeStmp / waveLength * M_PI);
	}


	/**
	* Converts knots to metres per second
	*/
	float ProcessorUtil::ktsToMs(float kts) {
		return kts * 0.51444444444F;
	}


	/**
	* Returns the current date and time. Format dd/mm/yyyy hh:mm:ss
	*/
	std::string ProcessorUtil::getDateTime() {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];

		localtime_s(&tstruct, &now);

		strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct); // Date/time format http://en.cppreference.com/w/cpp/chrono/c/strftime

		return buf;
	}
}

