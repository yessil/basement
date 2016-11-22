///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Apr 16 2008)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __ASRApp__
#define __ASRApp__
#include "wx/wxprec.h"
#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextstyles.h>
#include <wx/richtext/richtextxml.h>
#include <wx/richtext/richtexthtml.h>
#include <wx/richtext/richtextformatdlg.h>
#include <wx/richtext/richtextsymboldlg.h>
#include <wx/richtext/richtextstyledlg.h>
#include <wx/richtext/richtextprint.h>
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#ifndef AUDIOTHREAD_H
	#include "AudioThread.h"
#endif
#ifndef READAUDIOTHREAD_H
	#include "ReadAudio.h"
#endif
#include "wx/cursor.h"
#include "Decoder.h"
#include "MyTaskbarIcon.h"
#include "wx/msw/ole/automtn.h"
#ifndef DEFEVENT_H
	#include "defevent.h"
#endif
#include "wx/process.h"
#define PROGRAM_TITLE _T("Демо версия модуля распознавания - 2012")
//extern "C" int text2wfreq(char* infile, char* outfile);
//extern "C" int wfreq2vocab(int vocab_size, int cutoff, char* infile, char* outfile);
//extern "C" int text2idngram(char* infile, char* outfile, char *vocab_filename);
//extern "C" int idngram2lm(int argc, char **argv);
//extern "C" int lm_convert(char* inflie, char *outfile);
///////////////////////////////////////////////////////////////////////////
class MyStatusBar : public wxStatusBar
{
public:
    MyStatusBar(wxWindow *parent);
    virtual ~MyStatusBar();

    // event handlers
    void OnSize(wxSizeEvent& event);
    void OnTimer(wxTimerEvent& WXUNUSED(event)) { UpdateClock(); }

	void UpdateClock();
	void SetValue(int value);
	void SetFlag(bool flag);
	void ResetTimer();
	void StopTimer();
    wxBitmap CreateBitmapForButton(bool on = false);
private:

	wxGauge *recGauge;
	wxBitmapButton *m_statbmp;
	wxTimer m_timer;
	int time;
	int incr;

    DECLARE_EVENT_TABLE()
};



///////////////////////////////////////////////////////////////////////////////
/// Class MyFrame1
///////////////////////////////////////////////////////////////////////////////
class MyFrame1 : public BaseFrame 
{
	private:

		void OnListen(wxCommandEvent& event);
		void OnRecord(wxCommandEvent& event);
		void OnPlay(wxCommandEvent& event);
		void OnDecode(wxCommandEvent& event);
		void OnOpenFile(wxCommandEvent& event);
		void OnSelected(wxListEvent& event);
		void OnAudioParams(wxCommandEvent& event);
		void OnProcessNewText(wxCommandEvent& event);
		void OnProcessTerm(wxProcessEvent& event);
		void OnShutdown(wxCommandEvent& event);
		void OnHelp(wxCommandEvent& event);
		void InitAudio();
		void InitDecoder();
		void AudioParams();
		void SaveParams();
		void LoadParams();
		void CheckDecoderSrv();
		wxListItem curItem;
		AudioThread *audioIO;
		ReadAudio *audioOut;
		DecoderThread *decoder;
		wxCursor *m_cursor;
		wxAutomationObject wordObject;
		bool isWordActive;
		wxConfigBase *pConfig;
		bool doModelUpdate;
		wxProcess *process;
		MyStatusBar* bar;
		char* launchdir;
		bool sendKeyStroke;
		

	protected:

		wxPanel*	m_panel1;
		wxRichTextCtrl* m_textbox;
		wxListCtrl* m_listCtrl;
		void OnSize(wxSizeEvent& event);
		static const int NUM_ITEMS = 300000;
		wxLocale m_locale;
		MyTaskBarIcon   *m_taskBarIcon;
	
	public:

		MyFrame1( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~MyFrame1();
    // event handlers (these functions should _not_ be virtual)
		void OnQuit(wxCloseEvent& event);
		void Quit();
		void OnHide(wxCommandEvent& event);
		void Hide();
		void OnAbout(wxCommandEvent& event);
		void DoSize();
		bool Record();
		virtual void WriteText(const wxString& text);// { m_textbox->WriteText(text); }
		void SetStatusbarText(wxString msg);
		void SetValue(int value);
		void SetFlag(bool flag);
		void ResetTimer();
		void StopTimer();
		void ProcessMessage(wxString msg);
		void Exec(wxString cmd);


private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
    void InitWithListItems();
	void InitWithReportItems();
	void Listen();
	void Decode();
	void OpenFile();
	void AudioParams(wxCommandEvent& event);
	void OnFont(wxCommandEvent& event);
	void OnClear(wxCommandEvent& event);
	void OnSendKeyStroke(wxCommandEvent& event);

};

// IDs for the controls and the menu commands
enum
{
    // menu items

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
	ASRApp_Quit = wxID_EXIT,
    ASRApp_About = wxID_ABOUT,
	ASRApp_Listen = wxID_HIGHEST + 1,
	ASRApp_Record = wxID_HIGHEST + 2,
	ASRApp_Decode = wxID_HIGHEST + 3,
	ASRApp_OpenFile = wxID_HIGHEST + 4,
	ASRApp_AudioParams = wxID_HIGHEST + 5,
	ASRApp_Hide = wxID_HIGHEST + 6,
	ASRApp_Play = wxID_HIGHEST + 7,
	ASRApp_Font = wxID_HIGHEST + 8,
	ASRApp_Clear = wxID_HIGHEST + 9,
	ASRApp_Shutdown = wxID_HIGHEST + 10,
	ASRApp_SendKeyStroke = wxID_HIGHEST + 11,
	ASRApp_Help = wxID_HELP,
    LIST_CTRL    = 1000,
	RICHTEXT_CTRL = 1001

};

#endif //__ASRApp__
// A custom status bar which contains controls, icons &c
