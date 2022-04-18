#include "GuiTrafficDialog.h"
#include "GuiMain.h"
#include "ProcessorCom.h"
#include "Track.h"
#include "FDRFile.h"
#include "FTDISFile.h"

/**
* Definition events and event table
*/
wxBEGIN_EVENT_TABLE(GuiTrafficDialog, wxDialog)
	// Buttons
	EVT_BUTTON(wxID_ADD, GuiTrafficDialog::ButtonAddTrack)
	EVT_BUTTON(wxID_SAVE, GuiTrafficDialog::ButtonSaveOptions)
wxEND_EVENT_TABLE()

/**
* Constructor. Defines and initializes GUI components
*/
	GuiTrafficDialog::GuiTrafficDialog(GuiMain* guiMain, ProcessorCom* com) : wxDialog(NULL, -1, wxT("AI Traffic"), wxDefaultPosition, wxSize(535, 465)) {
	// Set handler / references
	this->guiMain = guiMain;
	this->cpuCom = guiMain->cpuCom;
	
	// Load Config from DB
	this->GetTrafficDialogConfig();

	//Define UI Panel
	wxPanel* panel = new wxPanel(this, -1);
	panel->SetBackgroundColour(wxColour(69, 69, 69));
	panel->SetForegroundColour(wxColour(221, 221, 221));

	// -----------------------------------------------------------
	// Box: Add new track
	wxStaticBox* trackNew = new wxStaticBox(panel, -1, wxT("Add Track"), wxPoint(5, 5), wxSize(510, 55));
	headingFont = trackNew->GetFont();
	headingFont.SetWeight(wxFONTWEIGHT_BOLD);
	trackNew->SetFont(headingFont);

	wxArrayString simApiList;
	for (auto& simApi : simApis) {
		simApiList.Add(wxString::Format(wxT("%i"), simApi.simApiId));
	}
	wxStaticText* selectSimApiTxt = new wxStaticText(panel, -1, "API", wxPoint(15, 30));
	selectSimApi = new wxComboBox(panel, -1, wxString::Format(wxT("%i"), simApis.at(0).simApiId), wxPoint(55, 25), wxSize(50, 26), simApiList);
	selectSimApi->SetBackgroundColour(wxColour(31, 31, 31));
	selectSimApi->SetForegroundColour(wxColour(255, 255, 255));

	int listCrsr = 0;
	wxArrayString simTypeLiveryList;
	for (auto& simObject : simObjects) {
		char* typeLivery = new char[strlen(simObject.type) + strlen(simObject.livery) + 2]; //+ 2 chars for space/delimeter and terminating char
		strcpy(typeLivery, simObject.type);
		strcat(typeLivery, " ");
		strcat(typeLivery, simObject.livery);
		simTypeLiveryList.Add(typeLivery);
		delete [] typeLivery;
		listCrsr++;
	}
	wxStaticText* simObject= new wxStaticText(panel, -1, "Model", wxPoint(120, 30));
	selectSimObject = new wxComboBox(panel, -1, simTypeLiveryList[0], wxPoint(160, 26), wxSize(140, 25), simTypeLiveryList);
	selectSimObject->SetBackgroundColour(wxColour(31, 31, 31));
	selectSimObject->SetForegroundColour(wxColour(255, 255, 255));

	wxStaticText* timeOffsetTxt = new wxStaticText(panel, -1, "Offset", wxPoint(315, 30));
	timeOffset = new wxTextCtrl(panel, -1, wxT("0"), wxPoint(353, 25), wxSize(42, 25));
	timeOffset->SetBackgroundColour(wxColour(31, 31, 31));
	timeOffset->SetForegroundColour(wxColour(255, 255, 255));

	wxButton* addTrackButton = new wxButton(panel, wxID_ADD, wxT("Load File"), wxPoint(410, 23), wxSize(90, 30));
	addTrackButton->SetBackgroundColour(wxColour(48, 48, 48));
	addTrackButton->SetForegroundColour(wxColour(221, 221, 221));
	
	// -----------------------------------------------------------
	// Box: List tracks
	wxStaticBox* trackList = new wxStaticBox(panel, -1, wxT("Loaded Tracks"), wxPoint(5, 70), wxSize(510, 308));
	trackList->SetFont(headingFont);

	// Define table and layout
	grid = new wxGrid(panel, wxID_ANY, wxPoint(15, 115), wxSize(489, 250));
	grid->CreateGrid(0, 5);
	grid->SetColLabelSize(0);
	grid->SetRowLabelSize(0);
	grid->SetLabelBackgroundColour(wxColour(48, 48, 48));
	grid->SetLabelTextColour(wxColour(221, 221, 221));
	grid->SetGridLineColour(wxColour(48, 48, 48));
	grid->SetDefaultCellBackgroundColour(wxColour(31, 31, 31));
	grid->SetDefaultCellTextColour(wxColour(255, 255, 255));
	grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_CENTER);
	
	//Column: Track ID (hidden)
	grid->HideCol(0);
	
	//Column: Sim API 
	wxStaticText* gridSimApiLabel = new wxStaticText(panel, -1, "API", wxPoint(15, 98), wxSize(74, 10), wxALIGN_CENTER);
	gridSimApiLabel->SetFont(headingFont);
	gridSimApiLabel->SetBackgroundColour(wxColour(48, 48, 48));	
	grid->SetColSize(1, 75);
	grid->SetColLabelValue(1, "API");
	wxGridCellEditor* simApiColumnEditor = new wxGridCellChoiceEditor(simApiList, false);
	wxGridCellAttr* simApiColumnAttr = new wxGridCellAttr;
	simApiColumnAttr->SetEditor(simApiColumnEditor);
	simApiColumnAttr->SetReadOnly();
	grid->SetColAttr(1, simApiColumnAttr);

	//Column: Sim Object
	wxStaticText* gridSimObjectLabel = new wxStaticText(panel, -1, "Model", wxPoint(90, 98), wxSize(191, 10), wxALIGN_CENTER);
	gridSimObjectLabel->SetFont(headingFont);
	gridSimObjectLabel->SetBackgroundColour(wxColour(48, 48, 48));
	grid->SetColSize(2, 192); 
	grid->SetColLabelValue(2, "Model");
	wxGridCellEditor* simObjectColumnEditor = new wxGridCellChoiceEditor(simTypeLiveryList, false);
	wxGridCellAttr* simObjectColumnAttr = new wxGridCellAttr;
	simObjectColumnAttr->SetEditor(simObjectColumnEditor);
	simObjectColumnAttr->SetReadOnly();
	grid->SetColAttr(2, simObjectColumnAttr);

	//Column: Time Offset
	wxStaticText* gridTimeOffsetLabel = new wxStaticText(panel, -1, "Offset", wxPoint(282, 98), wxSize(44, 10), wxALIGN_CENTER);
	gridTimeOffsetLabel->SetFont(headingFont);
	gridTimeOffsetLabel->SetBackgroundColour(wxColour(48, 48, 48));
	grid->SetColSize(3, 45);
	grid->SetColLabelValue(3, "Offset");
	grid->SetColFormatNumber(3);
	
	//Column: File
	wxStaticText* gridFileLabel = new wxStaticText(panel, -1, "File", wxPoint(327, 98), wxSize(159, 10), wxALIGN_CENTER);
	gridFileLabel->SetFont(headingFont);
	gridFileLabel->SetBackgroundColour(wxColour(48, 48, 48));
	grid->SetColSize(4, 160);
	grid->SetColLabelValue(4, "File");
	wxGridCellAttr* fileColumnAttr = new wxGridCellAttr;
	fileColumnAttr->SetReadOnly();
	grid->SetColAttr(4, fileColumnAttr);

	//Colum: Vertical Scrollbar
	wxStaticText* gridScrollbarLabel = new wxStaticText(panel, -1, " ", wxPoint(487, 98), wxSize(17, 10), wxALIGN_CENTER);
	gridScrollbarLabel->SetFont(headingFont);
	gridScrollbarLabel->SetBackgroundColour(wxColour(48, 48, 48));

	// Rows
	grid->SetDefaultRowSize(25);
	grid->AppendRows(100);

	// Event bindings
	grid->Bind(wxEVT_GRID_CELL_LEFT_CLICK, &GuiTrafficDialog::LeftClickCell, this);
	grid->Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &GuiTrafficDialog::RightClickCell, this);

	// Save button
	wxButton* saveButton = new wxButton(panel, wxID_SAVE, wxT("Save"), wxPoint(230, 388), wxSize(70, 30));
	saveButton->SetBackgroundColour(wxColour(48, 48, 48));
	saveButton->SetForegroundColour(wxColour(221, 221, 221));

	// Populate Grid
	this->LoadGrid();
}


