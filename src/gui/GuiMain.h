#ifndef GuiMain_H
#define GuiMain_H

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/thread.h>
#include <wx/image.h>
#include <wx/aboutdlg.h> 
#include <wx/help.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "GuiOverlayDialog.h"
#include "GuiOptionsDialog.h"
#include "GuiTrafficDialog.h"

using namespace std;

// Forward declarations to avoid circular include.
class Processor; // #include "Processor.h" in.cpp file
class ProcessorCom; // #include "ProcessorCom.h" in.cpp file
class Track; // #include "Track.h" in .cpp file
class FTDISFile; // #include "FTDISfile.h" in .cpp file
class FDRFile; // #include "FDRfile.h" in .cpp file

/** 
* Flight Data Recorder Main Graphical User Interface
*
* This represents the applications's main GUI and the implementation of its event handlers
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
	
protected:
	// Run wxWidgets macros
	wxDECLARE_EVENT_TABLE();

private:
	// Gui Main
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
	wxMenu* options = nullptr;
	wxMenu *help = nullptr;
	wxStatusBar *statusBar = nullptr;
	wxStaticText *text = nullptr;
	wxStaticText *label = nullptr;
	wxStaticText* statusBarTxt = nullptr;
	wxCHMHelpController *helpCtrl = nullptr;

	// Gui Dialogs
	GuiOverlayDialog* overlayDialog = nullptr;
	GuiOptionsDialog* optionsDialog = nullptr;
	GuiTrafficDialog* trafficDialog = nullptr;

	vector<Track*> tracks;
	Track* track = nullptr;
	FDRFile *fdrFile = nullptr;
	FTDISFile *ftdisFile = nullptr;

	wxString exePathFile;

	int procCount = 0, fileType = 0;

	bool newRecording = true;

public:
	/**
	* Pause the the processor / worker thread
	* 
	* @param	Handler processor / worker thread
	* @param	wxCriticalSection to protect cpu thread pointer
	* @return	bool
	*/
	bool PauseProcessor(Processor* cpuHandler, wxCriticalSection& critSection);
	
	/**
	* Stops the processor / worker thread and resets it to start of processing
	* 
	* @param	Handler processor / worker thread
	* @param	wxCriticalSection to protect cpu thread pointer
	* @return	bool
	*/
	bool StopProcessor(Processor *cpuHandler, wxCriticalSection &critSection);

	/**
	* Resets and & handles deletion of processor / worker thread
	*
	* @param	Handler processor / worker thread
	* @param	wxCriticalSection to protect cpu thread pointer
	* @return	bool
	*/
	bool ResetProcessor(Processor* cpuHandler, wxCriticalSection& critSection);

	/**
	* Handles processor / worker thread updates
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void OnThreadUpdate(wxCommandEvent& evt);

	/**
	* Resets User Interface
	* @param	status text		String
	* @return	void
	*/
	void ResetUI(std::string statusText);

	/** 
	* Response to load file menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void LoadInputFile(wxCommandEvent &evt);

	/**
	* Add new track to track vector
	*
	* @param	track	Type Track 
	* @return	void
	*/
	void AddTrack(Track* track);
	
	/**
	* Remove track from track vector
	*
	* @param	trackId		int
	* @return	void
	*/
	void RemoveTrack(int trackId);

	/**
	* Return tracks / track vector
	*
	* @param	void
	* @return	Vector	track (pointer)
	*/
	vector<Track*> GetTracks();

	/**
	* Return specific track with trackId
	*
	* @param	trackId		int
	* @return	Vector		track (pointer)
	*/
	Track* GetTrack(int trackId);

	/**
	* This method returns the path to the executable
	*
	* @return	Path to executable	Path to executable	string (UTF 8)
	*/
	string GetExePathUTF8();

	/**
	* Response to record button press
	* 
	* @param	Address of wxCommandEvent	
	* @return	void
	*/
	void ButtonRecordOutputFile(wxCommandEvent& evt);

	/** 
	* Response to play inputfile button 
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonPlayInputFile(wxCommandEvent& evt);

	/**
	* Response to pause inputfile button press
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonPauseInputFile(wxCommandEvent& evt);

	/** 
	* Response to stop inputfile button 
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonStopInputFile(wxCommandEvent& evt);

	/**
	* Response to fast forward button press
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonForwardInputFile(wxCommandEvent& evt);

	/**
	* Response to fast rewind button press
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void ButtonRewindInputFile(wxCommandEvent& evt);

	/**
	* Response to quit application menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void QuitApp(wxCommandEvent& evt);

	/**
	* Response to Reset Processor option
	*
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuResetProcessor(wxCommandEvent& evt);

	/**
	* Response to AI Traffic menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuShowTrafficDialog(wxCommandEvent& evt);

	/**
	* Response to Overlay menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuShowOverlayDialog(wxCommandEvent& evt);
	
	/**
	* Response to preferences menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuShowOptionsDialog(wxCommandEvent& evt);

	/**
	* Response to about help menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuShowHelpWindow(wxCommandEvent& evt);

	/**
	* Response to about menu option
	* 
	* @param	Address of wxCommandEvent
	* @return	void
	*/
	void MenuShowAboutDialog(wxCommandEvent& evt);
};
#endif