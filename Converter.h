#pragma once
#include "wave2feat.h"
#include "wx/string.h"
class Converter
{
public:
	Converter(void);
	~Converter(void);
	int Convert(char* wfile, char* ffile);
	int Convert(wxString wfile);

private:
	char* argv[37];
};
