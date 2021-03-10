
#include <GuiApp.h>

// Inlcude macro to create WinMain
wxIMPLEMENT_APP(GuiApp);

GuiApp::GuiApp() {
}

GuiApp::~GuiApp() {
}

/**
* Initializes the GUI's main window
*/
bool GuiApp::OnInit() {
	wxInitAllImageHandlers();
	mainWindow = new GuiMain();
	mainWindow->Show();
	return true;
}