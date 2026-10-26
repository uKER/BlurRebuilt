#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Application settings and user preferences.
//
class AppSettings
{
public:
	AppSettings();
	~AppSettings();

	/// Set default settings.
	void SetDefaults();

	// Load
	// Save

	wxString		m_clipBrowserRootDir;
};


extern AppSettings	g_AppSettings;

#endif

