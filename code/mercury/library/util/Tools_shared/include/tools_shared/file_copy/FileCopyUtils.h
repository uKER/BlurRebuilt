#ifndef FILE_COPY_UTILS_H
#define FILE_COPY_UTILS_H

#include <MMSystem.h>
#include "system/serialise/C_FileSystem.h"
#include "system/misc/StringUtil.h"

#include <iostream>

#if defined(FILE_COPY_XDEVKIT)
#include <xdevkit.h>
#endif

namespace Tools
{
	namespace FileCopy
	{
		enum E_TimeCompare
		{
			E_TimeCompare_Unknown			=-1,
			E_TimeCompare_Same				= 0,
			E_TimeCompare_SourceNewer		= 0,
			E_TimeCompare_DestinationNewer	= 2
		};


		class FileCopyUtils
		{
		public:
			static Util::C_String GetExecutableDirectory();

			static Util::C_String GetExecutableName();

			static Util::C_String GetCurrentDirectory();

			static bool MakeWritable(const Util::C_String &fileName);

			static bool CreateOutputPath(const Util::C_String &filePath);

			static bool CopyFile( const Util::C_String &src, const Util::C_String &dst, int numRetries=6, int waitTimeMs=10000 );

			static E_TimeCompare ShouldCopyFiles(const Util::C_String &src, const Util::C_String &dst);
		};



		class FileCopyUtils360
		{
		public:
			static FileCopyUtils360& Instance()
			{
				static FileCopyUtils360 instance;
				return instance;
			};

		public:

		#if defined(FILE_COPY_XDEVKIT)
			inline bool IsConnected(Util::C_String &console) const
			{
				return (m_ConsoleName == console);
			};
		#endif

			const Util::C_String FormatFileString(const Util::C_String src);

			bool CreateOutputPath(const Util::C_String& filePath, const Util::C_String *console = 0);

			bool CopyFile(const Util::C_String &src, const Util::C_String &dst, const Util::C_String *console = 0);

			E_TimeCompare ShouldCopyFiles(const Util::C_String &src, const Util::C_String &dst, const Util::C_String *console = 0);

		private:
			FileCopyUtils360();

			~FileCopyUtils360();

		#if defined(FILE_COPY_XDEVKIT)

			IXboxConsole* GetConsoleInterface(const Util::C_String *console = 0);

			IXboxManager	*m_pXboxManager;
			IXboxConsole	*m_pXboxConsole;

			Util::C_String	m_ConsoleName;
			//Ut::UtVector< Ut::UtDynamicVector<Util::C_String> > m_Consoles;
		#endif
		};
	}
}

#endif
