#include "GuiOptionsDialog.h"
#include "GuiMain.h"
#include "ProcessorCom.h"

/**
* Definition events and event table
*/
wxBEGIN_EVENT_TABLE(GuiOptionsDialog, wxDialog)
    // Buttons
    EVT_BUTTON(wxID_SAVE, GuiOptionsDialog::ButtonSaveOptions)
wxEND_EVENT_TABLE()


/**
* Constructor. Defines and initializes GUI components.
*/
GuiOptionsDialog::GuiOptionsDialog(GuiMain* guiMain, ProcessorCom* com) : wxDialog(NULL, -1, wxT("Preferences"), wxDefaultPosition, wxSize(350, 265)) {
    // Set handler / references
    this->guiMain = guiMain;
    this->cpuCom = com;

    // Define UI
    wxPanel* panel = new wxPanel(this, -1);
    panel->SetBackgroundColour(wxColour(69, 69, 69));
    panel->SetForegroundColour(wxColour(221, 221, 221));

    // Box: Replay Options
    wxStaticBox* replay = new wxStaticBox(panel, -1, wxT("Replay"), wxPoint(5, 5), wxSize(325, 173));
    headingFont = replay->GetFont();
    headingFont.SetWeight(wxFONTWEIGHT_BOLD);
    replay->SetFont(headingFont);

    // Dropdown Sim APIs
    for (auto& simApi : simApis) {
        simApiList.Add(wxString::Format(wxT("%i"), simApi.simApiId));
    }
    wxStaticText* selectSimApiTxt = new wxStaticText(panel, -1, "Sim API", wxPoint(15, 30));
    selectSimApi = new wxComboBox(panel, -1, wxString::Format(wxT("%i"), defApi), wxPoint(90, 25), wxDefaultSize, simApiList);
    selectSimApi->SetBackgroundColour(wxColour(31, 31, 31));
    selectSimApi->SetForegroundColour(wxColour(255, 255, 255));

    wxStaticText* cbHalfRateTxt = new wxStaticText(panel, -1, "Enable Half Rate", wxPoint(15, 60));
    cbHalfRate = new wxCheckBox(panel, -1, wxT(""), wxPoint(170, 60), wxDefaultSize);

    wxStaticText* cbCustSoundTxt = new wxStaticText(panel, -1, "Enable Custom Sound", wxPoint(15, 90));
    cbCustSound = new wxCheckBox(panel, -1, wxT(""), wxPoint(170, 90), wxDefaultSize);

    wxStaticText* cbBeebSoundTxt = new wxStaticText(panel, -1, "Enable Notification Sound", wxPoint(15, 120));
    cbBeebSound = new wxCheckBox(panel, -1, wxT(""), wxPoint(170, 120), wxDefaultSize);

    wxStaticText* cbCamShakeTxt = new wxStaticText(panel, -1, "Enable Camera Shake", wxPoint(15, 150));
    cbCamShake = new wxCheckBox(panel, -1, wxT(""), wxPoint(170, 150), wxDefaultSize);
   
    wxButton* saveButton = new wxButton(panel, wxID_SAVE, wxT("Save"), wxPoint(140, 187), wxSize(70, 30));
    saveButton->SetBackgroundColour(wxColour(48, 48, 48));
    saveButton->SetForegroundColour(wxColour(221, 221, 221));

    // Load Config from DB and set UI flags
    this->LoadOptionsDialogConfig();
}


/**
* Response to save button press.
*/
void GuiOptionsDialog::ButtonSaveOptions(wxCommandEvent& evt) {
    OptionsStruct options = cpuCom->GetOptions();
    
    options.simApi = wxAtoi(selectSimApi->GetValue());
    options.halfRate = cbHalfRate->GetValue();
    options.customSound = cbCustSound->GetValue();
    options.beepStartEnd = cbBeebSound->GetValue();
    options.camShake = cbCamShake->GetValue();
    cpuCom->SetOptions(options);

    this->SaveOptionsDialogConfig();
    
    this->Close();
}


