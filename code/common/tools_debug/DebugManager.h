#ifndef TU_DEBUG_MANAGER_H
#define TU_DEBUG_MANAGER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "LogFile.h"
#ifdef _WIN32
#include <windows.h>
#endif

namespace Tu
{
	class DebugManager
	{
	public:
		typedef void(*DebugTextCallback)(const std::string& text);

		DebugManager()
		{
			m_referenceCount = 0;
			m_debugEnabledFlag = false;
			m_logFileEnabledFlag = false;
			m_callbackEnabledFlag = false;
		}

		~DebugManager()
		{
		}

		void Create(const std::string& fileName = std::string(""), bool overwrite=true)
		{
			if (m_referenceCount == 0)
			{
				m_pCallbackFunc = 0;
				m_pLogFile = 0;

				if (fileName == "") 
				{
					m_logFileEnabledFlag = false;
					m_pLogFile = 0;
				} 
				else 
				{
					m_logFileEnabledFlag = true;
					m_pLogFile = new LogFile(fileName, overwrite);
				}
				m_pLastString = new std::string;
			}
			m_referenceCount++;
		}

		void Destroy()
		{
			if (m_referenceCount < 2)
			{
				if (m_pLogFile != NULL)
				{
					delete m_pLogFile;
					m_pLogFile = 0;
					delete m_pLastString;
					m_pLastString = 0;
				}

				m_referenceCount = 0;
			}
			else
			{
				m_referenceCount--;
			}
		}

		void Clear()
		{
			if (m_pLastString)
			{
				m_pLastString->clear();
			}
			if (m_consoleEnabledFlag)
			{
				std::cout << std::flush;
			}
		}

		void SetConsoleEnabledFlag(bool flag)
		{
			m_consoleEnabledFlag = flag;
		}


		void SetLogFileEnabledFlag(bool flag)
		{
			m_logFileEnabledFlag = flag;
		}

		void SetDebugEnabledFlag(bool flag)
		{
			m_debugEnabledFlag = flag;
		}

		void SetCallbackEnabledFlag(bool flag)
		{
			m_callbackEnabledFlag = flag;
		}

		void SetCallback(void (*pLogCallbackFunc)(const std::string& text))
		{
			m_pCallbackFunc = pLogCallbackFunc;
		}

		DebugTextCallback GetCallback()
		{
			return m_pCallbackFunc;
		}

		DebugManager& operator<<(const char* string) 
		{
			AddText(string);
			return *this;
		}

		DebugManager& operator<<(const std::string& string) 
		{
			AddText(string);
			return *this;
		}

		DebugManager& operator<<(const U8 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const U16 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const U32 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const S8 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const S16 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const S32 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const F32 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

		DebugManager& operator<<(const F64 var) 
		{
			std::ostringstream stream;
			stream << var;
			AddText(stream.str());

			return *this;
		}

#ifdef __MATHS_H__
		template <class T, int N>
		DebugManager& operator <<(const UtMaths::BaseVector<T, N>& value)
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << " ";
				}
			}

			return*	this;
		}

		template <class T, int N>
		DebugManager& operator <<(const UtMaths::BasicMatrix<T, N>& value) 
		{
			for (U32 i = 0; i < value.SIZE; i++)
			{
				*this << "Row ";
				*this << i;
				*this << " = [";
				*this << value[i];
				if (i < value.SIZE-1)
				{
					*this << "]\n";
				}
			}
			*this << "]\n";

			return*	this;
		}
#endif

		const std::string GetText() const
		{
			if (m_pLastString)
			{
				return *m_pLastString;
			}
			else
			{
				return std::string();
			}
		}

	private:
		void AddText(const std::string& text)
		{
			if (m_pLastString)
			{
				*m_pLastString += text;
			}
			if (m_consoleEnabledFlag == true) 
			{
				AddConsoleText(text);
			}
			if (m_logFileEnabledFlag == true)
			{
				if (m_pLogFile != 0)
				{
					m_pLogFile->AddText(text);
				}
			}
			if (m_debugEnabledFlag == true)
			{
				AddDebugText(text);
			}
			if (m_callbackEnabledFlag == true)
			{
				if (m_pCallbackFunc != 0)
				{
					m_pCallbackFunc(text);
				}
			}
		}

		void AddConsoleText(const std::string& text)
		{
			if (m_consoleEnabledFlag == true) 
			{
				std::cout << text << std::flush;
			}
		}

		void AddLogFileText(const std::string& text)
		{
			if (m_logFileEnabledFlag == true) 
			{
				if (m_pLogFile != 0) 
				{
					m_pLogFile->AddText(text);
				}
			}
		}

		void AddDebugText(const std::string& text)
		{
			if (m_debugEnabledFlag == true)
			{
				#ifdef _WIN32
				OutputDebugString(text.c_str());
				#endif
			}
		}

		LogFile*		m_pLogFile;
		bool			m_logFileEnabledFlag;
		bool			m_consoleEnabledFlag;
		bool			m_callbackEnabledFlag;
		bool			m_debugEnabledFlag;
		void			(*m_pCallbackFunc)(const std::string& text);
		std::string*	m_pLastString;
		U32				m_referenceCount;
	};
}

extern void Log(char *sz, ...);

#endif
