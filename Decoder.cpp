#include "Decoder.h"
#include "BaseFrame.h"
#include "wx/dir.h"
#include "wx/textfile.h"
#include "pio.h"
#include "wx/log.h"
#include "AudioThread.h"
#include <wx/wfstream.h>
#define BUFLEN 500
#define INET
extern const wxEventType wxEVT_ProcessNewText;

DecoderThread::DecoderThread(void):wxThread()
{
	stop = false;
	isReady = true;
	restart = false;
}

DecoderThread::DecoderThread(BaseFrame *f){

	stop = false;
	frame = f;
	isReady = true;
	addr.Hostname(f->SERVER);
	addr.Service(f->PORT);
	sock = new wxSocketClient();
	sock->SetFlags(wxSOCKET_WAITALL);
	sock->SetTimeout(TIMEOUT);
	debug = false;
	restart = false;

}

DecoderThread::~DecoderThread(void)
{
		sock->Close();
		sock->Destroy();
}

bool DecoderThread::IsReady(){ return isReady; }

void DecoderThread::WriteText(const wxString& text){

		wxString msg;
		// before doing any GUI calls we must ensure that this thread is the only
		// one doing it!
		wxMutexGuiEnter();
		msg << text;
		((BaseFrame*)frame)->WriteText(msg);
		wxMutexGuiLeave();
}

void DecoderThread::WriteText(const wxString& text, int errNum){

 	wxString tmp = text;
	tmp.Printf(_T("\nError: %d - %s"), errNum);
	WriteText(tmp);
}

int DecoderThread::Initialize(){

	//if (sock->IsDisconnected()){
	//	sock->Connect(addr, true);
	//	//while ( !sock->WaitOnConnect(-1, 0) );
	//}

	return 0;
}

void *DecoderThread::Entry(){

	while(!TestDestroy()){
		Sleep(10);
		if (stop){
			return NULL;
		}
		debug = frame->debug;
		if (isReady)
			ProcessQueue();
	}
	return NULL;
}

void DecoderThread::Decode(){

}

void DecoderThread::Pause(){
	isReady = false;
}
void DecoderThread::Resume(){
	isReady = true;
}


void DecoderThread::ProcessQueue(){

	isReady = false;
	bool sent = false;
	wxString uttfile =_T("feat\\recorded.mfc");
	char file[FILENAME_LENGHT];
	char bakFile[FILENAME_LENGHT];
	int renamed = -1;

	if (stop){
		int rr= SndMsg("0");
		WriteText(_("\nОстановлен\n"));
		return;
	}
	if (restart){
		SndMsg("0");
		WriteText(_("\nРестарт\n"));
		restart = false;
		isReady = true;
		return;
	}

	wxArrayString files;
	wxDir::GetAllFiles(_T("wav"), &files, _T("*.raw"));
	int n = files.Count();
	int newts;

	if (n>0)
		wxCopyFile(files[n-1], _("wav\\lastfile.rw"), true);

	for (int i=0; i < n; i++){
		wxThread::Sleep(100);
		if (cnv.Convert(files[i]) < 0){
			wxLogMessage(_("Проблема в Convert()"));
			return;
		}
		
#ifdef INET
		sent=SendFile(uttfile);
#else
		newts = stat_mtime(uttfile);
		SndMsg((const char*)wxString::Format(_T("%ld"), newts).ToAscii());
#endif
		do {
			strcpy(bakFile,(char*)(files[i] + _T(".bak")).char_str());
			strcpy(file, (char*)files[i].char_str());
			remove(bakFile);
			renamed = rename(file, bakFile);

		} while(renamed!=0); 
		try {
			SetFlag(false);
			ResetTimer();

			if (sent)
				GetResult();

			SetFlag(true);
			StopTimer();

		} catch ( ... ){
			wxLogMessage(_("Проблема в GetResult()"));
		}
	}
	//for (int i =0; i < files->Count(); i++)
	isReady = true;
	files.Clear();
}

void DecoderThread::Stop(){

	sock->InterruptWait();
	sock->Close();
	stop = true;
}
bool DecoderThread::Stopped(){

	return stop;
}
void DecoderThread::ReStart(){

	if (!restart)
		restart = true;
}

void DecoderThread::Start(){

	stop = false;
	isReady = true;
}


