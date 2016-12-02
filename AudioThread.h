#pragma once
#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H
#include "wx/thread.h"
#include "wx/string.h"
#include "BaseFrame.h"
#include "Converter.h"
#include "wx/dir.h"
#include "wx/textfile.h"
#include "wx/config.h"
#include "sndfile.h"
#if defined (__CYGWIN__)
#include <w32api/windows.h>
#include <w32api/mmsystem.h>
#elif (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
#include <windows.h>
#include <mmsystem.h>
#endif

typedef struct {
	HGLOBAL h_whdr;
	LPWAVEHDR p_whdr;
	HGLOBAL h_buf;
	LPSTR p_buf;
} ad_wbuf_t;

#include <ad.h>
#include <err.h>

#define SAMPLE_RATE  (16000)
#define FRAMES_PER_BUFFER (32768)//(32768)
#define NUM_CHANNELS    (1)
#define ZERO_MARGIN_KEY	_T("/Audio/ZERO_MARGIN")
#define	SILENCE_CUTOFF_KEY _T("/Audio/SILENCE_CUTOFF")
#define	SPEECH_LENGTH_KEY _T("/Audio/SPEECH_LENGTH")
#define	SERVER_KEY _T("/Setup/server")
#define TIMEOUT 1000
#define MAX_NOISE_LEVEL_COUNT 40
static int zc = 0;
static int nzc = 0;
static int silence, speech, sil_cutoff;
static bool debug = false;
struct ad_rec_s {
	HWAVEIN h_wavein;	/* "HANDLE" to the audio input device */
	ad_wbuf_t *wi_buf;	/* Recording buffers provided to system */
	int32 n_buf;	/* #Recording buffers provided to system */
	int32 opened;	/* Flag; A/D opened for recording */
	int32 recording;
	int32 curbuf;	/* Current buffer with data for application */
	int32 curoff;	/* Start of data for application in curbuf */
	int32 curlen;	/* #samples of data from curoff in curbuf */
	int32 lastbuf;	/* Last buffer containing data after recording stopped */
	int32 sps;		/* Samples/sec */
	int32 bps;		/* Bytes/sample */
};

class AudioThread:
	public wxThread
{
public:
	AudioThread(void);
	AudioThread(BaseFrame *frm);
    virtual void *Entry();
	virtual ~AudioThread(void);
	//wxString fileName;
	bool playIt;
	bool recordIt;
	int Initialize();
    int            err;
	void Stop();
	//static void RawToWav(char* file);
	int getTimer();
	void ResetTimer();
	void ToggleRecord();

private:
    ad_rec_t			*in_ad;
    int                 timeout;
	int16 frames[FRAMES_PER_BUFFER];
	BaseFrame*			frame;
	Converter			cnv;
	volatile bool		stop;
	int					fileNum;
	FILE				*dump;
	bool OpenFile(char* filename);
	void StartStream();
	void StopStream();
	void SaveToFile(uint32 num_frames, int16* frames);
	void WriteText(const wxString& msg);
	void WriteText(const wxString& msg, int errorNum);
	void ResetFiles();
	void Record();
	void Record2();
	void SaveFile();
	void SetValue(int value);
	int noiseLevelCounter = 0;// counter for computing noise level in first seconds of recording

};
#endif// AUDIOTHREAD_H

