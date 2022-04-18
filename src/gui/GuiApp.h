#ifndef GuiApp_H
#define GuiApp_H

#include <wx/wx.h>
#include "GuiMain.h"

/** 
* Flight Data Recorder
* 
* This is an early prototype based on the MS FS2020 SDK to demonstrate how to enable flight recording
* and replay functionality within MS Flight Simulator 2020, using the SimConnect API.
*
* @author muppetlabs@fswindowseat.com
*/
class GuiApp : public wxApp {

public:
	GuiApp();
	~GuiApp();

private:
	GuiMain *mainWindow = nullptr;

public:
	
	/** 
	* Initializes the GUI's main window
	*/
	bool OnInit();
};
#endif