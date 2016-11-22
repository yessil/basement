#ifndef AUDIOTHREAD_H
#include "AudioThread.h"
#endif
//#include "sndfile.h"
//#include "BaseFrame.h"
//#include "wx/dir.h"
//#include "err.h"


/*******************************************************************/

AudioThread::AudioThread(void):wxThread()
{
	//playIt = false;
	//recordIt = false;
	//stop = false;
	//err = 0;

}

AudioThread::AudioThread(BaseFrame *f){

//	playIt = false;
//	recordIt = false;
//	stop = false;
//	isWriting = false;
//	err = 0;
//	frame = f;
//	in_ad = 0;
////	sf = NULL;
////	dump = NULL;
//	fileNum = 0;
//	nzc = 0;
//	timeout = 0;
//	isZeroSet = false;
}

AudioThread::~AudioThread(void)
{
	//free(sf);
	////free(in_ad);
	//if (dump!=NULL){
	//	fclose(dump);
	//	dump = NULL;
	//}

}

void AudioThread::WriteText(const wxString& text){

		//wxString msg;
		//// before doing any GUI calls we must ensure that this thread is the only
		//// one doing it!
		//wxMutexGuiEnter();
		//msg << text;
		//((BaseFrame*)frame)->WriteText(msg);
		//wxMutexGuiLeave();
}

void AudioThread::SetValue(int value){

		//wxMutexGuiEnter();
		//if (stop)
		//	return;
		//((BaseFrame*)frame)->SetValue(value);
		//wxMutexGuiLeave();
}


void AudioThread::WriteText(const wxString& text, int errNum){

	//wxString tmp = text;
	//tmp.Printf(_T("\nError: %d - %s"), errNum);
	//WriteText(tmp);
	//err = 0; // reset error
}

int AudioThread::Initialize(){


	//silence = frame->ZERO_MARGIN; // Порог тишины
	//sil_cutoff = frame->SILENCE_CUTOFF; // Длина паузы
	//speech = frame->SPEECH_LENGTH; // Длина речи
	//zc = 0;
	//nzc = 0;
	//ResetFiles();
	//fileNum = 0;
	//timeout = 0;

	return err;
}

void *AudioThread::Entry(){

	while(!TestDestroy()){
		Sleep(10);
		timeout++;
		silence = frame->ZERO_MARGIN;
		sil_cutoff = frame->SILENCE_CUTOFF;
		speech = frame->SPEECH_LENGTH;
		debug = frame->debug;


		if (stop ){
			StopStream();
			Exit();
			continue;
		}

		if (recordIt){
			if (in_ad == 0 )
				StartStream();
			else 
				Record();
		} else {
			SetValue(0);
			//SaveFile();
			if (in_ad!=0 && in_ad->recording)
				ad_stop_rec(in_ad);
			//StopStream();
		}
	}
	return NULL;
}

void AudioThread::StartStream(){

	//if (in_ad == 0)
	//	if ((in_ad = ad_open_sps(SAMPLE_RATE)) == NULL) {
	//		frame->WriteText(_T("Failed to open audio input device\n"));
	//		return;
	//	}
	////	if (!in_ad->recording)
	//		ad_start_rec(in_ad);

	//frame->SetStatusbarText(_T("Начало записи..."));

}

void AudioThread::StopStream()
{
	//if (in_ad == 0)
	//	return;
	//ad_stop_rec(in_ad);
 //   ad_close(in_ad);
}

void AudioThread::SaveToFile(uint32 num_frames, int16* frames){

/*
	SF_INFO      info;

	info.samplerate = (unsigned int)(SAMPLE_RATE + 0.5);
	info.frames = num_frames;
	info.channels = NUM_CHANNELS;
	info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	info.sections = 1;
	info.seekable = 0;

	if (debug)
		frame->SetStatusbarText(wxString::Format(_T("zc: %d / nzc: %d "), zc, nzc));
	if ( zc > silence && nzc > speech){

		wxString wfile = wxString::Format(_T("wav\\recorded%03d.wav"), fileNum);

		if (sf == NULL){
			sf = sf_open((char*)wfile.char_str(), SFM_WRITE, &info);
			frame->SetStatusbarText(wfile);
		}

		if( !sf )
		{
			err = -1;
		}
		else
		{
			sf_count_t n = sf_writef_short(sf, (short *)frames, (sf_count_t)num_frames);
			err = sf_close(sf);
		}
		if (zc > sil_cutoff){
			sf = NULL;
			fileNum++;
		}
		zc = 0;
		nzc = 0;

	}
*/
}

void AudioThread::ResetFiles(){

	//wxArrayString *files = new wxArrayString();
	//wxDir::GetAllFiles(_T("."), files, _T("recorded*.*"));
	//int n = (*files).Count();
	//if (dump !=NULL)
	//	fclose(dump);
	//for (int i=0; i < n; i++)
	//	remove((char*)(*files)[i].char_str());
	//delete files;

}
void AudioThread::Stop(){

	stop = true;
}

void AudioThread::ToggleRecord(){

	recordIt = ! recordIt;
}

