#ifndef GuiTrafficDialog_H
#define GuiTrafficDialog_H

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/grid.h>
#include <sqlite3.h>

using namespace std;

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file
class Track; // #include "Track.h" in .cpp file
class FTDISFile; // #include "FTDISfile.h" in .cpp file
class FDRFile; // #include "FDRfile.h" in .cpp file


/**
* Flight Data Recorder AI Traffic Dialog - Graphical User Interface
*
* This represents the applications's AI Traffic dialog and the implementation of its event handlers
*
* @author muppetlabs@fswindowseat.com
*/
class GuiTrafficDialog : public wxDialog {

public:
	GuiTrafficDialog(GuiMain* guiMain, ProcessorCom* com);

protected:
	// Run wxWidgets macros
	wxDECLARE_EVENT_TABLE();

private:
	GuiMain *guiMain = nullptr;
	Track* track = nullptr;
	FDRFile* fdrFile = nullptr;
	FTDISFile* ftdisFile = nullptr;
	ProcessorCom* cpuCom = nullptr;
	wxGrid* grid = nullptr;
	wxComboBox* selectSimApi = nullptr;
	wxComboBox* selectSimObject = nullptr;
	wxTextCtrl* timeOffset = nullptr;
	wxFont headingFont;
	int fileType = 0;

	struct SimApi {
		int recId;
		int simApiId;
	};
	
	vector<SimApi> simApis;

	struct SimObject {
		int recId;
		char* type;
		char* livery;
		char* cfgUuid;
	};
	
	vector<SimObject> simObjects;

public:
	/**
	* Response to load file button press
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonAddTrack(wxCommandEvent& evt);

	/**
	* Response to save button press
	*
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonSaveOptions(wxCommandEvent& evt);

	/**
	* Response to left click cell event 
	*
	* @param	Address of wxGridEvent
	* @return	void
	*/
	void LeftClickCell(wxGridEvent& evt);

	/**
	* Response to left click cell event
	*
	* @param	Address of wxGridEvent
	* @return	void
	*/
	void RightClickCell(wxGridEvent& evt);

	/**
	* Load track file and add to / update track vector
	*
	* @param	trackid
	* @return	void
	*/
	void LoadTrack(int trackId);

	/**
	* Load grid with track data
	*
	* @param	void
	* @return	void
	*/
	void LoadGrid();

	/**
	* Save grid data to track(s)
	*
	* @param	void
	* @return	void
	*/
	void SaveGrid();

	/**
	* Get traffic APIs and models from config database
	*
	* @param	void
	* @return	void
	*/
	void GetTrafficDialogConfig();
};
#endif