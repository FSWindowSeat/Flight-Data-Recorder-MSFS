#include "GuiMain.h"
#include "Processor.h"
#include "ProcessorCom.h"
#include "Track.h"
#include "FDRFile.h"
#include "FTDISFile.h"
#include "Resource.h"


/** 
* Definition events and event table 
*/
wxBEGIN_EVENT_TABLE(GuiMain, wxFrame)
	// Menu Items
	EVT_MENU(wxID_OPEN, GuiMain::LoadInputFile)
	EVT_MENU(wxID_REFRESH, GuiMain::MenuResetProcessor)
	EVT_MENU(wxID_EXIT, GuiMain::QuitApp)
	EVT_MENU(wxID_NETWORK, GuiMain::MenuShowTrafficDialog)
	EVT_MENU(wxID_INDEX, GuiMain::MenuShowOverlayDialog)
	EVT_MENU(wxID_PREFERENCES, GuiMain::MenuShowOptionsDialog)
	EVT_MENU(wxID_HELP, GuiMain::MenuShowHelpWindow)
	EVT_MENU(wxID_ABOUT, GuiMain::MenuShowAboutDialog)
	
	// Buttons
	EVT_BUTTON(wxID_BACKWARD, GuiMain::ButtonRewindInputFile)
	EVT_BUTTON(wxID_STOP, GuiMain::ButtonStopInputFile)
	EVT_BUTTON(wxID_EXECUTE, GuiMain::ButtonPlayInputFile)
	EVT_BUTTON(wxID_FORWARD, GuiMain::ButtonForwardInputFile)
	EVT_BUTTON(wxID_STATIC, GuiMain::ButtonPauseInputFile)
	EVT_BUTTON(wxID_NEW, GuiMain::ButtonRecordOutputFile)

	// Catch events from processor/worker thread
	EVT_COMMAND(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED, GuiMain::OnThreadUpdate)

wxEND_EVENT_TABLE()