void AudioThread::Record2(){

//		uint32 num_frames;
//		char fname[20];
//		static int m = 0;
//
//		//if (!in_ad->recording)
//			ad_start_rec(in_ad);
//
//		sprintf(fname, "wav\\recorded");
//		if (dump == NULL)
//			if ((dump = fopen(fname, "wb")) == 0) {
//				E_ERROR("Cannot open dump file %s\n", fname);
//				return;
//			}
//
//		for(int i=0; i<FRAMES_PER_BUFFER; i++)
//			frames[i]=0;
//		num_frames = ad_read(in_ad, frames, FRAMES_PER_BUFFER);
//		zc=0;
//		for (int i=0; i<num_frames; i++){
//			if (abs(frames[i])< silence){
//				zc++;
//			}
//		}
///*		if (zc == num_frames){
//			return;
//		}*/
//		if (m++ == 2){
//			SetValue((abs(frames[0])+abs(frames[1])+abs(frames[2])) / 3); // level indicator on status panel
//			m =0;
//		}
//		if (fwrite(frames, sizeof(int16), num_frames, dump) < num_frames) {
//			E_ERROR("Error writing audio to dump file.\n");
//		}
}

void AudioThread::SaveFile(){

		//char fname[20];
		//char newname[20];
		//int fsize = 0;

		//sprintf(fname, "wav\\recorded");

		//if (dump == NULL)
		//	return;
		//fsize = dump->_cnt;
		//fclose(dump);
		//if (fsize > 0){
		//	strcpy(newname, fname);
		//	rename(fname, strcat(newname, ".raw"));
		//}
		//dump = NULL;
		//ad_stop_rec(in_ad);
}



void AudioThread::Record(){

//	uint32 num_frames;
//	char fname[20];
//	char newname[20];
//	static int k = 0;
//	static int m = 0;
//	int i;
//	//int noiseFrameCount = 0;
//	//if (!in_ad->recording)
//		ad_start_rec(in_ad);
//
//	sprintf(fname, "wav\\recorded%03d", fileNum);
//	if (dump == NULL){
//		if ((dump = fopen(fname, "wb")) == 0) {
//			E_ERROR("Cannot open dump file %s\n", fname);
//			return;
//		}
//	}
//	for (i=0; i < FRAMES_PER_BUFFER; i++){
//		frames[i]=0;
//	}
//	num_frames = ad_read(in_ad, frames, FRAMES_PER_BUFFER);
//    if (num_frames > 0) {
//		if (!isZeroSet){
//			int s = 0;
//			int cnt = 0;
//			for(i = 1000; i<num_frames; i++, cnt++){// skip first 1000 frames -- assuming startup audio signal burst
//				s += abs(frames[i]);
//			}
//			if (cnt>0){
//				silence = s / cnt;
//				frame->ZERO_MARGIN = max(silence, frame->ZERO_MARGIN);
//				isZeroSet = true;
//				if (debug)
//					WriteText(wxString::Format(_("\nSilence: %d frames: %d\n"), silence, num_frames));
//			}
//		}
//		if (m++ == 2){
//			SetValue((abs(frames[0])+abs(frames[1])+abs(frames[2])) / 3); // level indicator on status panel
//			m =0;
//		}
//    /** dump the recorded audio to disk */
//		for (i=0; i<num_frames; i++){
//			if (abs(frames[i])< silence){
//				zc++;
//				//frames[i]=0;
//			}
//			else {
//				zc = 0;
//				nzc++;
//			}
//		}
//		if (debug)
//				frame->SetStatusbarText(wxString::Format(_T("nzc: %d / zc: %d"), nzc, zc));
//		if (nzc > speech)
//			isWriting = true;
//
//		if (zc > sil_cutoff /* && nzc > speech*/){
//			if (debug)
//				frame->SetStatusbarText(wxString::Format(_T("zc: %4d / nzc: %4d / silence: %4d"), zc, nzc, silence));
//			//if (fwrite(frames, sizeof(int16), num_frames, dump) < num_frames) {
//			//	E_ERROR("Error writing audio to dump file.\n");
//			//}
//			int fsize = dump->_cnt;
//			fclose(dump);
//			if (fsize >0){
//				strcpy(newname, fname);
//				rename(fname, strcat(newname, ".raw"));
//				sprintf(fname, "wav\\recorded%03d", ++fileNum);
//				zc = 0;
//				nzc = 0;
//			}
////				E_INFO("new file: %s\n", fname);
//			dump = fopen(fname, "wb");
//			isWriting = false;
//		}
//		if (dump==NULL || ! isWriting)
//			return;
//        if (fwrite(frames, sizeof(int16), num_frames, dump) < num_frames) {
//			E_ERROR("Error writing audio to dump file.\n");
//		} else
//			isWriting = true;
//    }
}
/*
 void AudioThread::RawToWav(char* raw){
 
	SF_INFO      info;
	//info.format = SF_FORMAT_PCM_16;
	SNDFILE *sf, *sfw;
	int16 buf[4096];
	
	memset (&info, 0, sizeof (info)) ;
	wxString wfile = wxString::FromAscii(raw);
	wfile.Append(_T(".wav"));

	sf = sf_open(raw, SFM_READ, &info);
	//const char *s = sf_strerror (NULL);
	info.format = SF_FORMAT_WAV;
	sfw = sf_open((char*)wfile.char_str(), SFM_WRITE, &info);
	while(sf_read_short(sf, (short *)buf, 4096)> 0){
		sf_write_short(sfw, (short *)buf, 4096);
	}
	sf_close(sf);
	sf_close(sfw);

}
*/
 int AudioThread::getTimer(){ return timeout; }
 void AudioThread::ResetTimer(){ 

	 //if (dump!=NULL){
		// SaveFile();
		// return;
		////fclose(dump);
	 //}
	 ////dump = NULL;
	 //ResetFiles();
	 //timeout = 0;
 }