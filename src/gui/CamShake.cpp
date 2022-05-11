#include "CamShake.h"
#include "GuiMain.h"

/**
* Constructor / Destructor
*/
CamShake::CamShake(GuiMain* guiMain){
    // Set handler / references
    this->guiMain = guiMain;
    // @todo Add version number to config db
    this->windowName = L"Microsoft Flight Simulator - 1.25.9.0";
    this->window = FindWindowW(NULL, windowName);
    
    // Initialize camera
    this->InitCam();
}


/**
* Initializes camera and sets default values.  
*
* @return       Return code: 0 successful, 1 failed to run
*/
int CamShake::InitCam(){

    // Load default config vars from db
    if(LoadCamShakeConfig() == 1)
        return 1;

    // Set default values, i.e. pixel/degree
    this->camUpdateCycleT = 0.01f; // 10 ms

    // Initialize Simplex Noise for each axis
    unsigned int seed = 123456;
    this->camShakeX.ReSeed(seed);
    this->camShakeY.ReSeed(seed + 1);
    this->camShakeZ.ReSeed(seed + 2);
    
    this->initCam = true;
    return 0;
}


/**
* Set camera position viewpoint (in MSFS 2020 main application window)
* 
* @return       Return code: 0 successful, 1 camera not initialized, 2 min time between updates not reached, 3 MSFS window not in focus
*/
int CamShake::SetCameraPos(CamShakeStruct cpuData, float &camX, float &camY, float &camZ) {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    // Catch: Camera not initialized
    if(!this->initCam)
        return 1;
    
    // Catch: Insufficient CPU data and/or min time between updates not reached
    if(cpuData.fltTime <= 0 || cpuData.fltTime - this->camLastUpdateT < this->camUpdateCycleT) {
        //resetMouse();
        return 2;
    }
  
    // Catch: MSFS 2020 window not in focus 
    if (GetForegroundWindow() != window) {
        //resetMouse(); 
        return 3;
    }
        

    // Send mouse movement instructions to window to set camera position
    GetWindowRect(window, &winPos);
    //@note X/Y axis limits in degrees and absolute (3 degs), as there's no perfect 1:1 relationship between mouse input and camera movement
    if (abs(cpuData.simCamPosX) <= 3 && abs(cpuData.simCamPosY) <= 3) {
        //resetMouse();
        // Adjust frequency and amplitude to CPU data
        // On ground: Hold
        if (cpuData.absGroundAlt <= 1 && cpuData.spd <= 3) {
            this->camFreq.x = 0.025f;
            this->camAmpl.x = 0.10f;
            this->camFreq.y = 0.025f;
            this->camAmpl.y = 0.25f;
            this->camFreq.z = 0.01f;
            this->camAmpl.z = 2.5f;

        // On ground: Taxi
        } else if (cpuData.absGroundAlt <= 1 && cpuData.spd > 3 && cpuData.spd <= 25) {
            this->camFreq.x = 0.025f + (0.05f * ((cpuData.spd - 3.0f) / (25.0f - 3.0f)));
            this->camAmpl.x = 0.10f;
            this->camFreq.y = 0.025f + (0.20f * ((cpuData.spd - 3.0f) / (25.0f - 3.0f)));
            this->camAmpl.y = 0.25f - (0.15f * ((cpuData.spd - 3.0f) / (25.0f - 3.0f))); // Target .10f
            this->camFreq.z = 0.01f;
            this->camAmpl.z = 2.5f;

        // On ground: Takeoff / landing roll
        } else if (cpuData.absGroundAlt <= 1 && cpuData.spd > 25) {
            this->camFreq.x = 0.075f + (0.10f * ((cpuData.spd - 25.0f) / (100.0f - 25.0f)));// Target .085f
            this->camAmpl.x = 0.10f + (0.05f * ((cpuData.spd - 25.0f) / (100.0f - 25.0f))); // Target .15f
            this->camFreq.y = 0.225f + (0.225f * ((cpuData.spd - 25.0f) / (100.0f - 25.0f)));// Target .45f
            this->camAmpl.y = 0.10f + (0.20f * ((cpuData.spd - 25.0f) / (100.0f - 25.0f))); // Target .30f
            this->camFreq.z = 0.025f;
            this->camAmpl.z = 2.5f;

        // In flight: Departure / approach
        } else if (cpuData.absGroundAlt > 1 && cpuData.spd > 25 &&  cpuData.flaps > 0) {
            this->camFreq.x = 0.025f + (0.025f * (cpuData.flaps / 40));// Target .05f
            this->camAmpl.x = 0.15f;
            this->camFreq.y = 0.025f + (0.025f * (cpuData.flaps / 40));// Target .05f
            this->camAmpl.y = 0.30f;
            this->camFreq.z = 0.02f;
            this->camAmpl.z = 3.5f;

        // In flight: Cruise
        } else {
            this->camFreq.x = 0.025f;
            this->camAmpl.x = 0.15f;
            this->camFreq.y = 0.025f;
            this->camAmpl.y = 0.25f;
            this->camFreq.z = 0.02f;
            this->camAmpl.z = 3.5f;
        }
        
        // Calc Simplex Noise input params
        simplNoise.x += this->camFreq.x * cpuData.simReplayRate;
        simplNoise.y += this->camFreq.y * cpuData.simReplayRate;
        simplNoise.z += this->camFreq.z * cpuData.simReplayRate;

        // Calc camera position (X/Y/Z Axis)
        x = this->camLimit.x * this->camAmpl.x * this->camShakeX.Noise(simplNoise.x);
        y = this->camLimit.y * this->camAmpl.y * this->camShakeY.Noise(simplNoise.y);
        z = this->camLimit.z * this->camAmpl.z * this->camShakeZ.Noise(simplNoise.z);
        
        // Calc camera movement and update position
        this->camMov.x = x - this->camPos.x;
        this->camPos.x += this->camMov.x;

        this->camMov.y = y - this->camPos.y;
        this->camPos.y += this->camMov.y;

        this->camMov.z = z - this->camPos.z;
        this->camPos.z += this->camMov.z;

        moveMouse(winPos, this->camMov);
    
    // Limits exceeded, reset view 
    } else {
        // Save offset between current position on Simplex Noise curve and center / zero position

        // Reset position to center / zero
        this->camPos.x = 0;
        this->camPos.y = 0;
        this->camPos.z = 0;

        pressKey(GetWindowThreadProcessId(window, NULL), 'f');
    }

    camX = this->camPos.x;
    camY = this->camPos.y;
    camZ = this->camPos.z;

    this->camLastUpdateT = cpuData.fltTime;

    return 0;
}

