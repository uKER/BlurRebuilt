#ifndef MAYA_IO_PROGRESS_H
#define MAYA_IO_PROGRESS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Progress : public Aftk::C_Progress
	{
	public:
		void SetTitle(const std::string& title)
		{
			m_title = title;
		}

		//Returning true will eventually abort the current operation. For the moment this is however
		//not implemented.
		bool OnProgress(F32 percent)
		{
			GuiSetWorkDone(percent);
			GuiUpdateProgressBar(m_title);
			return false;
		}
		
		//Callback function for printing output to the user while they wait.
		void OnPrintProgress(const std::string& text)
		{
		}
		
		void OnPrintWarning(const std::string& text)
		{
		}
		
		void OnPrintError(const std::string& text)
		{
		}

	private:
		std::string	m_title;
	};
}
