#ifndef MERCURY_SHIM_OPTIONS_H
#define MERCURY_SHIM_OPTIONS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDom;
}

namespace Tools
{
	namespace Shim
	{
		typedef Ut::UtVector<Ut::ArrayVector<FileCopy::C_PlatformCopyLocations, Util::E_Platform_Count> >
			ShimCopyLocations;

		struct C_SvnOtions
		{
			Sys::C_FilePath svnGetLatestCmd;
			Btl::String user;
			Btl::String pass;
		};

		struct C_SvnFolder
		{
			Sys::C_FilePath localPath;
			Btl::String remotePath;
		};

		class C_Options
		{
		public:
			virtual ~C_Options() {}

			virtual bool ParseXml(const Util::C_String& xmlFilePath, const Sys::C_XmlDom& dom);

			const Util::C_String& GetExportName() const;
			const Util::C_String& GetXmlFilePath() const;

			const Util::C_String& GetRootToolsPath() const;
			const Util::C_String& GetServerRootToolsPath() const;
			const Util::C_String& GetRootProjectPath() const;

			//Raw asset location.
			const Util::C_String& GetRootAssetPath() const;
			//Temporary export location.
			const Util::C_String& GetRootTempPath() const;
			//Output location
			const Util::C_String& GetRootWorkingPath() const;

			//Bitfield describing export settings.
			const U32 GetExportInfo() const;
			//The name of the PC which originally launched our job.
			const Util::C_String& GetLaunchComputerName() const;
			//Platform specific locations for working and target (game) folders.
			const ShimCopyLocations& GetCopyLocations() const;

			//Tool paths
			const Util::C_String& GetBootstrapFilePath() const;
			const Util::C_String& GetRobocopyFilePath() const;
			const Util::C_String& GetUberCopyFilePath() const;
			const Util::C_String& GetAftkConvFilePath() const;
			const C_SvnFolder& GetArtToolsSvn() const;

			const C_SvnOtions& GetSvnOptions() const;

		protected:
			bool SetLaunchComputerName();

			Util::C_String m_shimExportName;
			Util::C_String m_shimXmlFilePath;

			Util::C_String m_shimRootToolsPath;
			Util::C_String m_shimServerRootToolsPath;

			Util::C_String m_shimRootProjectPath;

			Util::C_String m_shimRootAssetPath;
			Util::C_String m_shimRootTempPath;
			Util::C_String m_shimRootWorkingPath;

			U32 m_shimExportInfo;
			Util::C_String m_shimLaunchComputerName;
			
			Util::C_String m_shimBootstrapFilePath;
			Util::C_String m_shimRobocopyFilePath;
			Util::C_String m_shimUberCopyFilePath;
			Util::C_String m_shimAftkConvFilePath;
			C_SvnFolder		m_shimSvnArtToolsSvn;

			Util::C_String m_shimAlienbrainScript;
			Util::C_String m_shimAlienbrainDefaultUser;
			Util::C_String m_shimAlienbrainDefaultPass;
			Ut::Vector<Ut::Pair<Util::C_String, Util::C_String> > m_shimAlienbrainMappings;

			ShimCopyLocations m_shimCopyLocations;
			C_SvnOtions m_svnOptions;
		};
	}
}

#endif

