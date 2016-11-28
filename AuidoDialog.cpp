#include "AudioDialog.h"
#include <wx/sizer.h>


BEGIN_EVENT_TABLE(AudioDialog, wxDialog)
    EVT_BUTTON(wxID_OK, AudioDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, AudioDialog::OnCancel)
END_EVENT_TABLE()

IMPLEMENT_CLASS(AudioDialog, wxDialog)

#include "AudioThread.h"

AudioDialog::AudioDialog(): wxDialog(){

}
AudioDialog::AudioDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
: wxDialog(parent, id, title, pos, size, style)
{

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridSizer* gSizer1;
	gSizer1 = new wxGridSizer( 2 );
	
	m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Порог тишины"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText1->Wrap( -1 );
	gSizer1->Add( m_staticText1, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	zeroMarginCtl = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gSizer1->Add( zeroMarginCtl, 0, wxALL, 5 );
	
	m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Длина паузы"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	gSizer1->Add( m_staticText2, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	zeroCutOff = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gSizer1->Add( zeroCutOff, 0, wxALL, 5 );

	m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Длина речи"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1);
	gSizer1->Add( m_staticText3, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	speechMargin = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gSizer1->Add( speechMargin, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Сервер"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap(-1);
	gSizer1->Add( m_staticText4, 0, wxALL|wxALIGN_RIGHT, 5 );


	txtServer = new wxTextCtrl( this, wxID_ANY, wxT("localhost"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer1->Add( txtServer, 0, wxALL, 5 );

	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Режим отладки"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap(-1);
	gSizer1->Add( m_staticText5, 0, wxALL|wxALIGN_RIGHT, 5 );

	debugChk = new wxCheckBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize );
	gSizer1->Add( debugChk, 0, wxALL, 5 );

	wxSizer *buttonSizer = CreateButtonSizer(wxOK);// | wxCANCEL);
    if ( buttonSizer )
    {
		gSizer1->Add(buttonSizer, wxSizerFlags().Align(wxALIGN_RIGHT));//DoubleBorder());
    }
	buttonSizer = CreateButtonSizer(wxCANCEL);
	if (buttonSizer)
	{
		gSizer1->Add(buttonSizer, wxSizerFlags().Align(wxALIGN_LEFT));//DoubleBorder());
	}

	zeroMarginCtl->SetRange(0,10000);
	zeroCutOff->SetRange(0, 50000);
	speechMargin->SetRange(0, 50000);

	this->SetSizer( gSizer1 );
	this->Layout();
}

void AudioDialog::SetValue(const wxString& type, int value){

	if (type.Contains(ZERO_MARGIN_KEY)){
		zeroMarginCtl->SetValue(value);
		return;
	}
	if (type.Contains(SILENCE_CUTOFF_KEY)){
		zeroCutOff->SetValue(value);
		return;
	}
	if (type.Contains(SPEECH_LENGTH_KEY)){
		speechMargin->SetValue(value);
		return;
	}

	debugChk->SetValue((bool)value);

}

void AudioDialog::SetStrValue(const wxString& type, wxString value){

	if (type.Contains(SERVER_KEY)){
		txtServer->SetValue(value);
		return;
	}
}

int AudioDialog::GetValue(const wxString& type){

	if (type.Contains(ZERO_MARGIN_KEY))
		return  zeroMarginCtl->GetValue();
	if (type.Contains(SILENCE_CUTOFF_KEY))
		return zeroCutOff->GetValue();
	if (type.Contains(SPEECH_LENGTH_KEY))
		return speechMargin->GetValue();

	return debugChk->IsChecked();
}

wxString AudioDialog::GetStrValue(const wxString& type){

	if (type.Contains(SERVER_KEY))
		return txtServer->GetValue();

	return _("localhost");
}


void AudioDialog::OnOK(wxCommandEvent& WXUNUSED(event)){

	EndModal(wxID_OK);
}
void AudioDialog::OnCancel(wxCommandEvent& WXUNUSED(event)){

	EndModal(wxID_CANCEL);
}