/** 
* Constructor. Defines and initializes GUI components
*/
GuiMain::GuiMain() : wxFrame(nullptr, wxID_ANY, "Flight Data Recorder", wxDefaultPosition, wxSize(400,149), (wxDEFAULT_FRAME_STYLE & ~wxMAXIMIZE_BOX & ~wxRESIZE_BORDER)) {

	cpuCom = new ProcessorCom();
	exePathFile = wxStandardPaths::Get().GetExecutablePath();
	overlayDialog = new GuiOverlayDialog(this, cpuCom);
	optionsDialog = new GuiOptionsDialog(this, cpuCom);
	trafficDialog = new GuiTrafficDialog(this, cpuCom);
	this->SetIcon(wxICON(FDRIcon));
	this->SetBackgroundColour(wxColour(69,69,69));

	// Definition Menu Bar
	menuBar = new wxMenuBar;
	file = new wxMenu;
	file->Append(wxID_OPEN, wxT("&Open"), wxT(""));
	file->Append(wxID_REFRESH, wxT("&Reset Recorder"), wxT(""));
	file->AppendSeparator();
	file->Append(wxID_EXIT, wxT("&Quit"));
	menuBar->Append(file, wxT("&File"));
	
	options = new wxMenu;
	options->Append(wxID_NETWORK, wxT("&AI Traffic"), wxT(""));
	options->Append(wxID_INDEX, wxT("&OBS Overlay"), wxT(""));
	options->Append(wxID_PREFERENCES, wxT("&Preferences"), wxT(""));
	menuBar->Append(options, wxT("&Options"));
	
	help = new wxMenu;
	help->Append(wxID_HELP, wxT("&View Help"), wxT(""));
	help->Append(wxID_ABOUT);
	menuBar->Append(help, wxT("&Help"));
	
	SetMenuBar(menuBar);

	// Definition GUI components
	btnRewind = new wxBitmapButton(this, wxID_BACKWARD, btnRewindPNG, wxPoint(10, 12), wxSize(45, 45), wxBORDER_NONE);
	btnStop = new wxBitmapButton(this, wxID_STOP, btnStopPNG, wxPoint(60, 12), wxSize(45, 45), wxBORDER_NONE);
	btnPlay = new wxBitmapButton(this, wxID_EXECUTE, btnPlayPNG, wxPoint(110, 12), wxSize(45, 45), wxBORDER_NONE);
	btnForward = new wxBitmapButton(this, wxID_FORWARD, btnForwardPNG, wxPoint(160, 12), wxSize(45, 45), wxBORDER_NONE);
	btnPause = new wxBitmapButton(this, wxID_STATIC, btnPausePNG, wxPoint(210, 12), wxSize(45, 45), wxBORDER_NONE);
	btnRecord = new wxBitmapButton(this, wxID_NEW, btnRecordPNG, wxPoint(330, 12), wxSize(45, 45), wxBORDER_NONE);

	text = new wxStaticText(this, wxID_ANY, "0", wxPoint(270, 20), wxSize(50, 12));
	text->SetForegroundColour(wxColour(190, 190, 190));
	
	label = new wxStaticText(this, wxID_ANY, "Frame", wxPoint(270, 40), wxSize(50, 12));
	label->SetForegroundColour(wxColour(190, 190, 190));

	// Definition Statusbar
	statusBar = CreateStatusBar(1);	
	statusBar->SetBackgroundColour(wxColour(59, 59, 59));
	statusBar->SetForegroundColour(wxColour(221, 221, 221));
	statusBarTxt = new wxStaticText(this->statusBar, wxID_ANY, wxT(""), wxPoint(10, 5), wxDefaultSize, 0);
	statusBarTxt->Show(true);

	// Initialize Help controller
	helpCtrl = new wxCHMHelpController;
	helpCtrl->Initialize(wxFileName(exePathFile).GetPath() + wxT("/FlightDataRecorder.chm"));

	// Reset UI
	this->ResetUI("Not connected to sim!");
}


/**
* Destructor
*/
GuiMain::~GuiMain() {
	this->StopProcessor(cpuThread, cpuThreadCS);
	Destroy();
	
	// Clean up
	for (auto& track : tracks)
		delete track;
	tracks.clear();

	delete overlayDialog;
	delete optionsDialog;
	delete trafficDialog;
	delete fdrFile;
	delete ftdisFile;
}


