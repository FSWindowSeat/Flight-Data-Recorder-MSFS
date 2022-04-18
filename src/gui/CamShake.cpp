#include "CamShake.h"
#include "GuiMain.h"

/**
* Constructor / Destructor
*/
CamShake::CamShake(GuiMain* guiMain){
    // Set handler / references
    this->guiMain = guiMain;
    this->windowName = L"Microsoft Flight Simulator - 1.21.13.0";
    this->window = FindWindowW(NULL, windowName);

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
    // @todo Load from db config table
    this->camUpdateCycleT = 0.1f; // @todo Update for production code, .5 seconds for testing only
    this->camPixelScale.x = 25; // Approximation: 100px / 1.2 deg @
    this->camPixelScale.y = 25; // Approximation: 
    this->camPixelScale.z = 5;

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
*/
int CamShake::SetCameraPos(CamShakeStruct cpuData, float &camX, float &camY, float &camZ) {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    
    // Catch: Camera not initialized
    if(!this->initCam)
        return 1;
    
    // Catch: Insufficient CPU data and/or min time between updates not reached
    if(cpuData.fltTime <= 0 || cpuData.fltTime - this->camLastUpdateT < this->camUpdateCycleT)
        return 2;
    
    // Review and adjust movement scale, i.e pixel/degree, X/Y Axis
    /*
    if(camPos.x != 0.0f && cpuData.simCamPosX != 0.0f && camPos.y != 0.0f && cpuData.simCamPosY != 0.0f && cpuData.fltTime - this->camScaleLastUpdateT > 5.0f){
        this->camPixelScale.x *= this->camPos.x / cpuData.simCamPosX; 
        this->camPixelScale.y *= this->camPos.y / cpuData.simCamPosY;
        this->camScaleLastUpdateT = cpuData.fltTime;
    }*/

    // Calc camera position (X/Y/Z Axis)
    x = this->camLimit.x * this->camAmpl.x * this->camShakeX.Noise(cpuData.fltTime * pow(camFreq.x,2));
    y = this->camLimit.y * this->camAmpl.y * this->camShakeY.Noise(cpuData.fltTime * pow(camFreq.y,2));
    z = this->camLimit.z * this->camAmpl.z * this->camShakeZ.Noise(cpuData.fltTime * pow(camFreq.z,2));
    
    // Calc camera movement and update position
    this->camMov.x = x - this->camPos.x;
    this->camPos.x += this->camMov.x;
    camX = this->camPos.x;

    this->camMov.y = y - this->camPos.y;
    this->camPos.y += this->camMov.y;
    camY = this->camPos.y;

    this->camMov.z = z - this->camPos.z;
    this->camPos.z += this->camMov.z;
    camZ = this->camPos.z;
    
    this->camLastUpdateT = cpuData.fltTime;

    // Calculate Offset and adjust movement to compensate


    // Catch: MSFS 2020 window not in focus 
    // @todo move case to top. This position is for testing/debugging only
    //if(GetForegroundWindow() != window)  
    //    return 3;

    // Send mouse movement instructions to window to set camera position
    //GetWindowRect(window, &winPos);
    //moveMouse(winPos, this->camMov);


    return 0;
}

/**
* Use native Windows API to provide mouse input to 
* MSFS 2020 main application window
*/ 
void CamShake::moveMouse(RECT winPos, Position mousePos){
    Position ctrPos;
    INPUT ip;
    ip.type = INPUT_MOUSE;
    ip.mi.mouseData = 0;
    
    // Get window center position
    ctrPos.x = ((winPos.right - winPos.left) / 2) + winPos.left;
    ctrPos.y = ((winPos.bottom - winPos.top) / 2) + winPos.top;

    // Center mouse in window
    ip.mi.dx = ctrPos.x * (65536 / GetSystemMetrics(SM_CXSCREEN));
    ip.mi.dy = ctrPos.y * (65536 / GetSystemMetrics(SM_CYSCREEN));
    ip.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &ip, sizeof(ip));

    // Move mouse and interact (button)
    ip.mi.dx = (int) (mousePos.x * this->camPixelScale.x);
    ip.mi.dy = (int) (mousePos.y * this->camPixelScale.y);
    ip.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_MOVE;
    SendInput(1, &ip, sizeof(ip));
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