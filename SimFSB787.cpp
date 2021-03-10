// Includes
#define _USE_MATH_DEFINES

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <cmath> 
#include <SimConnect.h>
#include <SimModelBehavior.cpp>

/**
* FTDIS Sim Model Behaviour for MS FS2020 Asobo B787
*
* Definition of sim specific behaviour for strategy pattern
*
* @author muppetlabs@fswindowseat.com
*/
class SimFSB787 : public SimModelBehavior
{
private:

	HANDLE  hSimConnect = NULL;

	bool revEngaged = false, splrEngaged = false, splrArmed = false, navL = false, beaconL = false, strobeL = false, landingL = false, taxiL = false,
		wingL = false, gear = true;

	int startAPUProcStep, startEngineProcStep, gearStat, signNoSmk, signSeat, signAtt, trackModelFct;

	float fuelStartL = 0.0, fuelStartC = 0.0, fuelStartR = 0.0, fuelEndL = 0.0, fuelEndC = 0.0, fuelEndR = 0.0, elapsedTimeDelta,
		switchInterval, startAPUTime, prevFlexL, prevFlexLIn, prevFlexR, prevFlexRIn, trackAlt, trackModelFctTime;

	float fuel[3];

	enum EVENT_ID {
		KEY_AILERON_SET,
		KEY_SPOILERS_ARM_SET,
		KEY_SPOILERS_SET,
		KEY_TOGGLE_NAV_LIGHTS,
		KEY_TOGGLE_BEACON_LIGHTS,
		KEY_STROBES_SET,
		KEY_LANDING_LIGHTS_SET,
		KEY_TOGGLE_TAXI_LIGHTS,
		KEY_TOGGLE_WING_LIGHTS,
		KEY_GEAR_SET,
		KEY_TOGGLE_STARTER2,
		KEY_ENGINE_AUTO_START,
		KEY_ENGINE_AUTO_SHUTDOWN
	};

	enum DATA_DEFINE_ID {
		THROTTLE_1,
		THROTTLE_2,
		AILERONS,
		FLAPS,
		SPOILER
	};

	struct controls {
		float throttle;
		float ailerons;
		int flaps;
		int spoiler;
	};

	controls flightCtrls = { 0.0, 0.0, 0, 0 };

	struct systems {
		int navLights;
		int beaconLights;
		int landingLights;
	};

	systems flightSyst = { 0,0,0 };

public:

	/**
	* Constructor
	*/
	SimFSB787() {
		// Set Aircraft Fuel Parameters
		fuelStartL = 10000.0F;
		fuelStartC = 50000.0F;
		fuelStartR = 10000.0F;
		fuelEndL = 1500.0F;
		fuelEndC = 1500.0F;
		fuelEndR = 1500.0F;

		// Set Aircraft geometry and control parameters
		this->acCntrAlt = 4.403881F;
		this->acFixedPitch = -0.767F;
		this->acFixedTrim = -0.39F;

		// Set Aircraft propulsion parameters
		this->acEngineIdle = 0.00F;
		this->acEngineMax = 95.0F;
	}


