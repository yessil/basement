#include "wx/dialog.h"
#include "wx/defs.h"
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include "wx/checkbox.h"
#include <wx/textctrl.h>

class AudioDialog : public wxDialog
{
public:
	AudioDialog(void);
	AudioDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Параметры"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 221,48 ), long style = wxDEFAULT_DIALOG_STYLE );
	void SetValue(const wxString& type = _T("DEBUG"), int value = (int) false);
	int GetValue(const wxString& type = _T("DEBUG"));
	void SetStrValue(const wxString& type = _T("DEBUG"), wxString value = _T(""));
	wxString GetStrValue(const wxString& type = _T("DEBUG"));
	void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

private:

	wxSpinCtrl *zeroMarginCtl;
	wxSpinCtrl *zeroCutOff;
	wxSpinCtrl *speechMargin;
	wxCheckBox *debugChk;
	wxTextCtrl* txtServer;
	wxStaticText* m_staticText1;
	wxStaticText* m_staticText2;
	wxStaticText* m_staticText3;
	wxStaticText* m_staticText4;
	wxStaticText* m_staticText5;

	DECLARE_EVENT_TABLE()
    DECLARE_DYNAMIC_CLASS(AudioDialog)
    DECLARE_NO_COPY_CLASS(AudioDialog)
};
