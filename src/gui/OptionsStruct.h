#ifndef OptionsStruct_H
#define OptionsStruct_H

#include <string>

struct OptionsStruct
{
	int simApi;
	bool halfRate;
	bool customSound;
	bool beepStartEnd;
	bool camShake;
	int depHH;
	int depMM;
	int depGMTHH;
	int depGMTMM;
	std::string destName;
	int fltHH;
	int fltMM;
	int destGMTHH;
	int destGMTMM;
};
#endif