int DecoderThread::SndMsg(const char* msg, int len){

	if (stop)
		Exit();

	int res = 0;
	if (sock->IsDisconnected()){
		sock->Connect(addr, true);
		//sock->WaitOnConnect(60, 0);
	}
	if (sock->IsConnected()){
		res = sock->WriteMsg((void*)msg, len).LastCount(); 
		if (res!=len)
			WriteText(wxString::Format(_("Error! len: %d <> res: %d\n"), len, res));
//		if (debug)
//			WriteText(wxString::Format(_("Sent: len: %d\n"), len));
	} else 
		WriteText(_("<< Не подключен ! >>\n"));
	return res;
}

int DecoderThread::SndMsg(const char* msg){

	int len = strlen(msg);
	return SndMsg(msg, len);
}


#ifdef INET
void DecoderThread::GetResult(){

	if (stop)
		Exit();

	char buf[BUFLEN]; 
	wxCommandEvent eventNewText(wxEVT_ProcessNewText);
	wxString s;
	int len =0;

	if (!sock->IsConnected()){
		sock->Connect(addr, true);
	}

	if (!sock->IsConnected()){
		WriteText(_("<< Не подключен ! >>\n"));
		return;
	}
	if (debug)
		WriteText(_("\nGetting result"));
	while (true){
		sock->ReadMsg(buf, BUFLEN);
		len = sock->LastCount();
		if (debug)
			WriteText(wxString::Format(_T("Read: %5d bytes"), len));
		if (len > 0){
			s.Empty();
			s.Append( frame->SERVER.Lower()== _("localhost") ?  (const wxChar*)buf : wxString::FromUTF8(buf));
			s.Replace(_T("<UNK>"), _T(" "), true);
			s.Replace(_T(" В "), _T(""), true); // TODO
			eventNewText.SetString(s);
			wxPostEvent(frame, eventNewText);
		} else {
			WriteText(_("<< Ошибка приема ! >>\n"));
		}
		if (len<BUFLEN)
			break;
	}
//	sock->Close();
}
#else
void DecoderThread::GetResult(){

	char buf[BUFLEN]; 
	wxCommandEvent eventNewText(wxEVT_ProcessNewText);
	wxString s;
	int lc =0;

	if (sock->IsConnected()){
		if (debug)
			WriteText(_("Waiting for result\n"));
		sock->ReadMsg((void*)buf, BUFLEN); 
		sscanf(buf, "%d", &lc);
		if (debug)
			WriteText(wxString::Format(_("Receiving length: %d \n"), lc));
		memset(buf, 0, lc);
		sock->ReadMsg(buf, lc);
		s.Append((const wxChar*)buf);
		if (debug)
			WriteText(s);
		s.Replace(_T("<UNK>"), _T(" "), true);
		eventNewText.SetString(s);//GetLastLine());//(s);
		wxPostEvent(frame, eventNewText);

	} else {
		WriteText(_("<< Не подключен ! >>\n"));
	}
}
#endif

bool DecoderThread::SendFile(wxString file){

	char buf[BUFLEN];
	int len = 0;
	if (!wxFileExists(file)){
		return false;
	}
	if (stop)
		stop = stop;
	wxFileInputStream fi(file);// = new wxFileInputStream
	if (!fi.IsOk()){
	//	WriteText(_(" Ошибка чтения файла\n"));
		return false;
	}
	if (debug)
		WriteText(_("\nSending file"));
	while(fi.CanRead()){
		fi.Read(buf, BUFLEN);
		len = fi.LastRead();
//		if (debug)
//			WriteText(wxString::Format(_T("%d\n"), len));
		if (SndMsg(buf, len) != len){
			WriteText(_(" Ошибка передачи файла\n"));
			return false;
		}
	}
	return true;
}
void DecoderThread::SetFlag(bool flag){

		wxMutexGuiEnter();
		((BaseFrame*)frame)->SetFlag(flag);
		wxMutexGuiLeave();
}

void DecoderThread::ResetTimer(){

	wxMutexGuiEnter();
	((BaseFrame*)frame)->ResetTimer();
	wxMutexGuiLeave();
}

void DecoderThread::StopTimer(){

	wxMutexGuiEnter();
	((BaseFrame*)frame)->StopTimer();
	wxMutexGuiLeave();

}