/**
* Use native Windows API to provide mouse input to 
* an application window
*/ 
void CamShake::moveMouse(RECT winPos, Position mousePos){
    Position ctrPos;
    INPUT ip[4] = {};
    ZeroMemory(ip, sizeof(ip));
    
    // Get window dimensions
    this->winHeight = winPos.bottom - winPos.top;    
    this->winWidth = winPos.right - winPos.left;

    // Get window center position
    ctrPos.x = (float)(this->winWidth / 2) + winPos.left;
    ctrPos.y = (float)(this->winHeight / 2) + winPos.top;

    // Center mouse in window on window focus
    ip[0].type = INPUT_MOUSE;
    ip[0].mi.dx = (DWORD)ctrPos.x * (65536 / GetSystemMetrics(SM_CXSCREEN));
    ip[0].mi.dy = (DWORD)ctrPos.y * (65536 / GetSystemMetrics(SM_CYSCREEN));
    ip[0].mi.mouseData = 0;
    // @note With the middle button enabled, center cursor actions triggers corresponding camera movements as well, i.e. center only once
    if (!this->mouseCtrButton) {
        ip[1].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        this->mouseCtrButton = true;
    } else {
        ip[1].mi.dwFlags = 0; //MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
    }
    ip[0].mi.time = 0;
    ip[0].mi.dwExtraInfo = 0;

    // Press middle mouse button once on window focus (to enable MSFS2020 camera)
    ip[1].type = INPUT_MOUSE;
    ip[1].mi.dx = (DWORD)((mousePos.x / 100) * this->winWidth);
    ip[1].mi.dy = (DWORD)((mousePos.y / 100) * this->winHeight);
    ip[1].mi.mouseData = 0;
    if (!this->mouseCtrButton) {
        ip[1].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        this->mouseCtrButton = true;
    } else
        ip[1].mi.dwFlags = 0;
    ip[1].mi.time = 0;
    ip[1].mi.dwExtraInfo = 0;

    // Move mouse (to adjust MSFS2020 camera position)
    ip[2].type = INPUT_MOUSE;
    ip[2].mi.dx = (DWORD)((mousePos.x / 100) * this->winWidth);
    ip[2].mi.dy = (DWORD)((mousePos.y / 100) * this->winHeight);
    ip[2].mi.mouseData = 0;
    ip[2].mi.dwFlags = MOUSEEVENTF_MOVE;
    ip[2].mi.time = 0;
    ip[2].mi.dwExtraInfo = 0;

    // Move mouse wheel (to adjust MSFS2020 camera zoom)
    ip[3].type = INPUT_MOUSE;
    ip[3].mi.dx = (DWORD)((mousePos.x / 100) * this->winWidth);
    ip[3].mi.dy = (DWORD)((mousePos.y / 100) * this->winHeight);
    ip[3].mi.mouseData = (DWORD)mousePos.z;
    ip[3].mi.dwFlags = MOUSEEVENTF_WHEEL;
    ip[3].mi.time = 0;
    ip[3].mi.dwExtraInfo = 0;

    SendInput(ARRAYSIZE(ip), ip, sizeof(INPUT));
   
}