/**
* Get APIs and preferences from config database
*/
void GuiOptionsDialog::LoadOptionsDialogConfig() {
    sqlite3* dbCon = nullptr;
    sqlite3_stmt* stmt = nullptr;
    std::string appPathFile = this->guiMain->GetExePathUTF8() + "config.db";
    OptionsStruct options = cpuCom->GetOptions();

    if (sqlite3_open(appPathFile.c_str(), &dbCon) == SQLITE_OK) {

        // Get APIs
        int rc = sqlite3_prepare_v2(dbCon, "SELECT * FROM simapi", -1, &stmt, NULL);
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            SimApi simApi;
            simApi.recId = sqlite3_column_int(stmt, 0);
            simApi.simApiId = sqlite3_column_int(stmt, 1);
            simApis.push_back(simApi);
        }

        // Get and assign preferences
        rc = sqlite3_prepare_v2(dbCon, "SELECT * FROM option", -1, &stmt, NULL);
        while (sqlite3_step(stmt) != SQLITE_DONE) {
            switch (sqlite3_column_int(stmt, 0)) {
                case defApiId: {
                    defApi = sqlite3_column_int(stmt, 2);
                    break;
                }
                case halfRateId: {
                    halfRate = sqlite3_column_int(stmt, 2);
                    break;
                }
                case custSoundId: {
                    custSound = sqlite3_column_int(stmt, 2);
                    break;
                }
                case notificationId: {
                    notification = sqlite3_column_int(stmt, 2);
                    break;
                }
                case camShakeId: {
                    camShake = sqlite3_column_int(stmt, 2);
                    break;
                }
                break;
            }
        }
        // Finalize and close connection
        sqlite3_finalize(stmt);
        sqlite3_close(dbCon);
    }

    // Update values in GUI and save options to processor com
    if(selectSimApi != nullptr) {
        selectSimApi->SetValue(wxString::Format(wxT("%i"), defApi));
        options.simApi = wxAtoi(selectSimApi->GetValue());
    }
    
    if (cbHalfRate != nullptr) {
        cbHalfRate->SetValue(halfRate);
        options.halfRate = cbHalfRate->GetValue();
    }
    
    if (cbCustSound != nullptr) {
        cbCustSound->SetValue(custSound);
        options.customSound = cbCustSound->GetValue();
    }
    
    if (cbBeebSound != nullptr) {
        cbBeebSound->SetValue(notification);
        options.beepStartEnd = cbBeebSound->GetValue();
    }

    if (cbCamShake != nullptr) {
        cbCamShake->SetValue(camShake);
        options.camShake = cbCamShake->GetValue();
    }

    cpuCom->SetOptions(options);
}


/**
* Save APIs and preferences to config database
*/
void GuiOptionsDialog::SaveOptionsDialogConfig() {
    int rc;
    sqlite3* dbCon = nullptr;
    sqlite3_stmt* stmt = nullptr;
    string appPathFile = this->guiMain->GetExePathUTF8() + "config.db";

    if (sqlite3_open(appPathFile.c_str(), &dbCon) == SQLITE_OK) {
    
        const char* sql = "UPDATE option SET val = ?1 WHERE id = ?2;";
        rc = sqlite3_prepare_v3(dbCon, sql, -1, 0, &stmt, NULL);

        // Update: API
        rc = sqlite3_bind_int(stmt, 1, wxAtoi(selectSimApi->GetValue()));
        rc = sqlite3_bind_int(stmt, 2, defApiId);
        rc = sqlite3_step(stmt);
        rc = sqlite3_reset(stmt);

        // Update: Run half rate
        rc = sqlite3_bind_int(stmt, 1, cbHalfRate->GetValue());
        rc = sqlite3_bind_int(stmt, 2, halfRateId);
        rc = sqlite3_step(stmt);
        rc = sqlite3_reset(stmt);

        // Update: Custom sound
        rc = sqlite3_bind_int(stmt, 1, cbCustSound->GetValue());
        rc = sqlite3_bind_int(stmt, 2, custSoundId);
        rc = sqlite3_step(stmt);
        rc = sqlite3_reset(stmt);

        // Update: Notification sound
        rc = sqlite3_bind_int(stmt, 1, cbBeebSound->GetValue());
        rc = sqlite3_bind_int(stmt, 2, notificationId);
        rc = sqlite3_step(stmt);
        rc = sqlite3_reset(stmt);

        // Update: Cam Shake flag
        rc = sqlite3_bind_int(stmt, 1, cbCamShake->GetValue());
        rc = sqlite3_bind_int(stmt, 2, camShakeId);
        rc = sqlite3_step(stmt);
        rc = sqlite3_reset(stmt);

        // Close connection
        sqlite3_finalize(stmt);
        sqlite3_close(dbCon);

    }
}
