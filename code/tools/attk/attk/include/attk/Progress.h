#ifndef ATTK_PROGRESS_H
#define ATTK_PROGRESS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_Progress
	{
	public:
		FORCE_INLINE void SetMin(U32 min);

		FORCE_INLINE void SetMax(U32 max);

		///\brief Increments our progress (prefix form).
		///
		///@return The current progress value.
		FORCE_INLINE const U32* operator++();

		///\brief Increments our progress (postfix form).
		///
		///@param dummy Dummy argument required for postfix form of increment operator.
		///@return The current progress value.
		FORCE_INLINE const U32* operator++(int dummy);

		///\brief This function is invoked whenever a progress bar percentage needs updating.
		///
		///Note that returning false will eventually abort the current operation. 
		///
		///@param progress The units of work completed.
		virtual bool SetProgress(U32 progress) = 0;
		
		///\brief This function is invoked whenever progress bar text needs updating.
		///
		///@param text Text to display.
		virtual void SetText(const std::string& text) = 0;

		FORCE_INLINE U32 GetProgress() const;

		FORCE_INLINE F32 GetProgressPercent() const;
	
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		U32 m_min;
		U32 m_max;
		U32 m_progress;
	};
#endif

	FORCE_INLINE void C_Progress::SetMin(U32 min)
	{
		m_min = min;
	}

	FORCE_INLINE void C_Progress::SetMax(U32 max)
	{
		m_max = max;
	}

	FORCE_INLINE const U32* C_Progress::operator++()
	{
		return (*this)++;
	}

	FORCE_INLINE const U32* C_Progress::operator++(int dummy)
	{
		m_progress++;
	}

	FORCE_INLINE U32 C_Progress::GetProgress() const
	{
		return m_progress;
	}

	FORCE_INLINE F32 C_Progress::GetProgressPercent() const
	{
		return (F32)m_progress/(F32)(m_max - m_min)*100.f;
	}
}

#endif
