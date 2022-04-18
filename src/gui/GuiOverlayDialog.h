#ifndef GuiOverlayDialog_H
#define GuiOverlayDialog_H

#include <wx/wx.h>
#include <wx/timectrl.h>
#include <wx/datetime.h>
#include "OptionsStruct.h"

// Forward declarations to avoid circular include.
class GuiMain;  // #include "GuiMain.h" in .cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file

/**
* Flight Data Recorder OBS Overlay Dialog - Graphical User Interface
*
* This represents the applications's OBS Overlay dialog and the implementation of its event handlers
*
* @author muppetlabs@fswindowseat.com
*/
class GuiOverlayDialog : public wxDialog {

public:
	GuiOverlayDialog(GuiMain* guiMain, ProcessorCom* com);

protected:
	// Run wxWidgets macros
	wxDECLARE_EVENT_TABLE();

private:
	GuiMain* guiMain = nullptr;
	ProcessorCom* cpuCom = nullptr;
	wxTimePickerCtrl* depTime = nullptr;
	wxComboBox* depGMTZone = nullptr;
	wxTextCtrl* destName = nullptr;
	wxComboBox* destGMTZone = nullptr;
	wxTimePickerCtrl* fltDur = nullptr;
	wxFont headingFont;
	
public:
	/**
	* Response to save button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonSaveOptions(wxCommandEvent& evt);
};
#endif