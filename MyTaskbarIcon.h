#include "wx/taskbar.h"

class MyTaskBarIcon: public wxTaskBarIcon
{
public:
#if defined(__WXCOCOA__)
    MyTaskBarIcon(wxTaskBarIconType iconType = DEFAULT_TYPE)
    :   wxTaskBarIcon(iconType)
#else
    MyTaskBarIcon()
#endif
    { isMicOn = false; }

    void OnLeftButtonDClick(wxTaskBarIconEvent&);
    void OnMenuRestore(wxCommandEvent&);
    void OnMenuExit(wxCommandEvent&);
    void OnMenuMicOff(wxCommandEvent&);
    void OnMenuMicOn(wxCommandEvent&);
       void OnMenuCheckmark(wxCommandEvent&);
       void OnMenuUICheckmark(wxUpdateUIEvent&);
    void OnMenuSub(wxCommandEvent&);
    virtual wxMenu *CreatePopupMenu();

private:
	bool isMicOn;

DECLARE_EVENT_TABLE()
};
