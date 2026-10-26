#ifndef AFTK_PROGRESS_H
#define AFTK_PROGRESS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	///Interface for progress notifications.
	class I_Progress
	{
	public:
		///\brief This function is invoked whenever a progress bar percentage needs updating.
		///
		///Note that returning false will eventually abort the current operation. 
		///
		///@param percent The current ammount of progress.
		virtual bool OnProgress(F32 percent) = 0;
		
		///\brief This function is invoked whenever progress bar text needs updating.
		///
		///@param text Text to display.
		virtual void OnPrintProgress(const std::string& text) = 0;
	};
}

#endif
