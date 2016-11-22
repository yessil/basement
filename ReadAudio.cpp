#ifndef READAUDIOTHREAD_H
#include "ReadAudio.h"
#endif
//#include "sndfile.h"
#include "BaseFrame.h"
#include "wx/dir.h"
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
paAudioData	gAudata;
int			iBuffMark;
int			iFillLevel;
static bool isReading = false;
static bool isWriting = false;



/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paAudioData *data = (paAudioData*)&gAudata;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
	int finished = paContinue;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
//    printf(" frame %d", *wptr);
	while (isReading)
		wxMilliSleep(10);

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
		if (gAudata.fid ==NULL || feof(gAudata.fid))
			finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

/*******************************************************************/

void ReadAudio::ReadFile(){

	isReading = true;

	if (gAudata.fid ==NULL){
		gAudata.fid = fopen((char*)fileName.ToAscii().data(), "rb"); //TODO ToAscii ??
	}
	if( ferror(gAudata.fid)) {
		err = paBadStreamPtr;
		WriteText(_T("\nНе могу открыть файл ") + fileName);
		return;
	} 

	size_t count = 0;
	for( i=0; i<numSamples; i++ ) gAudata.recordedSamples[i] = 0;
	int res = feof(gAudata.fid);
	if ( res!=EOF){
		count = fread( gAudata.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), gAudata.maxFrameIndex, gAudata.fid );//TODO 10 ?
		gAudata.frameIndex = 0;
	}
	else
		fclose(gAudata.fid);

	isReading = false;
}

ReadAudio::ReadAudio(void):wxThread()
{
	playIt = false;
	recordIt = false;
	stop = false;
	err = paNoError;

}

ReadAudio::ReadAudio(BaseFrame *f){

	playIt = false;
	recordIt = false;
	stop = false;
	err = paNoError;
	frame = f;
}

ReadAudio::~ReadAudio(void)
{
    Pa_Terminate();
	if (gAudata.recordedSamples != NULL)
		free(gAudata.recordedSamples);

}

void ReadAudio::WriteText(const wxString& text){

		wxString msg;
		// before doing any GUI calls we must ensure that this thread is the only
		// one doing it!
		wxMutexGuiEnter();
		msg << text;
		((BaseFrame*)frame)->WriteText(msg);
		wxMutexGuiLeave();
}

void ReadAudio::WriteText(const wxString& text, int errNum){

	wxString tmp = text;
	tmp.Printf(_T("\nError: %d - %s"), errNum);
	WriteText(tmp);
	err = paNoError; // reset error
}

int ReadAudio::Initialize(){


    err = Pa_Initialize();
	if (err != paNoError){
		WriteText(_T("Pa_Initialize"), err);
		return err;
	}
	stream = NULL;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	//WriteText(wxString::Format("\nAudio Initialize: device %d", inputParameters.device));
    inputParameters.channelCount = 1;                    
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	const PaDeviceInfo* dev = Pa_GetDeviceInfo( inputParameters.device );
	if (dev!=NULL)
		inputParameters.suggestedLatency = dev->defaultLowInputLatency;
	else
		return -1;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    outputParameters.channelCount = 1;                   
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

	gAudata.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	iFillLevel = totalFrames - 1; // blind shot
	gAudata.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
	gAudata.recordedSamples = NULL;
	fileNum = 0;

	return err;
}

void *ReadAudio::Entry(){

	while(!TestDestroy()){
		Sleep(10);
		debug2 = frame->debug;
		if (frame->stop){
			StopStream();
			Exit();
			continue;
		}
		if (gAudata.frameIndex > iFillLevel){
			if (playIt)
				ReadFile();
		}
	}
	return NULL;
}

void ReadAudio::StartStream(bool in){

	if (err != paNoError)
		return;

	gAudata.recordedSamples = (SAMPLE *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
    if( gAudata.recordedSamples == NULL )  {
		err = paInsufficientMemory;
		return;
    }

	for( i=0; i<numSamples; i++ ) gAudata.recordedSamples[i] = 0;
    gAudata.frameIndex = 0;

	if (!in) {

		playIt = true;
		recordIt = false;
		ReadFile();
		err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER2,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              playCallback,
              NULL );
	}

    if( stream )
    {
        err = Pa_StartStream( stream );
		if( err != paNoError ) {
			WriteText(_T("Pa_StartStream"), err);
			return;
		}
    }
}

void ReadAudio::StopStream()
{

    if( stream )
    {
        err = Pa_CloseStream( stream );
		if( err != paNoError ) { WriteText(_T("Pa_CloseStream"), err); return;}
    }
	if( gAudata.recordedSamples )       /* Sure it is NULL or valid. */
	{
        free( gAudata.recordedSamples );
	}
	stream = NULL;
	gAudata.recordedSamples = NULL;
	if (gAudata.fid !=NULL){
		fclose(gAudata.fid);
		gAudata.fid = NULL;
	}
	recordIt = false;
}


void ReadAudio::Stop(){

	//stop = true;
}