/**
* Pauses the processor / worker thread
*/
bool GuiMain::PauseProcessor(Processor* cpuHandler, wxCriticalSection& critSection) {
	{
		wxCriticalSectionLocker enter(critSection);
		// Does the processor thread still exist?
		if (cpuHandler) {
			if (cpuHandler->IsRunning()){
				cpuHandler->Pause();
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
}


/**
* Stops the processor / worker thread and resets it to start of processing
*/
bool GuiMain::StopProcessor(Processor* cpuHandler, wxCriticalSection& critSection) {
	int t = 0, timeOut = 5000;

	// Stop processor
	{
		wxCriticalSectionLocker enter(critSection);
		// Does the processor thread still exist?
		if (cpuHandler) {
			if (cpuHandler->IsRunning())
				cpuHandler->Pause();

			if (cpuHandler->Delete() != wxTHREAD_NO_ERROR) {
				wxLogError("Can't delete the thread!");
				return false;
			}
		}
	}
	// Exit from the critical section to give the thread the chance to enter its destructor
	while (t <= timeOut) {
		{ // Check the Com handler to see if the the desctructor has been triggered
			wxCriticalSectionLocker enter(critSection);
			if (!cpuCom->GetSimConnect())
				break;
		}
		// wait for thread completion
		wxThread::This()->Sleep(1);

		if (t == timeOut)
			return false;
		t++;
	}

	// Reset cursor position and UI
	cpuCom->SetCursorPos(0);
	this->ResetUI("");

	return true;
}


/**
* Handles deletion of main processor / worker thread
*/
bool GuiMain::ResetProcessor(Processor *cpuHandler, wxCriticalSection& critSection) {
	
	this->StopProcessor(cpuHandler, critSection);

	// Clean up tracks
	for (auto& track : tracks)
		delete track;
	tracks.clear();
	
	// Reset cursor position and UI
	cpuCom->SetCursorPos(0);
	this->SetTitle("Flight Data Recorder");
	this->ResetUI("");
	
	return true;
}


/**
* Handles processor / worker thread updates
*/
void GuiMain::OnThreadUpdate(wxCommandEvent& evt) {
	// Update "Frame" label
	text->SetLabel(std::to_string(cpuCom->GetCursorPos()));

	// Update: Frame rate label
	statusBarTxt->SetLabelText(wxT("x" + std::to_string(cpuCom->GetReplayRate())));

	// Check connectivity. Close thread and reset status in case SimConnect cannot connect
	if (!cpuCom->GetSimConnect()) {
		// Stop processor
		this->StopProcessor(cpuThread, cpuThreadCS);
		// Reset  UI
		this->ResetUI("");
	}
}


/**
* Resets User Interface
*/
void GuiMain::ResetUI(std::string msg) {
	btnRewind->SetBackgroundColour(wxColour(69, 69, 69));
	btnRewind->SetBitmap(btnRewindPNG);
	btnRewind->SetBitmapHover(btnRewindActivePNG);

	btnStop->SetBackgroundColour(wxColour(69, 69, 69));
	btnStop->SetBitmap(btnStopPNG);
	btnStop->SetBitmapHover(btnStopActivePNG);

	btnPlay->SetBackgroundColour(wxColour(69, 69, 69));
	btnPlay->SetBitmap(btnPlayPNG);
	btnPlay->SetBitmapHover(btnPlayActivePNG);

	btnForward->SetBackgroundColour(wxColour(69, 69, 69));
	btnForward->SetBitmap(btnForwardPNG);
	btnForward->SetBitmapHover(btnForwardActivePNG);

	btnPause->SetBackgroundColour(wxColour(69, 69, 69));
	btnPause->SetBitmap(btnPausePNG);
	btnPause->SetBitmapHover(btnPauseActivePNG);

	btnRecord->SetBackgroundColour(wxColour(69, 69, 69));
	btnRecord->SetBitmap(btnRecordPNG);
	btnRecord->SetBitmapHover(btnRecordActivePNG);

	text->SetLabel("0");
	statusBarTxt->SetLabelText(msg);
}


/** 
* Response to load file menu option 
*/
void GuiMain::LoadInputFile(wxCommandEvent &evt) {
	
	// Active Replay: Stop processor
	if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
		// Stop processor
		this->StopProcessor(cpuThread, cpuThreadCS);
	} 
	// Active Recording: Stop processor (end recording) and open save file dialog 
	if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
		// Stop processor
		this->StopProcessor(cpuThread, cpuThreadCS);
		
		// Create a "save file" dialog
		wxFileDialog* SaveDialog = new wxFileDialog(
			this, wxT("Save As"), wxEmptyString, wxEmptyString,
			wxT("Flight Data Recorder File (*.fdr)|*.fdr"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

		// If the user clicks "Save" instead of "Cancel"
		if (SaveDialog->ShowModal() == wxID_OK) {
			// Save file
			fdrFile->SaveFile((std::string)SaveDialog->GetPath());
		}
	}
	
	// Create a "open file" dialog
	wxFileDialog* OpenDialog = new wxFileDialog(
		this, wxT("Choose a file to open"), wxEmptyString, wxEmptyString,
		wxT("Flight Recorder Files (*.fdr)|*.fdr|FTDIS Data Files (*.ftd)|*.ftd"),
		wxFD_OPEN, wxDefaultPosition);

	// If the user clicks "Open" instead of "Cancel"
	if (OpenDialog->ShowModal() == wxID_OK) {
		// Set file type, Flight Data Recorder = 0, FTDIS = 1 
		fileType = OpenDialog->GetFilterIndex();

		// Load Flight Data Recorder file (.fdr)
		if(fileType == 0){			
			fdrFile = new FDRFile();
			// Load input file
			if (fdrFile->LoadFile((std::string)OpenDialog->GetPath())) {
				// Update status and amend title to show file name
				SetTitle(wxString("Flight Data Recorder - ") << OpenDialog->GetFilename());

				if (fdrFile->fileData.size() > 0) {
					cpuCom->SetCursorPos(0);
					statusBarTxt->SetLabelText(wxT("FDR input file loaded!"));
				}
				// Create new track, assign file (address) and add track (address) to vector
				track = new Track(Track::TrackType::USER);
				track->SetFileType(Track::FileType::FDR);
				track->SetSimApi(cpuCom->GetSimApi());
				track->SetFile(fdrFile);

				// Remove existing user tracks and add new track
				for (unsigned i = 0; i < tracks.size(); ++i) {
					if (tracks.at(i)->GetTrackType() == Track::TrackType::USER)
						tracks.erase(tracks.begin() + i);
				}
				tracks.insert(tracks.begin(), track);
			}
		}
		//Load FTDIS file (.ftd)
		else if(fileType == 1){
			ftdisFile = new FTDISFile();
			// Load input file
			if (ftdisFile->LoadFile((std::string)OpenDialog->GetPath())) {
				// Update status and amend title to show file name
				SetTitle(wxString("Flight Data Recorder - ") << OpenDialog->GetFilename());

				if(ftdisFile->fileData.size() > 0){
					cpuCom->SetCursorPos(0);
					statusBarTxt->SetLabelText(wxT("FTD input file loaded!"));
				}
				// Create new track, assign file (address) and add track (address) to vector
				track = new Track(Track::TrackType::USER);
				track->SetFileType(Track::FileType::FTDIS);
				track->SetSimApi(cpuCom->GetSimApi());
				track->SetFile(ftdisFile);

				// Remove existing user tracks and add new track
				for (unsigned i = 0; i < tracks.size(); ++i) {
					if (tracks.at(i)->GetTrackType() == Track::TrackType::USER)
						tracks.erase(tracks.begin() + i);
				}
				tracks.insert(tracks.begin(), track);
			}
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Add new track to track vector
* @param	Track
*/
void GuiMain::AddTrack(Track* track) {
	this->tracks.push_back(track);
}


/**
* Remove track from track vector
* @param	Track
*/
void GuiMain::RemoveTrack(int trackId) {

	auto iterator = std::find_if(this->tracks.begin(), this->tracks.end(), [&](const Track* track) {
		return track->trackId == trackId;
		});

	(*iterator)->ClearFile();
	this->tracks.erase(iterator);
}


/**
* Return tracks / track vector
*
* @param	void
* @return	Vector	track (pointer)
*/
vector<Track*> GuiMain::GetTracks() {
	return this->tracks;
}


/**
* Return specific track with trackId
*/
Track* GuiMain::GetTrack(int trackId) {
	
	auto iterator = std::find_if(this->tracks.begin(), this->tracks.end(), [&](const Track* track) {
		return track->trackId == trackId;
	});

	if (iterator == this->tracks.end())
		return nullptr;
	else
		return (*iterator);
}


/**
* This method returns the path to the executable
*
* @return	Path to executable	string (UTF 8)
*/
string GuiMain::GetExePathUTF8(){	
	wchar_t code;
	wchar_t pathBuf[MAX_PATH] = {};
	GetModuleFileNameW(NULL, pathBuf, MAX_PATH);

	wstring::size_type len = std::wstring(pathBuf).find_last_of(L"\\/");
	std::string outputStr;

	// Convert wchar to UTF-8. Characters that aren't on the ANSI code page (0-127) are replaced by '?' characters
	size_t i = 0;
	while (pathBuf[i] != '\0' && i <= len) {
		code = pathBuf[i];
		if (code < 128)
			//outputStr[i] = char(code);
			outputStr.push_back(char(code));
		else {
			outputStr[i] = '?';
			if (code >= 0xD800 && code <= 0xD8FF)
				// lead surrogate, skip the next code unit, which is the trail
				i++;
		}
		i++;
	}
	
	return outputStr;
}



/**
* Response to Reset Processor option
*/
void GuiMain::MenuResetProcessor(wxCommandEvent& evt) {

	// Check connectivity. Close thread and reset status in case SimConnect cannot connect
	if (!cpuCom->GetSimConnect()) {
		this->ResetProcessor(cpuThread, cpuThreadCS);
		this->ResetUI("");
	}
}


/**
* Response to Traffic menu option
*/
void GuiMain::MenuShowTrafficDialog(wxCommandEvent& evt) {
	// Refresh data grid and show dialog
	trafficDialog->LoadGrid();
	trafficDialog->Show(true);
}


/**
* Response to OBS Overlay menu option
*/
void GuiMain::MenuShowOverlayDialog(wxCommandEvent& evt) {
	overlayDialog->Show(true);
}


/**
* Response to Preferences menu option
*/
void GuiMain::MenuShowOptionsDialog(wxCommandEvent& evt) {
	// Load options and show dialog
	optionsDialog->LoadOptionsDialogConfig();
	optionsDialog->Show(true);
}


/**
* Response to record button press
*/
void GuiMain::ButtonRecordOutputFile(wxCommandEvent& evt) {

	{	wxCriticalSectionLocker enter(cpuThreadCS);

		// Active Replay (Save File): Stop processor  
		if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && fdrFile->GetFileType() == FDRFile::FileType::saveFile) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);
			newRecording = true;
		}
		// Active Replay (Temp Recording File): Stop processor
		else   if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && fdrFile->GetFileType() == FDRFile::FileType::tmpFile) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);
			newRecording = false;

			// Cut temp recording file at current position, to allow seamless continuation of recording
			fdrFile->fileData.resize(cpuCom->GetCursorPos());
		}
		
		// Active Recording: Stop processor (end recording) and open save file dialog 
		if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);
			newRecording = true;

			// Create a "open file" dialog
			wxFileDialog* SaveDialog = new wxFileDialog(
				this, wxT("Save As"), wxEmptyString, wxEmptyString,
				wxT("Flight Data Recorder File (*.fdr)|*.fdr"),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

			// If the user clicks "Save" instead of "Cancel"
			if (SaveDialog->ShowModal() == wxID_OK) {
				// Save file
				fdrFile->SaveFile((std::string)SaveDialog->GetPath());
			}
		}
		// Start Recording: Initialize and start new processor and update status
		else if (!cpuThread && !cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop) {
			// Initialize new processor
			if(newRecording)
				fdrFile = new FDRFile();
			cpuThread = new Processor(this, cpuCom, fdrFile);

			// Start processor
			if (cpuThread->Run() != wxTHREAD_NO_ERROR) {
				wxLogError("Can't create thread for flight data recorder!");
				delete cpuThread;
				cpuThread = NULL;
			} 
			// Update UI and status
			else {
				cpuCom->SetProcessorState(ProcessorCom::ProcessorState::record);
				btnRecord->SetBitmap(btnRecordingPNG);
				btnRecord->SetBitmapHover(btnRecordingPNG);
				statusBarTxt->SetLabelText(wxT("Recording started"));
			}
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/** 
* Response to play inputfile button press 
*/
void GuiMain::ButtonPlayInputFile(wxCommandEvent &evt) {
	
	{	wxCriticalSectionLocker enter(cpuThreadCS);
		
		// Start playback: Initialize and start new processor and update status
		if(this->tracks.size() > 0 && !cpuThread && !cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop){

			// Initialize new processor in separate thread
			cpuThread = new Processor(this, cpuCom, tracks);

			// Start processor
			if (cpuThread->Run() != wxTHREAD_NO_ERROR) {
				wxLogError("Can't create thread for processor!");
				delete cpuThread;
				cpuThread = NULL;
			}
			// Updates UI and status
			else {
				cpuCom->SetProcessorState(ProcessorCom::ProcessorState::replay);
				cpuCom->SetReplayRate(1.0F);
				btnPlay->SetBitmap(btnPlayActivePNG);
				statusBarTxt->SetLabelText(wxT("Playback started"));
			}

		}
		// Resume playback
		else if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && cpuCom->GetReplayRate() != 1 && cpuThread && cpuCom->GetReplayRate() != 0.5F) {
			cpuCom->SetReplayRate(1.0F);
			this->ResetUI("");
			btnPlay->SetBitmap(btnPlayActivePNG);
			statusBarTxt->SetLabelText(wxT("Playback resumed"));
		}
		// Slow motion playback
		else if (cpuThread && !cpuCom->GetHalfRate() && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && (cpuCom->GetReplayRate() == 1 || cpuCom->GetReplayRate() == 0.5F)) {
			cpuCom->SetReplayRate(cpuCom->GetReplayRate()/2.0F);
			statusBarTxt->SetLabelText(wxT("x" + std::to_string(cpuCom->GetReplayRate())));
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to pause inputfile button press
*/
void GuiMain::ButtonPauseInputFile(wxCommandEvent& evt) {
	// Pause replay
	if (cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
		{	wxCriticalSectionLocker enter(cpuThreadCS);

			if (cpuThread && cpuCom->GetSimConnect() && cpuThread->IsRunning()) {
				cpuCom->SetReplayRate(0.0F);
				this->ResetUI("");
				btnPause->SetBitmap(btnPauseActivePNG);
				statusBarTxt->SetLabelText(wxT("Playback paused"));
			}
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/** 
* Response to stop inputfile button press
*/
void GuiMain::ButtonStopInputFile(wxCommandEvent &evt) {
	
	// Stop replay
	if (cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {	
		
		// Pause processor
		this->PauseProcessor(cpuThread, cpuThreadCS);

		// Check for tmp recording file and enable option to save it
		if (cpuCom->GetCursorPos() > 0 && fdrFile && fdrFile->GetFileType() == FDRFile::FileType::tmpFile) {
			// Create a "open file" dialog
			wxFileDialog* SaveDialog = new wxFileDialog(
				this, wxT("Save As"), wxEmptyString, wxEmptyString,
				wxT("Flight Data Recorder File (*.fdr)|*.fdr"),
				wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

			// If the user clicks "Save" instead of "Cancel"
			if (SaveDialog->ShowModal() == wxID_OK) {
				// Save file
				fdrFile->SaveFile((std::string)SaveDialog->GetPath());
			}
		}

		// Stop processor
		this->StopProcessor(cpuThread, cpuThreadCS);
	}

	// Active Recording: Stop processor (end recording) and open save file dialog 
	if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
		// Stop processor
		this->ResetProcessor(cpuThread, cpuThreadCS);
		newRecording = true;

		// Create a "open file" dialog
		wxFileDialog* SaveDialog = new wxFileDialog(
			this, wxT("Save As"), wxEmptyString, wxEmptyString,
			wxT("Flight Data Recorder File (*.fdr)|*.fdr"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT, wxDefaultPosition);

		// If the user clicks "Save" instead of "Cancel"
		if (SaveDialog->ShowModal() == wxID_OK) {
			// Save file
			fdrFile->SaveFile((std::string)SaveDialog->GetPath());
		}
	}

	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to fast forward button press
*/
void GuiMain::ButtonForwardInputFile(wxCommandEvent& evt) {
	{	wxCriticalSectionLocker enter(cpuThreadCS);

		if (cpuThread && !cpuCom->GetHalfRate() && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
			int simRate = cpuCom->GetReplayRate();

			if (simRate >= 1 && simRate < 16)
				cpuCom->SetReplayRate(simRate * 2);
			else if (simRate <= -1 || simRate == 0)
				cpuCom->SetReplayRate(2.0F);

			this->ResetUI("");
			btnForward->SetBitmap(btnForwardActivePNG);
			statusBarTxt->SetLabelText(wxT("x" + std::to_string(cpuCom->GetReplayRate())));
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to fast rewind button press
*/
void GuiMain::ButtonRewindInputFile(wxCommandEvent& evt) {
	{	wxCriticalSectionLocker enter(cpuThreadCS);
		
		// Active Replay
		if (cpuThread && !cpuCom->GetHalfRate() && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
			int replayRate = cpuCom->GetReplayRate();

			if (replayRate <= -1 && replayRate > -16)
				cpuCom->SetReplayRate(replayRate * 2);
			else if (replayRate >= 1 || replayRate == 0)
				cpuCom->SetReplayRate(-2);

			this->ResetUI("");
			btnRewind->SetBitmap(btnRewindActivePNG);
			statusBarTxt->SetLabelText(wxT("x" + std::to_string(cpuCom->GetReplayRate())));

		}
		// Active Recording: Stop Recording and start (reverse replay)
		else if (cpuThread && cpuCom->GetSimConnect() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);

			// Initialize new processor
			cpuThread = new Processor(this, cpuCom, fdrFile);
		
			// Start processor
			if (cpuThread->Run() != wxTHREAD_NO_ERROR) {
				wxLogError("Can't create thread for processor!");
				delete cpuThread;
				cpuThread = NULL;
			}
			// Updates UI and status
			else {
				cpuCom->SetProcessorState(ProcessorCom::ProcessorState::replay);
				cpuCom->SetReplayRate(-2);
				this->ResetUI("");
				btnRewind->SetBitmap(btnRewindActivePNG);
				statusBarTxt->SetLabelText(wxT("x" + std::to_string(cpuCom->GetReplayRate())));
			}
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to quit application menu option
*/
void GuiMain::QuitApp(wxCommandEvent& evt)
{
	// Check for, and delete main processor thread
	{	wxCriticalSectionLocker enter(cpuThreadCS);

	if (cpuThread)
	{
		if (cpuThread->Delete() != wxTHREAD_NO_ERROR)
			wxLogError("Can't delete the thread!");
	}
	}

	Close(true);
}


/**
* Response to about help menu option

*/
void GuiMain::MenuShowHelpWindow(wxCommandEvent& evt) {
	
	helpCtrl->DisplayContents();

	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to about menu option
*/
void GuiMain::MenuShowAboutDialog(wxCommandEvent& WXUNUSED(event)){
	    wxAboutDialogInfo info;
	     info.SetName(L"Flight Data Recorder");
	     info.SetVersion(L"0.3.0 Alpha");
	     info.SetDescription(
			L"Flight Data Recorder for MS FS2021\n"
			L"");
		 info.SetWebSite(L"https://www.youtube.com/c/FSWindowSeat");
	     info.SetCopyright(L"(C) 2021 FSWindowSeat <muppetlabs@fswindowseat.com>");
		 info.SetLicence(
			 L"This program is free software: you can redistribute it and/or modify "
			 L"it under the terms of the GNU General Public License as published by "
			 L"the Free Software Foundation, either version 3 of the License, or "
			 L"(at your option) any later version.\n"
			 L"\n"
			 L"This program is distributed in the hope that it will be useful,"
			 L"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			 L"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
			 L"See the GNU General Public License for more details.\n"
			 L"\n"
			 L"You should have received a copy of the GNU General Public License "
			 L"along with this program. If not, see < https://www.gnu.org/licenses/>");
	
		 wxAboutBox(info);
}
