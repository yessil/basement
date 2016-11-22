#pragma once
#ifndef READAUDIOTHREAD_H
#define READAUDIOTHREAD_H

#include "wx/thread.h"
#include "wx/string.h"
#include "portaudio.h"
#include "BaseFrame.h"
#include "wx/dir.h"
#include "wx/textfile.h"
#include "wx/config.h"

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER2 (1024)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (1)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/

/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif


typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
	FILE*		fid;
}
paAudioData;

static bool debug2 = false;


class ReadAudio :
	public wxThread
{
public:
	ReadAudio(void);
	ReadAudio(BaseFrame *frm);
    virtual void *Entry();
	virtual ~ReadAudio(void);
	wxString fileName;
	bool playIt;
	bool recordIt;
	int Initialize();
    PaError            err;
	void Stop();

private:
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    SAMPLE              max, val;
    double              average;
	BaseFrame*			frame;
	volatile bool		stop;
	int					fileNum;

	void WriteText(const wxString& msg);
	void WriteText(const wxString& msg, int errorNum);
	void ReadFile();
public:
	void StopStream();
	void StartStream(bool);

};
#endif// AUDIOTHREAD_H
