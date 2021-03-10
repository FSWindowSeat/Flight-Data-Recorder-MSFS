# Flight-Data-Recorder-MSFS
Flight Data Recorder for Microsoft Flight Simulator 2020

## Introduction

The Flight Data Recorder enables you to record, save and replay flights in Microsoft Flight Simulator 2020 ©. It is a standalone application that opens in its own window and uses the SimConnect API to communicate with the flight simulator.

## Installation & Start

Flight Data Recorder is a standalone application. Just unzip the zip file to a folder of your choice.

Open the folder and double click on the FlightDataRecorder.exe file to start the application

Please refer to the help documentation that you can access from within the application. Just select Help -> View Help from the menu. 

## Limitations
 
This is an alpha version and at this point in time the Flight Data Recorder records the following data only,
- Aircraft position, speed and attitude
- Flight Controls, including ailerons, elevators, flaps and rudder
- Throttle 

The recording of advanced flight systems, e.g. FMC, engine controls for engine start, APU and aircraft lights aren't supported at this point in time. All of these controls are not easily accessible via the SimConnect API and require more work, others, such as the engine and APU controls, aren't supported at all at this point in time.

Please ensure that you're using the same aircraft and that your aircraft is at the same position, as it was at the start of your original recording. If you're using a different aircraft, or your positioned at a different airport, the replay might not work correctly.

The Flight Data Recorder records flight data at the sim's speed. Hence, if you record your flight at, e.g. 20, FPS you will notice some stutter at times during the replay. There's no functionality included to interpolate lower resolution data for higher FPS.  

The fast forward and rewind speeds are limited to 16x. Although it'd be possible to forward/rewind a recording at higher speeds it turned out to be unstable and might crash the sim, as it cannot keep up with loading the scenery.
