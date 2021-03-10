#include <GuiMain.h>
#include <Processor.h>
#include <ProcessorCom.h>
#include <FDRFile.h>
#include <FTDISFile.h>


/** 
* Definition events and event table 
*/
wxBEGIN_EVENT_TABLE(GuiMain, wxFrame)
	// Menu Items
	EVT_MENU(wxID_OPEN, GuiMain::LoadInputFile)
	EVT_MENU(wxID_EXIT, GuiMain::QuitApp)
	EVT_MENU(wxID_HELP, GuiMain::ShowHelpWindow)
	EVT_MENU(wxID_ABOUT, GuiMain::ShowAboutDialog)

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
GuiMain::GuiMain() : wxFrame(nullptr, wxID_ANY, "Flight Data Recorder", wxDefaultPosition, wxSize(400,149), (wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER)) {

	cpuCom = new ProcessorCom();
	exePathFile = wxStandardPaths::Get().GetExecutablePath();
	SetIcon(wxICON(FDRIcon));
	SetBackgroundColour(wxColour(69,69,69));

	// Definition Menu Bar
	menuBar = new wxMenuBar;
	file = new wxMenu;
	file->Append(wxID_OPEN, wxT("&Open"), wxT("Open a data recorder file"));
	file->AppendSeparator();
	file->Append(wxID_EXIT, wxT("&Quit"));
	menuBar->Append(file, wxT("&File"));
	
	help = new wxMenu;
	help->Append(wxID_HELP, wxT("&View Help"), wxT("View help information"));
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
	statusBar = CreateStatusBar(2);
	int widths[] = { -1, 150};
	statusBar->SetStatusWidths(2, widths);
	statusBar->SetBackgroundColour(wxColour(190, 190, 190));

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
}


/**
* Pauses the processor / worker thread
*/
bool GuiMain::PauseProcessor(Processor* cpuHandler, wxCriticalSection& critSection) {
	// Stop processor
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
* Handles deletion of main processor / worker thread
*/
bool GuiMain::StopProcessor(Processor *cpuHandler, wxCriticalSection& critSection) {
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
			if (!cpuCom->GetSimAPI())
				break;
		}
		// wait for thread completion
		wxThread::This()->Sleep(1);
		
		if (t == timeOut)
			return false;	
		t++;
	}
	
	// Reset  UI
	this->ResetUI("");
	
	return true;
}


