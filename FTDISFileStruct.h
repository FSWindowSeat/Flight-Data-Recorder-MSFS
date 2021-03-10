#ifndef FTDISFILESTRUCT_H
#define FTDISFILESTRUCT_H

struct FTDISFileStruct
{
	double lat;
	double lon;
	double alt;
	double hdg;
	double spd;
	double pitch;
	double bank;
	double alr;
	double flap;
	double splr;
	double gear;
	double thr;
	double noseWhl;
	int navL;
	int logoL;
	int beacL;
	int strbL;
	int taxiL;
	int rwyTurnL;
	int wngL;
	int lndgL;
	int signSmk;
	int	signSeat;
	int signAtt;
	int fltPhase;
	float fltTime;
};

#endif