/**
* Response to load file button press
*/
void GuiTrafficDialog::ButtonAddTrack(wxCommandEvent& evt) {
	// Save data 
	this->SaveGrid();
	
	// Load new track from file
	this->LoadTrack(0);
	
	// Reload grid
	this->LoadGrid();
}


/**
* Response to left click cell event
*
* @param	Address of wxCommandEvent
* @return	void
*/
void GuiTrafficDialog::LeftClickCell(wxGridEvent& evt) {
	if (cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop) {
		// Show dropdown: Model & Object columns
		if ((evt.GetCol() == 1 || evt.GetCol() == 2) && !this->grid->GetCellValue(evt.GetRow(), 0).IsEmpty()) {
			this->grid->SetGridCursor(evt.GetRow(), evt.GetCol());
			this->grid->ShowCellEditControl();
		}
		// Open File dialog: File column
		else if (evt.GetCol() == 4 && !this->grid->GetCellValue(evt.GetRow(), 0).IsEmpty()) {
			// Change track
			this->LoadTrack(std::stod(this->grid->GetCellValue(evt.GetRow(), 0).ToStdString()));
			// Save data & reload grid
			this->SaveGrid();
			this->LoadGrid();
		}
	}
	evt.Skip();
}


/**
* Response to left click cell event
* 
* @param	Address of wxCommandEvent
* @return	void
*/
void GuiTrafficDialog::RightClickCell(wxGridEvent& evt) {
	int i = 0;
	// Context menu / delete track
	if (!this->grid->GetCellValue(evt.GetRow(), 0).IsEmpty() && cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop) {
		wxMessageDialog* dial = new wxMessageDialog(NULL, _("Do you really want to delete this track?"), _("Delete Track"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
		
		switch (dial->ShowModal()) {
		case wxID_YES:
			// Remove track
			this->guiMain->RemoveTrack(std::stod(this->grid->GetCellValue(evt.GetRow(), 0).ToStdString()));
			break;
		case wxID_NO:
			/* do something */
			break;
		default: /* ignore or handle error */;
		};

		// Save data & reload grid
		this->SaveGrid();
		this->LoadGrid();
	}
	evt.Skip();
}



/**
* Response to save button press
*/
void GuiTrafficDialog::ButtonSaveOptions(wxCommandEvent& evt) {
	// Save data
	this->SaveGrid();

	// Reset UI & close
	SetTitle(wxString("AI Traffic"));
	this->Close();
}


/**
* Load track file and add to / update track vector
*/
void GuiTrafficDialog::LoadTrack(int trackId) {
	if (cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop) {
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

			// Create new track
			if (trackId == 0) {
				track = new Track(Track::TrackType::AI);
				int newTrackId = guiMain->GetTracks().size() + 1;
				track->SetTrackId(newTrackId);
			}
			// Update existing track
			else {
				track = guiMain->GetTrack(trackId);
				track->ClearFile();
			}
			// Load Flight Data Recorder file (.fdr)
			if (fileType == 0) {
				fdrFile = new FDRFile();
				// Load input file
				if (fdrFile->LoadFile((std::string)OpenDialog->GetPath())) {
					// Update status and amend title to show file name
					SetTitle(wxString("AI Traffic - Added ") << OpenDialog->GetFilename());

					// Create new track, assign file (address) and add track (address) to vector
					if (fdrFile->fileData.size() > 0) {
						track->SetFileName(OpenDialog->GetFilename().ToStdString());
						track->SetFileType(Track::FileType::FDR);
						track->SetFile(fdrFile);
					}

				}
			}
			//Load FTDIS file (.ftd)
			else if (fileType == 1) {
				ftdisFile = new FTDISFile();
				// Load input file
				if (ftdisFile->LoadFile((std::string)OpenDialog->GetPath())) {
					// Update status and amend title to show file name
					SetTitle(wxString("AI Traffic - Added ") << OpenDialog->GetFilename());

					// Create new track, assign file (address) and add track (address) to vector
					if (ftdisFile->fileData.size() > 0) {
						track->SetFileName(OpenDialog->GetFilename().ToStdString());
						track->SetFileType(Track::FileType::FTDIS);
						track->SetFile(ftdisFile);
					}
				}
			}
			// Add new track
			if (trackId == 0 && ((fdrFile && fdrFile->fileData.size() > 0) || (ftdisFile && ftdisFile->fileData.size() > 0))) {
				// Amend new track data
				track->SetSimApi(std::stod(this->selectSimApi->GetValue().ToStdString()));
				track->SetSimTypeLivery(this->selectSimObject->GetValue().ToStdString());
				if(this->selectSimObject->GetSelection() < 0)
					track->SetSimCfgUuid(simObjects.at(0).cfgUuid);
				else
					track->SetSimCfgUuid(simObjects.at(this->selectSimObject->GetSelection()).cfgUuid);
				track->SetTimeOffset(std::stod(this->timeOffset->GetValue().ToStdString()));

				// Add new track to vector
				guiMain->AddTrack(track);
			}
		}
	}
	else {
		wxMessageDialog* dial = new wxMessageDialog(NULL, _("Cannot load tracks during active replay. Please stop replay first."), _("Active Replay"), wxOK_DEFAULT | wxICON_ERROR);
		dial->ShowModal();
	}
}


/**
* Load grid with track data
*/
void GuiTrafficDialog::LoadGrid() {
	this->grid->ClearGrid();
	
	int aiTrackCrsr = 0;
	for (auto& track : guiMain->GetTracks()) {
		if (track->GetTrackType() == Track::TrackType::AI) {
			// Populate fields
			this->grid->SetCellValue(aiTrackCrsr, 0, std::to_string(track->GetTrackId()));
			this->grid->SetCellValue(aiTrackCrsr, 1, std::to_string(track->GetSimApi()));
			this->grid->SetCellValue(aiTrackCrsr, 2, track->GetSimTypeLivery());
			this->grid->SetCellValue(aiTrackCrsr, 3, std::to_string((int)track->GetTimeOffset()));
			this->grid->SetCellValue(aiTrackCrsr, 4, track->GetFileName());
			
			// Make fields editable
			bool readOnly = true;
			if (cpuCom->GetProcessorState() == ProcessorCom::ProcessorState::stop)
				readOnly = false;

			this->grid->SetReadOnly(aiTrackCrsr, 1, readOnly);
			this->grid->SetReadOnly(aiTrackCrsr, 2, readOnly);
			this->grid->SetReadOnly(aiTrackCrsr, 3, readOnly);

			aiTrackCrsr++;
		}
	}
}


/**
* Save grid data to track(s)
*/
void GuiTrafficDialog::SaveGrid() {
	Track* track = nullptr;
	int row = 0;
	
	// Loop through (filled) rows, find corresponding track based on track id (hidden in col 0) and update data accordingly
	while (row < 100) {
		if (!this->grid->GetCellValue(row, 0).IsEmpty()) {
			track = this->guiMain->GetTrack(std::stod(this->grid->GetCellValue(row, 0).ToStdString()));
			if(track) {

				track->SetSimApi(std::stod(this->grid->GetCellValue(row, 1).ToStdString()));
				
				track->SetSimTypeLivery(this->grid->GetCellValue(row, 2).ToStdString());
				
				// Get index of selected value of GridCellChoiceEditor 
				// Note: Yes I have to do it this way, as there is no way to get the index of the 
				//		 selected item from the GridCellChoiceEditor directly. It isn't elegant, but it works
				for (int i = 0; i < simObjects.size(); i++) {
					char* typeLivery = new char[strlen(simObjects.at(i).type) + strlen(simObjects.at(i).livery) + 2]; //+ 2 chars for space/delimeter and terminating char
					strcpy(typeLivery, simObjects.at(i).type);
					strcat(typeLivery, " ");
					strcat(typeLivery, simObjects.at(i).livery);
					
					if(this->grid->GetCellValue(row, 2) == typeLivery){
						track->SetSimCfgUuid(simObjects.at(i).cfgUuid);
						break;
					}
					delete[] typeLivery;
				}	
				
				track->SetTimeOffset(std::stod(this->grid->GetCellValue(row, 3).ToStdString()));
			}
			row++;
		}
		else {
			break;
		}
	}
}


/**
* Get APIs and models from config database
*/
void GuiTrafficDialog::GetTrafficDialogConfig() {	
	sqlite3* dbCon = nullptr;
	sqlite3_stmt* stmt = nullptr;
	string appPathFile = this->guiMain->GetExePathUTF8() + "config.db";

	int rc = sqlite3_open(appPathFile.c_str(), &dbCon);

	if (rc == SQLITE_OK) {

		// Get APIs
		rc = sqlite3_prepare_v2(dbCon, "SELECT * FROM simapi ORDER BY api", -1, &stmt, NULL);
		while (sqlite3_step(stmt) != SQLITE_DONE) {
			SimApi simApi;
			simApi.recId = sqlite3_column_int(stmt, 0);
			simApi.simApiId = sqlite3_column_int(stmt, 1);
			simApis.push_back(simApi);
		}
		sqlite3_finalize(stmt);
		
		// Get Sim Models
		rc = sqlite3_prepare_v2(dbCon, "SELECT * FROM simobject ORDER BY type, livery", -1, &stmt, NULL);
		while (sqlite3_step(stmt) != SQLITE_DONE) {
			SimObject simObject;
			simObject.recId = sqlite3_column_int(stmt, 0);
			simObject.type = _strdup((const char*)sqlite3_column_text(stmt, 1));
			simObject.livery = _strdup((const char*)sqlite3_column_text(stmt, 2));
			simObject.cfgUuid = _strdup((const char*)sqlite3_column_text(stmt, 3));
			simObjects.push_back(simObject);
		}
		sqlite3_finalize(stmt);
		
		sqlite3_close(dbCon);
	}
}