	/**
	* This method sets up a separate SimConnect call for the sim model's specific functions
	* and initializes all of the processing parameters
	*
	* @param	void
	* @return   void
	*/
	void performInitProcessing() {
		HRESULT hr;

		// Initialize processing vars
		startAPUProcStep = 0, startAPUTime = 0, startEngineProcStep = 0, gearStat = 1, signNoSmk = 0, signSeat = 0, signAtt = 0,
			elapsedTimeDelta = 0, switchInterval = 0, prevFlexL = 0.0, prevFlexLIn = 0.0, prevFlexR = 0.0, prevFlexRIn = 0.0,
			trackAlt = 0, trackModelFctTime = 0, trackModelFct = 0;

		// Create separate SimConnect call for sim model specific functions 
		if (SUCCEEDED(SimConnect_Open(&hSimConnect, "SimModelInterface", NULL, 0, 0, 0)))
		{
			// Add sim mode specific data definitions
			hr = SimConnect_AddToDataDefinition(hSimConnect, THROTTLE_1, "GENERAL ENG THROTTLE LEVER POSITION:1", "percent");
			hr = SimConnect_AddToDataDefinition(hSimConnect, THROTTLE_2, "GENERAL ENG THROTTLE LEVER POSITION:2", "percent");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_AILERON_SET, "AILERON_SET");
			hr = SimConnect_AddToDataDefinition(hSimConnect, FLAPS, "FLAPS HANDLE INDEX", "number");
			hr = SimConnect_AddToDataDefinition(hSimConnect, SPOILER, "SPOILERS HANDLE POSITION", "position");

			// System Controls: Map client events to sim events
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SPOILERS_ARM_SET, "SPOILERS_ARM_SET");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_SPOILERS_SET, "SPOILERS_SET");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_GEAR_SET, "GEAR_SET");

			// Light Controls: Map client events to sim events
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_NAV_LIGHTS, "TOGGLE_NAV_LIGHTS");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_BEACON_LIGHTS, "TOGGLE_BEACON_LIGHTS");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_STROBES_SET, "STROBES_SET");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_LANDING_LIGHTS_SET, "LANDING_LIGHTS_SET");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_TAXI_LIGHTS, "TOGGLE_TAXI_LIGHTS");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_WING_LIGHTS, "TOGGLE_WING_LIGHTS");

			// Engine Controls: Map client events to sim events
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_TOGGLE_STARTER2, "TOGGLE_STARTER2");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_ENGINE_AUTO_START, "ENGINE_AUTO_START");
			hr = SimConnect_MapClientEventToSimEvent(hSimConnect, KEY_ENGINE_AUTO_SHUTDOWN, "ENGINE_AUTO_SHUTDOWN");

		}
	}


	/**
	* Closes the separate SimConnect call for the sim model's specific functions
	*
	* @param	void
	* @return   void
	*/
	void performCloseProcessing() {
		HRESULT hr;

		hr = SimConnect_Close(hSimConnect);
	}


	/**
	* Calculates and sets the aircraft's fuel parameters
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @return   void
	*/
	void performSetFuel(float timeStmp, float flightTime) {
		fuel[0] = fuelStartC - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartC - fuelEndC));
		fuel[1] = fuelStartL - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartL - fuelEndL));
		fuel[2] = fuelStartR - ((1 - (flightTime - timeStmp) / flightTime) * (fuelStartR - fuelEndR));
	}


	/**
	* Runs through the aicraft's start APU procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @return   void
	*/
	void performStartAPUProc(float timeStmp, float startTime) {
		float apuSwitchDelay = 3, apuGenDelay = 50;

		// Set APU to ON
		if (startAPUProcStep == 0) {

			startAPUTime = timeStmp;
			startAPUProcStep = 1;
		}

		// Start APU
		if (startAPUProcStep == 1 && (timeStmp - startAPUTime) > apuSwitchDelay) {

			startAPUProcStep = 2;
		}

		// Wait for APU to start up and start generators
		if (startAPUProcStep == 2 && (timeStmp - startAPUTime) > apuGenDelay) {

			startAPUProcStep = 3;
		}

		if (startAPUProcStep == 3 && (timeStmp - startAPUTime) > (apuGenDelay + apuSwitchDelay)) {

		}
	}


	/**
	* Runs through the aircraft's startup procedure
	*
	* @param	timeStmp	Elapsed time in seconds
	* @param	startTime	Start time in seconds
	* @return   void
	*/
	void performStartEngineProc(float timeStmp, float startTime) {
		HRESULT hr;
		float stepDelay = 3, enginePreStartDelay = 15, enginePostStartDelay = 30;

		if (timeStmp >= startTime && startEngineProcStep <= 1) {

			// Engine no. 2: Start starter 2 to give engine 2 a head start for auto engine start (the best I can do at the moment)
			if (startEngineProcStep == 0) {
				hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_STARTER2, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				startEngineProcStep = 1;
			}

			// Engine no. 1: Run auto engine start procedure
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 1) && startEngineProcStep == 1) {
				hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_ENGINE_AUTO_START, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
				startEngineProcStep = 2;
			}

			/*
			// Start engine no. 2
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 1) && startEngineProcStep == 2) {

				startEngineProcStep = 3;
			}

			// Switch ignition no. 1 to Start
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 1) + (enginePostStartDelay * 1) && startEngineProcStep == 3) {

				startEngineProcStep = 4;
			}

			// Start engine no. 1
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 2) + (enginePostStartDelay * 1) && startEngineProcStep == 4) {

				startEngineProcStep = 5;
			}

			// Turn on Packs
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 2) + (enginePostStartDelay * 2) && startEngineProcStep == 5) {

				startEngineProcStep = 6;
			}

			// Turn on Engine generators
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 2) + (enginePostStartDelay * 2) && startEngineProcStep == 6) {

				startEngineProcStep = 7;
			}

			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 2) + (enginePostStartDelay * 2) && startEngineProcStep == 7) {

				startEngineProcStep = 8;
			}

			// Turn off APU
			if (timeStmp >= startTime + (stepDelay * startEngineProcStep) + (enginePreStartDelay * 2) + (enginePostStartDelay * 2) && startEngineProcStep == 8) {

				startEngineProcStep = 9;
			}
			*/

		}
	}


	/**
	* Runs through the stop engine procedure
	*
	* @param	void
	* @return   void
	*/
	void performStopEngineProc() {
		HRESULT hr;
		hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_ENGINE_AUTO_SHUTDOWN, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}


	/**
	* Converts the throttle value specified in the FTDIS input .fd file to the
	* corresponding value within the aircraft's throttle range
	*
	* @param	inputThrottle	Throttle value read from FTDIS input .ftd file
	* @return   float			Throttle value
	*/
	double performConvertThrottle(double inputThrottle) {
		double outputThrottle;

		if (inputThrottle <= -1)
			inputThrottle = (inputThrottle + 1) * -1;

		outputThrottle = (inputThrottle * (acEngineMax - acEngineIdle)) + acEngineIdle;

		return outputThrottle;
	}


	/**
	* Sets the aircraft's throttle
	*
	* @param	throttleVal		Throttle value
	* @return	void
	*/
	void performSetThrottle(double throttleVal) {
		HRESULT hr;
		double throttle = throttleVal;

		hr = SimConnect_SetDataOnSimObject(hSimConnect, THROTTLE_1, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(throttle), &throttle);
		hr = SimConnect_SetDataOnSimObject(hSimConnect, THROTTLE_2, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(throttle), &throttle);
	}


	/**
	* Sets the prop's feather, based on input throttle, flight phase, etc.
	* Used for props only
	*
	* @param	throttleVal		Throttle value
	* @param	fltPhase		Flight phase
	* @param	gear			Gear setting
	* @return	void
	*/
	void performSetProp(double throttleVal, int fltPhase, double gear) {

	}


	/**
	* Sets the aircraft's reversers
	*
	* @param	reverserSet		Reverser setting 0 = Off / 1 = On
	* @return	void
	*/
	void performSetReverser(int reverserSet) {
		if (reverserSet == 1) {

			revEngaged = true;
		}
		else if (reverserSet == 0) {

			revEngaged = false;
		}
	}


	/**
	* Sets the aircraft's ailerons
	*
	* @param	degrees		Aileron deflection in degrees
	* @return	void
	*/
	void performSetAileron(double degrees) {
		HRESULT hr;
		double fsAlrRng = 16383;
		double ailerons = degrees * fsAlrRng;

		hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_AILERON_SET, (DWORD)ailerons, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
	}


	/**
	* Sets the aircraft's elevator
	*
	* @param	degrees		Elevator deflection
	* @return	void
	*/
	void performSetElevator(double degrees) {

	}


	/**
	* Sets the aircraft's elevator trim
	*
	* @param	degrees		Elevator trim deflection
	* @return	void
	*/
	void performSetElevatorTrim(double degrees) {

	}


	/**
	* Sets the aircraft's rudder
	*
	* @param	degrees		Rudder deflection
	* @return	void
	*/
	void performSetRudder(double degrees) {

	}


	/**
	* Sets the aircrafts's flaps
	*
	* @param	stage		Flaps stage setting 1 to 5
	* @return	void
	*/
	void performSetFlaps(double stage) {
		HRESULT hr;
		double flapIndex = 0.0;

		switch ((int)stage) {
		case 1:
			flapIndex = 2.0;
			break;
		case 5:
			flapIndex = 3.0;
			break;
		case 10:
			flapIndex = 5.0;
			break;
		case 15:
			flapIndex = 7.0;
			break;
		case 25:
			flapIndex = 8.0;
			break;
		case 30:
			flapIndex = 9.0;
			break;
		default:
			flapIndex = 0.0;
		}

		hr = SimConnect_SetDataOnSimObject(hSimConnect, FLAPS, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(flapIndex), &flapIndex);
	}


	/**
	* Sets the aircraft's speed brakes / spoiler
	*
	* @param	spoilerSet		Spoiler setting 0 = Down / 1 = Mid / 2 = Up
	* @return	void
	*/
	void performSetSpoiler(double spoilerSet) {
		HRESULT hr;
		double spoiler;
		if ((int)spoilerSet == 1 && !splrArmed) {
			// Disengage
			if (splrEngaged) {
				spoiler = 0.0;
				hr = SimConnect_SetDataOnSimObject(hSimConnect, SPOILER, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(spoiler), &spoiler);
			}
		}
		else if ((int)spoilerSet == 2 && !splrEngaged) {
			spoiler = 1.0;
			// Ensure spoiler are disarmed
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SPOILERS_ARM_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			Sleep(1);
			// Extend spoiler
			hr = SimConnect_SetDataOnSimObject(hSimConnect, SPOILER, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(spoiler), &spoiler);
			//spoiler = 16383;
			//hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SPOILERS_SET, spoiler, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			splrEngaged = true;
			splrArmed = false;
		}
		else if ((int)spoilerSet == 0 && splrEngaged) {
			spoiler = 0.0;
			hr = SimConnect_SetDataOnSimObject(hSimConnect, SPOILER, SIMCONNECT_OBJECT_ID_USER, 0, 0, sizeof(spoiler), &spoiler);
			//hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_SPOILERS_SET, spoiler, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			splrEngaged = false;
		}
	}


	/**
	* Sets the aircrafts's gear
	*
	* @param	gearSet		Gear setting 1 = Down / 0 = Up
	* @return	void
	*/
	void performSetGear(double gearPos) {
		HRESULT hr;
		if ((int)gearPos == 1 && !gear) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_GEAR_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			gear = true;
		}
		else if ((int)gearPos == 0 && gear) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_GEAR_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			gear = false;
		}
	}


	/**
	* Sets the aircrafts's nose wheel deflection
	*
	* @param	noseWheelDegrees	Nose wheel deflection in degrees
	* @return	void
	*/
	void performSetNoseWheel(double noseWheelDegrees) {

	}


	/**
	* Sets the aircraft's brakes
	*
	* @param	brakeSet	Break setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetBrakes(double brakeSet) {

		/*if (brakeSet == 1)

		else if (brakeSet == 0)*/

	}


	/**
	* Sets the aircraft's navigation lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetNavLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !navL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_NAV_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			navL = true;
		}
		else if (lightSwitch == 0 && navL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_NAV_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			navL = false;
		}
	}


	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetBeaconLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !beaconL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_BEACON_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			beaconL = true;
		}
		else if (lightSwitch == 0 && beaconL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_BEACON_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			beaconL = false;
		}
	}


	/**
	* Sets the aircraft's beacon lights
	*
	* @param	lightSwitch		Switch setting 2 = On / 1 = Auto / 0 = Off
	* @return	void
	*/
	void performSetStrobeLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !strobeL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_STROBES_SET, 2, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			strobeL = true;
		}
		else if (lightSwitch == 0 && strobeL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_STROBES_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			strobeL = false;
		}
	}


	/**
	* Sets the aircraft's taxi lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetTaxiLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !taxiL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_TAXI_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			taxiL = true;
		}
		else if (lightSwitch == 0 && taxiL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_TAXI_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			taxiL = false;
		}
	}


	/**
	* Sets the aircraft's runway turnoff lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetRwyTurnoffLights(int lightSwitch) {
		/*
		if (lightSwitch == 1)
			XPLMSetDatai(acBeacL, 1);
		else if (lightSwitch == 0)
			XPLMSetDatai(acBeacL, 0);*/
	}


	/**
	* Sets the aircraft's wing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetWingLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !wingL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_WING_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			wingL = true;
		}
		else if (lightSwitch == 0 && wingL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_TOGGLE_WING_LIGHTS, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			wingL = false;
		}
	}


	/**
	* Sets the aircraft's landing lights
	*
	* @param	lightSwitch		Switch setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetLandingLights(int lightSwitch) {
		HRESULT hr;
		if (lightSwitch == 1 && !landingL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_LANDING_LIGHTS_SET, 1, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			landingL = true;
		}
		else if (lightSwitch == 0 && landingL) {
			hr = SimConnect_TransmitClientEvent(hSimConnect, SIMCONNECT_OBJECT_ID_USER, KEY_LANDING_LIGHTS_SET, 0, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
			landingL = false;
		}
	}


	/**
	* Sets the aircraft's cabin signs, as per the phase of flight
	*
	* @param	signSeatbelt		Fasten seatbelt sign setting 1 = On / 0 = Off
	* @param	signNoSmoke			No smoking sign setting 1 = On / 0 = Off
	* @param	signCabin			Cabin attendance sign setting 1 = On / 0 = Off
	* @return	void
	*/
	void performSetCabinSigns(float timeStmp, int signSeatbelt, int signNoSmoke, int signCabinAtt) {
		// Turn on no smoking sign


		// Turn on fasten seatbelt sign

		// Turn off fasten seatbelt sign


		// Trigger cabin attendance sign


	}


	/**
	* Sets the aircraft's altitude to the current barometric pressure
	*
	* @param	altitude	Aircraft's current altitude
	* @return	void
	*/
	void performSetAltimeter(double altitude) {
		float baroVal;

		if (altitude < 10000.0)
			baroVal = 0;
		else
			baroVal = 29.92F;

		/*
		XPLMSetDataf(acBaroL, baroVal);
		XPLMSetDataf(acBaroR, baroVal);*/
	}


	/**
	* Performs specific/unique model functions
	*
	* @param	timeStmp		Elapsed time in seconds
	* @param	flightTime		Total time of flight in seconds
	* @param	altitude		Aircraft's current altitude
	* @param	fltPhase		FTDIS flight phase
	* @return	void
	*/
	void performModelFunction(float timeStmp, float flightTime, double altitude, int fltPhase) {


	}


	/**
	* Defines and returns the model's camera view coordinates
	*
	* @param	camCoords		Camera view coordinates, phi, psi, the, x, y, z
	* @param	camSide			L/R
	* @param	camPos			F/B
	* @param	camVar			Iteration of specific L/R, F/B view
	* @return	void
	*/
	void getCamView(float camCoords[], char camSide, char camPos, int camVar) {

	}
};
