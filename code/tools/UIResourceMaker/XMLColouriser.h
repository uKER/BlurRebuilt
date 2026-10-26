#pragma once

class XMLColouriser
{
public:
	void Colourise(CRichEditCtrl *control,int orig_charpos);

private:
	TCHAR buffer[64*1024];
};

extern XMLColouriser gXMLColouriser;