#pragma once
#include "wx/thread.h"
#include "wx/string.h"
#include "BaseFrame.h"
#include "wx/socket.h"
#include "Converter.h"

extern "C" void srch_output_cleanUp();
extern "C" char* srch_output_result();

class DecoderThread :
	public wxThread
{
public:
	DecoderThread(void);
	DecoderThread(BaseFrame *frm);
    virtual void *Entry();
	virtual ~DecoderThread(void);
	bool debug;
	int Initialize();
	void Decode();
	void Start();
	void Stop();
	void ReStart();
	bool Stopped();
	void Pause();
	void Resume();
	bool IsReady();
	void SetFlag(bool flag);
	void ResetTimer();
	void StopTimer();
private:
    FILE*				fid;
	BaseFrame*			frame;
	bool SendFile(wxString file);
	void WriteText(const wxString& msg);
	void WriteText(const wxString& msg, int errorNum);
	void ProcessQueue();
	void GetResult();
	wxSocketClient		*sock;
	wxIPV4address		addr;
	int SndMsg(const char* msg);
	int SndMsg(const char* msg, int len);
	Converter			cnv;
	wxString GetLastLine();
	bool isReady;
	bool stop;
	bool restart;
	//long timeout;

};
