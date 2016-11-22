
// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "ASRAppFrame.h"
#include "wx/textfile.h"
#include "defevent.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
//#ifndef WX_PRECOMP
  //  #include "wx/wx.h"
//#endif
MyFrame1 *pframe;

#define EVT_ProcessNewText(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_ProcessNewText, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
	virtual int OnExit();
    virtual void OnUnhandledException();

};



// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MyFrame1, wxFrame)
    EVT_SIZE(MyFrame1::OnSize)
    EVT_MENU(ASRApp_Hide,  MyFrame1::OnHide)
    EVT_MENU(ASRApp_About, MyFrame1::OnAbout)
	EVT_MENU(ASRApp_Listen, MyFrame1::OnListen)
	EVT_MENU(ASRApp_Record, MyFrame1::OnRecord)
	EVT_MENU(ASRApp_Play, MyFrame1::OnPlay)
	EVT_MENU(ASRApp_Decode, MyFrame1::OnDecode)
	EVT_MENU(ASRApp_OpenFile, MyFrame1::OnOpenFile)
	EVT_MENU(ASRApp_Font, MyFrame1::OnFont)
	EVT_MENU(ASRApp_Clear, MyFrame1::OnClear)
	EVT_MENU(ASRApp_Shutdown, MyFrame1::OnShutdown)
	EVT_MENU(ASRApp_Help, MyFrame1::OnHelp)
	EVT_MENU(ASRApp_AudioParams, MyFrame1::OnAudioParams)
	EVT_LIST_ITEM_SELECTED(LIST_CTRL, MyFrame1::OnSelected)
    EVT_ProcessNewText(wxID_ANY, MyFrame1::OnProcessNewText)
	EVT_CLOSE(MyFrame1::OnQuit)
	//EVT_WINDOW_DESTROY(MyFrame1::Quit)
    EVT_END_PROCESS(wxID_ANY, MyFrame1::OnProcessTerm)
	EVT_MENU(ASRApp_SendKeyStroke, MyFrame1::OnSendKeyStroke)

END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyStatusBar, wxStatusBar)
    EVT_SIZE(MyStatusBar::OnSize)
	EVT_TIMER(wxID_ANY, MyStatusBar::OnTimer)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;
	SetVendorName(_T("IZET"));
    // create the main application window
	MyFrame1 *frame = new MyFrame1( NULL, -1, PROGRAM_TITLE, wxDefaultPosition, wxSize( 500,300 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	//wxFrame *frame = new wxFrame(NULL, -1, _T("Супер распознавалка"), wxDefaultPosition, wxSize( 500,300 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);
	frame->SetSize(wxSize(800,400));
	frame->SetIcon(wxICON(sample));
	
    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
	pframe = frame;
    return true;
}

int MyApp::OnExit(){

  return -1;
}

void MyApp::OnUnhandledException()
{
    try
    {
        throw;
    }
    catch ( ... )
    {
		throw;
    }
}


//extern MyApp & wxGetApp();
DECLARE_APP(MyApp)

enum {
    PU_RESTORE = 10001,
    PU_MIC_OFF,
    PU_MIC_ON,
    PU_EXIT,
    PU_CHECKMARK,
    PU_SUB1,
    PU_SUB2,
    PU_SUBMAIN
};


BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(PU_RESTORE, MyTaskBarIcon::OnMenuRestore)
    EVT_MENU(PU_EXIT,    MyTaskBarIcon::OnMenuExit)
    EVT_MENU(PU_MIC_OFF,MyTaskBarIcon::OnMenuMicOff)
    EVT_MENU(PU_MIC_ON,MyTaskBarIcon::OnMenuMicOn)
    EVT_MENU(PU_CHECKMARK,MyTaskBarIcon::OnMenuCheckmark)
    EVT_UPDATE_UI(PU_CHECKMARK,MyTaskBarIcon::OnMenuUICheckmark)
    EVT_TASKBAR_LEFT_DCLICK  (MyTaskBarIcon::OnLeftButtonDClick)
    EVT_MENU(PU_SUB1, MyTaskBarIcon::OnMenuSub)
    EVT_MENU(PU_SUB2, MyTaskBarIcon::OnMenuSub)
END_EVENT_TABLE()

void MyTaskBarIcon::OnMenuRestore(wxCommandEvent& )
{
    pframe->Show(true);
}

void MyTaskBarIcon::OnMenuExit(wxCommandEvent& )
{
    pframe->Close(true);
}

static bool check = true;

void MyTaskBarIcon::OnMenuCheckmark(wxCommandEvent& )
{
       check =!check;
}

void MyTaskBarIcon::OnMenuUICheckmark(wxUpdateUIEvent &event)
{
       event.Check( check );
}

void MyTaskBarIcon::OnMenuMicOff(wxCommandEvent&)
{
    //wxIcon icon(smile_xpm);

    if (!SetIcon(wxICON( mic_off), wxT("Микрофон выключен")))
        wxMessageBox(wxT("Could not set new icon."));
	if (isMicOn)
		isMicOn = pframe->Record();

}

void MyTaskBarIcon::OnMenuMicOn(wxCommandEvent&)
{
    if (IsIconInstalled())
    {
        RemoveIcon();
    }
    //else
    //{
    //    wxMessageBox(wxT("wxTaskBarIcon Sample - icon already is the old version"));
    //}
    if (!SetIcon(wxICON( mic_on), wxT("Микрофон включен")))
        wxMessageBox(wxT("Could not set new icon."));
	isMicOn = pframe->Record();
}

void MyTaskBarIcon::OnMenuSub(wxCommandEvent&)
{
    wxMessageBox(wxT("You clicked on a submenu!"));
}

// Overridables
wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
    // Try creating menus different ways
    // TODO: Probably try calling SetBitmap with some XPMs here
    wxMenu *menu = new wxMenu;
    menu->Append(PU_RESTORE, _T("&Показать"));
    menu->AppendSeparator();
	if (isMicOn)
		menu->Append(PU_MIC_OFF, _T("&Выключить микрофон"));    
	else
		menu->Append(PU_MIC_ON, _T("&Включить микрофон"));    
//    menu->Append(PU_NEW_ICON, _T("&Set New Icon"));
//    menu->AppendSeparator();
//    menu->Append(PU_CHECKMARK, _T("Checkmark"),wxT(""), wxITEM_CHECK);
//    menu->AppendSeparator();
//    wxMenu *submenu = new wxMenu;
//    submenu->Append(PU_SUB1, _T("One submenu"));
//    submenu->AppendSeparator();
//    submenu->Append(PU_SUB2, _T("Another submenu"));
//    menu->Append(PU_SUBMAIN, _T("Submenu"), submenu);
#ifndef __WXMAC_OSX__ /*Mac has built-in quit menu*/
//    menu->AppendSeparator();
    menu->Append(PU_EXIT,    _T("&Закончить"));
#endif
    return menu;
}

void MyTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
    pframe->Show(true);
}
