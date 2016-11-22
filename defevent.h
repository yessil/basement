#pragma once
#ifndef DEFEVENT_H
	#define DEFEVENT_H
#endif

#include "wx/wxprec.h"

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif


//extern const wxEventType wxEVT_NEW_TEXT;
//const wxEventType wxEVT_NEW_TEXT = wxNewEventType();
BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_LOCAL_EVENT_TYPE(wxEVT_ProcessNewText, 7777)
END_DECLARE_EVENT_TYPES()

DEFINE_EVENT_TYPE(wxEVT_ProcessNewText)
