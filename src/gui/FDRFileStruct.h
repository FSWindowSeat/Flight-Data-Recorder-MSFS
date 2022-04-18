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
	double thr1;
	double thr2;
	double thr3;
	double thr4;
	double engN1E1;
	double engN1E2;
	double engN1E3;
	double engN1E4;
	double alr;
	double elev;
	double rudder;
	double flaps;
	double splr;
	double gearPos;
	double gearAngle;
	double fuelCtr;
	double fuelLeft;
	double fuelRight;
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