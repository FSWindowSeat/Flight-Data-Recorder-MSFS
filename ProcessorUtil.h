#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

// Define name spaces
using namespace std;
namespace fs = std::filesystem;

/**
* Custom defined name space to collect all of the utility functions required for the Main Processor
*
* @author muppetlabs@fswindowseat.com
*/
namespace ProcessorUtil
{
	/**
	* Converts values in Radians to Degrees
	*
	* @param	radians		Value in Rad
	* @return   degrees		Value in Degrees
	*/
	double toDegrees(double radians);


	/**
	* Converts values in Degress to Radians
	*
	* @param	radians		Value in Degrees
	* @return   degrees		Value in Radians
	*/
	double toRadians(double degrees);


	/**
	* Converts metres to feet
	*
	* @param	metres		Value in metres
	* @return   feet		Value in feet
	*/
	double toFeet(double metres);


	/**
	* Converts feet to metres
	*
	* @param	feet		Value in feet
	* @return   metres		Value in metres
	*/
	double toMetres(double feet);


	/**
	* Returns a file's number of lines
	*
	* @param	filePathName	File path and name
	* @return					Number of lines
	*/
	int fileLength(fs::path filePathName);


	/**
	* Returns the path to the executable
	*
	* @return					Path to executable
	*/
	wstring getExePath();


	/**
	* Returns a random number within a given range
	*
	* @param	lo		Lower end of number range
	* @param	hi		Upper end of number range
	* @return			Random number within lo / hi range
	*/
	double randRange(double lo, double hi);


	/**
	* Returns a sin value, based upon the corresponding wave length and amplitude
	*
	* @param	timeStmp		Time stamp in seconds
	* @param	waveLength		Wave length in seconds
	* @return	amplitude		Amplitude 0 - 1
	*/
	float getSinVal(float timeStmp, float waveLength, float amplitude);

	/**
	* Converts knots to metres per second
	*
	* @param	kts		Speed in knots
	* @return	ms		Speed in metres per second
	*/
	float ktsToMs(float kts);

	/**
	* Returns the current date and time. Format dd/mm/yyyy hh:mm:ss
	*
	* @param
	* @return   string		Date and time. Format dd/mm/yyyy hh:mm:ss
	*/
	std::string getDateTime();
}