/**
* Handles processor / worker thread updates
*/
void GuiMain::OnThreadUpdate(wxCommandEvent& evt) {
	// Update "Frame" label
	text->SetLabel(std::to_string(cpuCom->GetCursorPos()));

	// Check connectivity. Close thread and reset status in case SimConnect cannot connect
	if (!cpuCom->GetSimAPI()) {
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
	statusBar->SetStatusText(msg, 1);
}


/** 
* Response to load file menu option 
*/
void GuiMain::LoadInputFile(wxCommandEvent &evt) {
	
	// Active Replay: Stop processor
	if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
		// Stop processor
		this->StopProcessor(cpuThread, cpuThreadCS);
	} 
	// Active Recording: Stop processor (end recording) and open save file dialog 
	if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
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
	if (OpenDialog->ShowModal() == wxID_OK)
	{
		// Set file type, Flight Data Recorder = 0, FTDIS = 1 
		fileType = OpenDialog->GetFilterIndex();

		// Load Flight Data Recorder file
		if(fileType == 0){
			fdrFile = new FDRFile();
			// Load input file
			if (fdrFile->LoadFile((std::string)OpenDialog->GetPath())) {
				// Update status and amend title to show file name
				SetTitle(wxString("Flight Data Recorder - ") << OpenDialog->GetFilename());

				if (fdrFile->fileData.size() > 0) {
					fileLoaded = true;
					cpuCom->SetCursorPos(0);
					statusBar->SetStatusText(wxT("FDR input file loaded!"), 1);
				}
			}
		}
		//Load FTDIS file
		else if(fileType == 1){
			ftdisFile = new FTDISFile();
			// Load input file
			if (ftdisFile->LoadFile((std::string)OpenDialog->GetPath())) {
				// Update status and amend title to show file name
				SetTitle(wxString("Flight Data Recorder - ") << OpenDialog->GetFilename());

				if(ftdisFile->fileData.size() > 0){
					fileLoaded = true;
					cpuCom->SetCursorPos(0);
					statusBar->SetStatusText(wxT("FTD input file loaded!"), 1);
				}
			}
		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to record button press
*/
void GuiMain::ButtonRecordOutputFile(wxCommandEvent& evt) {

	{	wxCriticalSectionLocker enter(cpuThreadCS);

		// Active Replay (Save File): Stop processor  
		if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && fdrFile->GetFileType() == FDRFile::FileType::saveFile) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);
			newRecording = true;
		}
		// Active Replay (Temp Recording File): Stop processor
		else   if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && fdrFile->GetFileType() == FDRFile::FileType::tmpFile) {
			// Stop processor
			this->StopProcessor(cpuThread, cpuThreadCS);
			newRecording = false;

			// Cut temp recording file at current position, to allow seamless continuation of recording
			fdrFile->fileData.resize(cpuCom->GetCursorPos());
		}
		
		// Active Recording: Stop processor (end recording) and open save file dialog 
		if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
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
		else if (!cpuThread && !cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop) {
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
				statusBar->SetStatusText(wxT("Recording started"), 1);
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
		if(fileLoaded && !cpuThread && !cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop){

			// Initialize new processor for corresponding file type
			if(fileType == 0) 
				cpuThread = new Processor(this, cpuCom, fdrFile);
			else if(fileType == 1)
				cpuThread = new Processor(this, cpuCom, ftdisFile);

			// Start processor
			if (cpuThread->Run() != wxTHREAD_NO_ERROR) {
				wxLogError("Can't create thread for processor!");
				delete cpuThread;
				cpuThread = NULL;
			}
			// Updates UI and status
			else {
				cpuCom->SetProcessorState(ProcessorCom::ProcessorState::replay);
				cpuCom->SetSimRate(1.0F);
				btnPlay->SetBitmap(btnPlayActivePNG);
				statusBar->SetStatusText(wxT("Playback started"), 1);
			}

		}
		// Resume playback
		else if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && cpuCom->GetSimRate() != 1 && cpuThread && cpuCom->GetSimRate() != 0.5F) {
			cpuCom->SetSimRate(1.0F);
			this->ResetUI("");
			btnPlay->SetBitmap(btnPlayActivePNG);
			statusBar->SetStatusText(wxT("Playback resumed"), 1);
		}
		// Slow motion playback
		else if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay && (cpuCom->GetSimRate() == 1 || cpuCom->GetSimRate() == 0.5F)) {
			cpuCom->SetSimRate(cpuCom->GetSimRate()/2.0F);
			statusBar->SetStatusText("x" + std::to_string(cpuCom->GetSimRate()), 1);
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
	if (cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
		{	wxCriticalSectionLocker enter(cpuThreadCS);

			if (cpuThread && cpuCom->GetSimAPI() && cpuThread->IsRunning()) {
				cpuCom->SetSimRate(0.0F);
				this->ResetUI("");
				btnPause->SetBitmap(btnPauseActivePNG);
				statusBar->SetStatusText(wxT("Playback paused"), 1);
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
	if (cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {	
		
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
	if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
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

	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to fast forward button press
* TODO: Change/Update MSFS2020 sim rate
*/
void GuiMain::ButtonForwardInputFile(wxCommandEvent& evt) {
	{	wxCriticalSectionLocker enter(cpuThreadCS);

		if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
			int simRate = cpuCom->GetSimRate();

			if (simRate >= 1 && simRate < 16)
				cpuCom->SetSimRate(simRate * 2);
			else if (simRate <= -1 || simRate == 0)
				cpuCom->SetSimRate(2.0F);

			this->ResetUI("");
			btnForward->SetBitmap(btnForwardActivePNG);
			statusBar->SetStatusText("x" + std::to_string(cpuCom->GetSimRate()), 1);

		}
	}
	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to fast rewind button press
* TODO: Change/Update MSFS2020 sim rate
*/
void GuiMain::ButtonRewindInputFile(wxCommandEvent& evt) {
	{	wxCriticalSectionLocker enter(cpuThreadCS);
		
		// Active Replay
		if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::replay) {
			int simRate = cpuCom->GetSimRate();

			if (simRate <= -1 && simRate > -16)
				cpuCom->SetSimRate(simRate * 2);
			else if (simRate >= 1 || simRate == 0)
				cpuCom->SetSimRate(-2);

			this->ResetUI("");
			btnRewind->SetBitmap(btnRewindActivePNG);
			statusBar->SetStatusText("x" + std::to_string(cpuCom->GetSimRate()), 1);

		}
		// Active Recording: Stop Recording and start (reverse replay)
		else if (cpuThread && cpuCom->GetSimAPI() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::record) {
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
				cpuCom->SetSimRate(-2);
				this->ResetUI("");
				btnRewind->SetBitmap(btnRewindActivePNG);
				statusBar->SetStatusText("x" + std::to_string(cpuCom->GetSimRate()), 1);
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
void GuiMain::ShowHelpWindow(wxCommandEvent& evt) {
	
	helpCtrl->DisplayContents();

	// Event has been handled, skip checking parents
	evt.Skip();
}


/**
* Response to about menu option
*/
void GuiMain::ShowAboutDialog(wxCommandEvent& WXUNUSED(event)){
	    wxAboutDialogInfo info;
	     info.SetName(L"Flight Data Recorder");
	     info.SetVersion(L"0.1.0 Alpha");
	     info.SetDescription(
			L"Flight Data Recorder for MS FS2020\n"
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
