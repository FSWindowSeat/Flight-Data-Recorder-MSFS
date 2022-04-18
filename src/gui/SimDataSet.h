#ifndef SIMDATASET_H
#define SIMDATASET_H

struct SimDataSet
{
	double velZ;
	double velY;
	double flapsLeadingPrc;
	double flapsTrailingPrc;
	double zulu;
	double groundAlt;
	double absGroundAlt;
	double staticCGAlt;
	double staticCGPitch;
	double simCamPosX;
	double simCamPosY;
};
#endif