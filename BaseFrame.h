#pragma once
#include "wx/frame.h"
#include "wx/log.h"
#define FILENAME_LENGHT 30

class BaseFrame :
	public wxFrame
{
public:
	BaseFrame(void);
	BaseFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
	virtual void WriteText(const wxString& text);
	virtual void SetStatusbarText(wxString msg);
	virtual void SetValue(int value);
	virtual void SetFlag(bool flag);
	virtual void ResetTimer();
	virtual void StopTimer();
	~BaseFrame(void);
	bool stop;
	virtual bool IsStop();
	int ZERO_MARGIN;
	int SILENCE_CUTOFF;
	int SPEECH_LENGTH;
	wxString SERVER;
	int PORT;
	bool debug;
	virtual void Exec(wxString cmd);
	wxLogChain *logchain;
	FILE *log;

};
