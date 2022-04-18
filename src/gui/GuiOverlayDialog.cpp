#include "GuiOverlayDialog.h"
#include "GuiMain.h"
#include "ProcessorCom.h"

/**
* Definition events and event table
*/
wxBEGIN_EVENT_TABLE(GuiOverlayDialog, wxDialog)
    // Buttons
    EVT_BUTTON(wxID_SAVE, GuiOverlayDialog::ButtonSaveOptions)
wxEND_EVENT_TABLE()


/**
* Constructor. Defines and initializes GUI components
*/
GuiOverlayDialog::GuiOverlayDialog(GuiMain* guiMain, ProcessorCom* com) : wxDialog(NULL, -1, wxT("OBS Overlay"), wxDefaultPosition, wxSize(535, 273)) {
    // Set handler / references
    this->guiMain = guiMain;
    this->cpuCom = com;

    // Define UI Panel
    wxPanel* panel = new wxPanel(this, -1);
    panel->SetBackgroundColour(wxColour(69, 69, 69));
    panel->SetForegroundColour(wxColour(221, 221, 221));

    // Box: Overlay Reference Data Fields
    wxStaticBox* overlay = new wxStaticBox(panel, -1, wxT("Reference Data"), wxPoint(5, 5), wxSize(510, 180));
    headingFont = overlay->GetFont();
    headingFont.SetWeight(wxFONTWEIGHT_BOLD);
    overlay->SetFont(headingFont);

    // Departure Time
    wxDateTime zeroDateTime = wxDateTime(wxDateTime::Now());
    zeroDateTime.SetHour(0);
    zeroDateTime.SetMinute(0);
    zeroDateTime.SetSecond(0);

    wxStaticText* depTimeTxt = new wxStaticText(panel, -1, "Local Departure Time", wxPoint(15, 30));
    depTime = new wxTimePickerCtrl(panel, -1, zeroDateTime, wxPoint(170, 25));
    depTime->SetBackgroundColour(wxColour(*wxRED));
    //depTime->SetBackgroundColour(wxColour(31, 31, 31));
    //depTime->SetForegroundColour(wxColour(31, 31, 31));

    // Departure GMT
    wxArrayString gmtZones;
    gmtZones.Add(wxT("(GMT-12:00) International Date Line West"));
    gmtZones.Add(wxT("(GMT-11:00) Midway Island, Samoa"));
    gmtZones.Add(wxT("(GMT-10:00) Hawaii"));
    gmtZones.Add(wxT("(GMT-09:00) Alaska"));
    gmtZones.Add(wxT("(GMT-08:00) Pacific Time (US and Canada); Tijuana"));
    gmtZones.Add(wxT("(GMT-07:00) Mountain Time (US and Canada)"));
    gmtZones.Add(wxT("(GMT-07:00) Chihuahua, La Paz, Mazatlan"));
    gmtZones.Add(wxT("(GMT-07:00) Arizona"));
    gmtZones.Add(wxT("(GMT-06:00) Central Time (US and Canada"));
    gmtZones.Add(wxT("(GMT-06:00) Saskatchewan"));
    gmtZones.Add(wxT("(GMT-06:00) Guadalajara, Mexico City, Monterrey"));
    gmtZones.Add(wxT("(GMT-06:00) Central America"));
    gmtZones.Add(wxT("(GMT-05:00) Eastern Time (US and Canada)"));
    gmtZones.Add(wxT("(GMT-05:00) Indiana (East)"));
    gmtZones.Add(wxT("(GMT-05:00) Bogota, Lima, Quito"));
    gmtZones.Add(wxT("(GMT-04:00) Atlantic Time (Canada)"));
    gmtZones.Add(wxT("(GMT-04:00) Caracas, La Paz"));
    gmtZones.Add(wxT("(GMT-04:00) Santiago"));
    gmtZones.Add(wxT("(GMT-03:30) Newfoundland and Labrador"));
    gmtZones.Add(wxT("(GMT-03:00) Brasilia"));
    gmtZones.Add(wxT("(GMT-03:00) Buenos Aires, Georgetown"));
    gmtZones.Add(wxT("(GMT-03:00) Greenland"));
    gmtZones.Add(wxT("(GMT-02:00) Mid-Atlantic"));
    gmtZones.Add(wxT("(GMT-01:00) Azores"));
    gmtZones.Add(wxT("(GMT-01:00) Cape Verde Islands"));
    gmtZones.Add(wxT("(GMT 00:00) Dublin, Edinburgh, Lisbon, London"));
    gmtZones.Add(wxT("(GMT 00:00) Casablanca, Monrovia"));
    gmtZones.Add(wxT("(GMT+01:00) Belgrade, Bratislava, Budapest, Prague"));
    gmtZones.Add(wxT("(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb"));
    gmtZones.Add(wxT("(GMT+01:00) Brussels, Copenhagen, Madrid, Paris"));
    gmtZones.Add(wxT("(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm"));
    gmtZones.Add(wxT("(GMT+01:00) West Central Africa"));
    gmtZones.Add(wxT("(GMT+02:00) Bucharest"));
    gmtZones.Add(wxT("(GMT+02:00) Cairo"));
    gmtZones.Add(wxT("(GMT+02:00) Helsinki, Kiev, Riga, Sofia, Tallinn, Vilnius"));
    gmtZones.Add(wxT("(GMT+02:00) Athens, Istanbul, Minsk"));
    gmtZones.Add(wxT("(GMT+02:00) Jerusalem"));
    gmtZones.Add(wxT("(GMT+02:00) Harare, Pretoria"));
    gmtZones.Add(wxT("(GMT+03:00) Moscow, St. Petersburg, Volgograd"));
    gmtZones.Add(wxT("(GMT+03:00) Kuwait, Riyadh"));
    gmtZones.Add(wxT("(GMT+03:00) Nairobi"));
    gmtZones.Add(wxT("(GMT+03:00) Baghdad"));
    gmtZones.Add(wxT("(GMT+03:30) Tehran"));
    gmtZones.Add(wxT("(GMT+04:00) Abu Dhabi, Muscat"));
    gmtZones.Add(wxT("(GMT+04:00) Baku, Tbilisi, Yerevan"));
    gmtZones.Add(wxT("(GMT+04:30) Kabul"));
    gmtZones.Add(wxT("(GMT+05:00) Ekaterinburg"));
    gmtZones.Add(wxT("(GMT+05:00) Islamabad, Karachi, Tashkent"));
    gmtZones.Add(wxT("(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi"));
    gmtZones.Add(wxT("(GMT+05:45) Kathmandu"));
    gmtZones.Add(wxT("(GMT+06:00) Astana, Dhaka"));
    gmtZones.Add(wxT("(GMT+06:00) Sri Jayawardenepura"));
    gmtZones.Add(wxT("(GMT+06:00) Almaty, Novosibirsk"));
    gmtZones.Add(wxT("(GMT+06:30) Yangon Rangoon"));
    gmtZones.Add(wxT("(GMT+07:00) Bangkok, Hanoi, Jakarta"));
    gmtZones.Add(wxT("(GMT+07:00) Krasnoyarsk"));
    gmtZones.Add(wxT("(GMT+08:00) Beijing, Chongqing, Hong Kong SAR, Urumq"));
    gmtZones.Add(wxT("(GMT+08:00) Kuala Lumpur, Singapore"));
    gmtZones.Add(wxT("(GMT+08:00) Taipei"));
    gmtZones.Add(wxT("(GMT+08:00) Perth"));
    gmtZones.Add(wxT("(GMT+08:00) Irkutsk, Ulaanbaatar"));
    gmtZones.Add(wxT("(GMT+09:00) Seoul"));
    gmtZones.Add(wxT("(GMT+09:00) Osaka, Sapporo, Tokyo"));
    gmtZones.Add(wxT("(GMT+09:00) Yakutsk"));
    gmtZones.Add(wxT("(GMT+09:30) Darwin"));
    gmtZones.Add(wxT("(GMT+09:30) Adelaide"));
    gmtZones.Add(wxT("(GMT+10:00) Canberra, Melbourne, Sydney"));
    gmtZones.Add(wxT("(GMT+10:00) Brisbane"));
    gmtZones.Add(wxT("(GMT+10:00) Hobart"));
    gmtZones.Add(wxT("(GMT+10:00) Vladivostok"));
    gmtZones.Add(wxT("(GMT+10:00) Guam, Port Moresby"));
    gmtZones.Add(wxT("(GMT+11:00) Magadan, Solomon Islands, New Caledonia"));
    gmtZones.Add(wxT("(GMT+12:00) Fiji Islands, Kamchatka, Marshall Islands"));
    gmtZones.Add(wxT("(GMT+12:00) Auckland, Wellington"));
    gmtZones.Add(wxT("(GMT+13:00) Nuku'alofa"));

    wxStaticText* depGMTZoneTxt = new wxStaticText(panel, -1, "GMT", wxPoint(15, 60));
    depGMTZone = new wxComboBox(panel, -1, wxT("(GMT 00:00) Dublin, Edinburgh, Lisbon, London"), wxPoint(170, 55), wxDefaultSize, gmtZones);
    depGMTZone->SetBackgroundColour(wxColour(31, 31, 31));
    depGMTZone->SetForegroundColour(wxColour(255, 255, 255));

    // Destination Name
    wxStaticText* destNameTxt = new wxStaticText(panel, -1, "Name Destination", wxPoint(15, 90));
    destName = new wxTextCtrl(panel, -1, wxT(""), wxPoint(170, 85));
    destName->SetBackgroundColour(wxColour(31, 31, 31));
    destName->SetForegroundColour(wxColour(255, 255, 255));

    // Destination GMT
    wxStaticText* destGMTZoneTxt = new wxStaticText(panel, -1, "GMT", wxPoint(15, 120));
    destGMTZone = new wxComboBox(panel, -1, wxT("(GMT 00:00) Dublin, Edinburgh, Lisbon, London"), wxPoint(170, 115), wxDefaultSize, gmtZones);
    destGMTZone->SetBackgroundColour(wxColour(31, 31, 31));
    destGMTZone->SetForegroundColour(wxColour(255, 255, 255));

    // Flight Duration
    wxStaticText* fltDurText = new wxStaticText(panel, -1, "Flight Duration", wxPoint(15, 150));
    fltDur = new wxTimePickerCtrl(panel, -1, zeroDateTime, wxPoint(170, 145));
    fltDur->SetBackgroundColour(wxColour(31, 31, 31));
    fltDur->SetForegroundColour(wxColour(31, 31, 31));

    // Save button
    wxButton* saveButton = new wxButton(panel, wxID_SAVE, wxT("Save"), wxPoint(230, 196), wxSize(70, 30));
    saveButton->SetBackgroundColour(wxColour(48, 48, 48));
    saveButton->SetForegroundColour(wxColour(221, 221, 221));
}


/**
* Response to save button press
*/
void GuiOverlayDialog::ButtonSaveOptions(wxCommandEvent& evt) {
    OptionsStruct options = cpuCom->GetOptions();
    options.depHH = depTime->GetValue().GetHour();
    options.depMM = depTime->GetValue().GetMinute();

    std::string timeStrg = depGMTZone->GetValue().ToStdString();
    options.depGMTHH = std::stoi(timeStrg.substr(timeStrg.find("GMT")+3, timeStrg.find(":")));
    options.depGMTMM = std::stoi(timeStrg.substr(timeStrg.find(":")+1, timeStrg.find(")")));

    options.destName = destName->GetValue().ToStdString();

    timeStrg = destGMTZone->GetValue().ToStdString();
    options.destGMTHH = std::stoi(timeStrg.substr(timeStrg.find("GMT") + 3, timeStrg.find(":")));
    options.destGMTMM = std::stoi(timeStrg.substr(timeStrg.find(":") + 1, timeStrg.find(")")));

    options.fltHH = fltDur->GetValue().GetHour();
    options.fltMM = fltDur->GetValue().GetMinute();
    
    cpuCom->SetOptions(options);

    this->Close();
}