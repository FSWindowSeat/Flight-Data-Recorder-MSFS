#pragma once

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/thread.h>
#include <wx/image.h>
#include <wx/aboutdlg.h> 
#include <wx/help.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

// Forward declarations to avoid circular include.
class Processor; // #include "Processor.h" in.cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file
class FTDISFile; // #include "FTDISfile.h" in .cpp file
class FDRFile; // #include "FDRfile.h" in .cpp file

/** 
* Flight Data Recorder Main Graphical User Interface
*
* This represents the applications's structure and the implementation of its event handlers
*
* @author muppetlabs@fswindowseat.com
*/
class GuiMain : public wxFrame {

public:
	GuiMain();
	~GuiMain();

	Processor *cpuThread = nullptr;
	ProcessorCom *cpuCom = nullptr;
	wxCriticalSection cpuThreadCS; // protects the cpu thread pointer
	
private:
	wxBitmapButton *btnRewind = nullptr;
	wxBitmapButton *btnForward = nullptr;
	wxBitmapButton *btnPlay = nullptr;
	wxBitmapButton *btnPause = nullptr;
	wxBitmapButton *btnStop = nullptr;
	wxBitmapButton *btnRecord = nullptr;

	wxBitmap btnRewindPNG = wxBitmap(wxT("ButtonRewind"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnRewindActivePNG = wxBitmap(wxT("ButtonRewindActive"), wxBITMAP_TYPE_PNG_RESOURCE);

	wxBitmap btnForwardPNG = wxBitmap(wxT("ButtonForward"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnForwardActivePNG = wxBitmap(wxT("ButtonForwardActive"), wxBITMAP_TYPE_PNG_RESOURCE);

	wxBitmap btnPlayPNG = wxBitmap(wxT("ButtonPlay"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnPlayActivePNG = wxBitmap(wxT("ButtonPlayActive"), wxBITMAP_TYPE_PNG_RESOURCE);
	
	wxBitmap btnPausePNG = wxBitmap(wxT("ButtonPause"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnPauseActivePNG = wxBitmap(wxT("ButtonPauseActive"), wxBITMAP_TYPE_PNG_RESOURCE);
	
	wxBitmap btnStopPNG = wxBitmap(wxT("ButtonStop"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnStopActivePNG = wxBitmap(wxT("ButtonStopActive"), wxBITMAP_TYPE_PNG_RESOURCE);
	
	wxBitmap btnRecordPNG = wxBitmap(wxT("ButtonRecord"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnRecordActivePNG = wxBitmap(wxT("ButtonRecordActive"), wxBITMAP_TYPE_PNG_RESOURCE);
	wxBitmap btnRecordingPNG = wxBitmap(wxT("ButtonRecording"), wxBITMAP_TYPE_PNG_RESOURCE);

	

	wxMenuBar *menuBar = nullptr;
	wxMenu *file = nullptr;
	wxMenu *help = nullptr;
	wxStatusBar *statusBar = nullptr;
	wxStaticText *text = nullptr;
	wxStaticText *label = nullptr;
	wxCHMHelpController *helpCtrl = nullptr;

	FDRFile *fdrFile = nullptr;
	FTDISFile *ftdisFile = nullptr;

	wxString exePathFile;

	int procCount = 0, fileType = 0;

	bool fileLoaded = false, newRecording = true;

	/**
	* Pause the the processor / worker thread
	* @param	Handler processor / worker thread
	* @param	wxCriticalSection to protect cpu thread pointer
	*/
	bool PauseProcessor(Processor* cpuHandler, wxCriticalSection& critSection);
	
	/**
	* Handles deletion of processor / worker thread
	* @param	Handler processor / worker thread
	* @param	wxCriticalSection to protect cpu thread pointer
	*/
	bool StopProcessor(Processor *cpuHandler, wxCriticalSection &critSection);
	
	/**
	* Handles processor / worker thread updates
	* @param	Address of wxCommandEvent
	*/
	void OnThreadUpdate(wxCommandEvent& evt);

	/**
	* Resets User Interface
	*/
	void ResetUI(std::string statusText);

	/** 
	* Response to load file menu option
	* @param	Address of wxCommandEvent
	*/
	void LoadInputFile(wxCommandEvent &evt);

	/**
	* Response to record button press
	* @param	Address of wxCommandEvent	
	*/
	void ButtonRecordOutputFile(wxCommandEvent& evt);

	/** 
	* Response to play inputfile button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonPlayInputFile(wxCommandEvent& evt);

	/**
	* Response to pause inputfile button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonPauseInputFile(wxCommandEvent& evt);

	/** 
	* Response to stop inputfile button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonStopInputFile(wxCommandEvent& evt);

	/**
	* Response to fast forward button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonForwardInputFile(wxCommandEvent& evt);

	/**
	* Response to fast rewind button press
	* @param	Address of wxCommandEvent
	*/
	void ButtonRewindInputFile(wxCommandEvent& evt);

	/**
	* Response to quit application menu option
	* @param	Address of wxCommandEvent
	*/
	void QuitApp(wxCommandEvent& evt);

	/**
	* Response to about help menu option
	* @param	Address of wxCommandEvent
	*/
	void ShowHelpWindow(wxCommandEvent& evt);

	/**
	* Response to about menu option
	* @param	Address of wxCommandEvent
	*/
	void ShowAboutDialog(wxCommandEvent& evt);

	// Run wxWidgets macros
	wxDECLARE_EVENT_TABLE();

};
