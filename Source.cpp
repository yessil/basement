#include "AudioThread.h"

AudioThread::AudioThread(void) :wxThread()
{
	//playIt = false;
	//recordIt = false;
	//stop = false;
	//err = 0;

}

AudioThread::AudioThread(BaseFrame *f){

		playIt = false;
		recordIt = false;
		stop = false;
		isWriting = false;
		err = 0;
		frame = f;
	//	in_ad = 0;
	//	sf = NULL;
	//	dump = NULL;
		fileNum = 0;
		nzc = 0;
		timeout = 0;
		isZeroSet = false;
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

void *AudioThread::Entry(){

	//while(!TestDestroy()){
	//	Sleep(10);
	//	timeout++;
	//	silence = frame->ZERO_MARGIN;
	//	sil_cutoff = frame->SILENCE_CUTOFF;
	//	speech = frame->SPEECH_LENGTH;
	//	debug = frame->debug;


	//	if (stop ){
	//		StopStream();
	//		Exit();
	//		continue;
	//	}

	//	if (recordIt){
	//		if (in_ad == 0 )
	//			StartStream();
	//		else 
	//			Record();
	//	} else {
	//		SetValue(0);
	//		//SaveFile();
	//		if (in_ad!=0 && in_ad->recording)
	//			ad_stop_rec(in_ad);
	//		//StopStream();
	//	}
	//}
	return NULL;
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

	//return err;
	return 0;
}

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

void AudioThread::Stop(){

	stop = true;
}

void AudioThread::ToggleRecord(){

	recordIt = !recordIt;
}