void CamShake::resetMouse() {
    Position ctrPos;
    INPUT ipCycle;
    
    // Get window center position
    ctrPos.x = (float)(this->winWidth / 2) + winPos.left;
    ctrPos.y = (float)(this->winHeight / 2) + winPos.top;

    ipCycle.type = INPUT_MOUSE;
    ipCycle.mi.dx = (DWORD)ctrPos.x * (65536 / GetSystemMetrics(SM_CXSCREEN));
    ipCycle.mi.dy = (DWORD)ctrPos.y * (65536 / GetSystemMetrics(SM_CYSCREEN));
    ipCycle.mi.mouseData = 0;
    if (this->mouseCtrButton) {
        ipCycle.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
        this->mouseCtrButton = false;
    } else
        ipCycle.mi.dwFlags = 0;
    ipCycle.mi.time = 0;
    ipCycle.mi.dwExtraInfo = 0;
    SendInput(1, &ipCycle, sizeof(ipCycle));

}

/**
* Use native Windows API to provide keyboard input to
* an application window
*/
void CamShake::pressKey(DWORD threadID, char mK) {
    HKL kbLayout = GetKeyboardLayout(threadID);

    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    //ASCII Codes: Special and lowercase chars
    if ((int)mK < 65 || (int)mK>90) {
        ip.ki.wScan = 0;
        ip.ki.wVk = VkKeyScanEx(mK, kbLayout);
    }
    //ASCI Codes: Uppercase chars
    else {
        ip.ki.wScan = mK;
        ip.ki.wVk = 0;
    }

    ip.ki.dwFlags = 0;
    SendInput(1, &ip, sizeof(INPUT));

    Sleep(1); // Logical ms required between key press down and up

    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}


/**
* Load camera shake configuration from config database
*/
int CamShake::LoadCamShakeConfig(){
    sqlite3* dbCon = nullptr;
    sqlite3_stmt* stmt = nullptr;
    std::string appPathFile = this->guiMain->GetExePathUTF8() + "config.db";

    if (sqlite3_open(appPathFile.c_str(), &dbCon) == SQLITE_OK) {
        int rc = sqlite3_prepare_v2(dbCon, "SELECT * FROM camshake", -1, &stmt, NULL);
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            switch (sqlite3_column_int(stmt, 0)) {
                case limitId: {
                    this->camLimit.x = (float) sqlite3_column_double(stmt, 2);
                    this->camLimit.y = (float) sqlite3_column_double(stmt, 3);
                    this->camLimit.z = (float) sqlite3_column_double(stmt, 4);
                    break;
                }
                case frequencyId: {
                    this->camFreq.x = (float) sqlite3_column_double(stmt, 2);
                    this->camFreq.y = (float) sqlite3_column_double(stmt, 3);
                    this->camFreq.z = (float) sqlite3_column_double(stmt, 4);
                    break;
                }
                case amplitudeId: {
                    this->camAmpl.x = (float) sqlite3_column_double(stmt, 2);
                    this->camAmpl.y = (float) sqlite3_column_double(stmt, 3);
                    this->camAmpl.z = (float) sqlite3_column_double(stmt, 4);
                    break;
                }
                break;
            }
        }
        // Finalize and close connection
        sqlite3_finalize(stmt);
        sqlite3_close(dbCon);

        return 0;   
    }
    else 
        return 1;
}