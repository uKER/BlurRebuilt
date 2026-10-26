#ifndef TU_LOGFILE_H
#define TU_LOGFILE_H

#include <string>
#include <fstream>

namespace Tu
{
	class LogFile
	{
	public:
		LogFile(const std::string& fileName, bool overwrite=true)
		{
			m_fileName = fileName;
			if (m_fileName != "") 
			{
				if (overwrite)
				{
					m_file.open(fileName.c_str());
				}
				else
				{
					m_file.open(fileName.c_str(), std::ios::app);
				}
			}
		}


		~LogFile()
		{
			if (m_file.is_open()) 
			{
				m_file.close();
			}
		}


		void AddText(const std::string& text)
		{
			if (m_file.is_open()) 
			{
				m_file << text << std::flush;
			}
		}


		const std::string& GetFileName() const
		{
			return m_fileName;
		}

	private:
		std::ofstream m_file;
		std::string m_fileName;
	};
}

#endif
