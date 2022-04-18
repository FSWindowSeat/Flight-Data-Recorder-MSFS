#ifndef GuiOptionsDialog_H
#define GuiOptionsDialog_H

#include <wx/wx.h>
#include <sqlite3.h>
#include "OptionsStruct.h"

using namespace std;

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file

/**
* Flight Data Recorder Options Dialog - Graphical User Interface
*
* This represents the applications's option dialog and the implementation of its event handlers
*
* @author muppetlabs@fswindowseat.com
*/
class GuiOptionsDialog : public wxDialog {

public:
	GuiOptionsDialog(GuiMain* guiMain, ProcessorCom* com);

protected:
	// Run wxWidgets macros
	wxDECLARE_EVENT_TABLE();

private:
	GuiMain* guiMain = nullptr;
	ProcessorCom* cpuCom = nullptr;
	wxComboBox* selectSimApi = nullptr;
	wxCheckBox* cbHalfRate = nullptr;
	wxCheckBox* cbCustSound = nullptr;
	wxCheckBox* cbBeebSound = nullptr;
	wxCheckBox* cbCamShake = nullptr;
	wxArrayString simApiList;
	wxFont headingFont;
	int defApi = 0, halfRate = 0, custSound = 0, notification = 0, camShake = 0;

	struct SimApi {
		int recId;
		int simApiId;
	};

	vector<SimApi> simApis;

	enum PreferencesId {
		defApiId = 1,
		halfRateId = 2,
		custSoundId = 3,
		notificationId = 4,
		camShakeId = 5
	};

public:
	/**
	* Response to save button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonSaveOptions(wxCommandEvent& evt);

	/**
	* Get APIs and preferences from config database
	*
	* @param	void
	* @return	void
	*/
	void LoadOptionsDialogConfig();

	/**
	* Save APIs and preferences to config database
	*
	* @param	void
	* @return	void
	*/
	void SaveOptionsDialogConfig();
};
#endif