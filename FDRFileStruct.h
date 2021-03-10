#ifndef FDRFILESTRUCT_H
#define FDRFILESTRUCT_H

struct FDRFileStruct
{
	double lat;
	double lon;
	double alt;
	double spd;
	double hdg;
	double pitch;
	double bank;
	double thr;
	double n1;
	double alr;
	double elev;
	double rudder;
	double flaps;
	double splr;
	double gear;
	int navL;
	int logoL;
	int beacL;
	int strbL;
	int taxiL;
	int rwyTurnL;
	int wngL;
	int lndgL;
	float fltTime;
};

#endif