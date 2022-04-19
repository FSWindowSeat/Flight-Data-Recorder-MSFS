# Flight-Data-Recorder-MSFS
Flight Data Recorder for Microsoft Flight Simulator 2020

## Introduction

The Flight Data Recorder enables you to record, save and replay flights in Microsoft Flight Simulator 2020 ©. It is a standalone application that opens in its own window and uses the SimConnect API to communicate with the flight simulator.

Preview video https://www.youtube.com/watch?v=yjAz1RRzy9E

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

## How to Build from Source
### Tool Chain
The code was written in Visual Studio 2022 and the MSVC 14.30.30705 compiler on Windows. To build the application from the source code and to reuse the CMake files you'll require the following tool chain,
- Visual Studio 2022 https://visualstudio.microsoft.com/downloads/
- Buildtools for Visual Studio 2022 https://visualstudio.microsoft.com/downloads/

### Dependencies
The source code has various dependencies. All of them are part of the git repository and referenced in the Visual Studio Solution file
- VCPKG: zlib, openssl, boost-system, boost-date-time, boost-regex, boost-interprocess, websocketpp and brotli packages (required for CPPREST SDK)
- CPPREST SDK https://github.com/microsoft/cpprestsdk
- WxWidgets https://github.com/wxWidgets/wxWidgets.git
- SQLite3 https://www.sqlite.org/index.html
- FMOD API https://www.fmod.com/download
- SimConnect SDK
- Windows 10 SDK (10.0.19041.0)

## License

Flight Data Recorder for MS FS2020
Copyright (C) 2022 FSWindowSeat <muppetlabs@fswindowseat.com>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.




