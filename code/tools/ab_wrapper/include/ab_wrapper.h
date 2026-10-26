#ifndef AB_WRAPPER_H
#define AB_WRAPPER_H

#pragma once

#include <btl/containers/list.h>
#include <btl/containers/vector.h>
#include <btl/utility/pair.h>
#include <btl/utility/string.h>

namespace Debug
{
	class Logger;
}

namespace Vcs
{
	class C_Ab
	{
	public:
		struct C_Config
		{
			typedef Btl::Pair<Btl::String, Btl::String> FolderMapping;
			Btl::Vector< FolderMapping > mappings;
			Btl::String scriptPath, user, password;
		};

		C_Ab(Debug::Logger& logger,
			bool (*pAbScriptExecCallback)(const char* cmdline, const char* cmdParams, const char* workingPath));
		virtual ~C_Ab();

		bool Initialise(const char* config);

		const C_Config& GetConfig() const;

		bool Update(const Btl::Vector<Btl::String>& filePaths) const;
		
		bool Sync(const Btl::Vector<Btl::String>& filePaths) const;

	private:
		bool PerformAction(const Btl::Vector<Btl::String>& filePaths, bool sync) const;

		bool LaunchAlienbrainScript(
			const C_Config::FolderMapping& mapping,
			const Btl::Vector<Btl::String>& foldersToGrab, 
			bool sync) const;

		Debug::Logger&	m_logger;
		bool			(*m_pAbScriptExecCallback)(const char* cmd, const char* cmdParams, const char* workingPath);
		C_Config		m_config;
	};


}

#endif
