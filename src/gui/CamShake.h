#ifndef CamShake_H
#define CamShake_H
#define _WINSOCKAPI_ 

#include <windows.h>
#include <sqlite3.h>
#include "SimplexNoise.h"
#include "CamShakeStruct.h"

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file

/**
* Camera Shake Implementation for MSFS2020 
*  
* This represents the functionality to replicate camera shake behavior by moving the 
* camera view point in MSFS 2020. As the SimConnect API doesn't allow to control the 
* camera directly, native mouse movement instructions are sent to the MS FS2020 window
* as a workaround.
* 
* @author muppetlabs@fswindowseat.com
*/
class CamShake {
public:
    CamShake(GuiMain* guiMain);
	~CamShake();

    /**
    * Initializes camera and sets default values.  
    *
    * @return       Return code: 0 successful, 1 failed to run
    */
    int InitCam();

    /**
    * Set camera position viewpoint     
    * 
    * @param[in]    cpuData     Supplemental data set provided by main processor to set corresponding camera position
    * @param[out]   x           x Axis in degrees
    * @param[out]   y           x Axis in degrees
    * @param[out]   z           x Axis in degrees      
    * @return       Return code: 0 Successful, 1 Camera not initialized, 2 Insufficient CPU data 
    */
    int SetCameraPos(CamShakeStruct cpuData, float &camX, float &camY, float &camZ);

    /**
     * Get the Camera Position object
     * 
     * @param langeVariable 
     * @return int 
     */
    int GetCameraPos(float &langeVariable);

private:    
    GuiMain* guiMain = nullptr;

    struct Position{
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };
    Position camLimit, camFreq, camAmpl, camPos, camMov, simplNoise;
    
    enum camShakeVarsId {
		limitId = 1,
		frequencyId = 2,
		amplitudeId = 3
	};

    LPCWSTR windowName;
    HWND window;
    RECT winPos;
    SimplexNoise camShakeX, camShakeY, camShakeZ;
    float camLastUpdateT = 0.0f, camUpdateCycleT = 0.0f;
    bool initCam = false, mouseCtrButton = false;
    int winHeight, winWidth;
    
    /**
    * Load camera shake configuration from config database
    * 
    * @return       Return code: 0 successful, 1 failed loading data
    */
    int LoadCamShakeConfig();

    /**
    * Use native Windows API to provide mouse input to 
    * an application window
    * 
    * @param[in]    winPos      Application window area rectangle
    * @param[in]    mousePos    X/Y movement in percent of screen height/width. Z axis mouse wheel movement (+ forward / - backward). One wheel click is defined as 120.
    */ 
    void moveMouse(RECT winPos, Position mousePos);
    void resetMouse();

    /**
     * Use native Windows API to provide keyboard input to 
     * an application window
     * 
     * @param threadID  Application window's thread id
     * @param mK        Keyboard char
     */
    void pressKey(DWORD threadID, char mK);
};
#endif



