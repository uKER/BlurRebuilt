#ifndef __CORE_SYSTEM_PROGRESS_H__
#define __CORE_SYSTEM_PROGRESS_H__

#pragma once

namespace Network
{
class TCPSocket;
};


namespace Progress
{

/// any progress bar should be derived from I_ProgressBar.
class I_ProgressBar
{
public:
							I_ProgressBar( I_ProgressBar* pParent );

	virtual					~I_ProgressBar();

    /// callback for the child to call on the parent to notify them of a progress update
	/// there is no need to implement this on child progress bars
	virtual bool			ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description ) { assert(0); return true; }

	/// pointer to the parent progress bar that requires notification
	I_ProgressBar			*m_pParent;
};

/// a simple progress bar that has a function to set the progress as percentage
class UpdateSimple : public I_ProgressBar
{
public:
							/// constructor
							UpdateSimple( I_ProgressBar* pParent, const Util::C_String &description);
							/// destructor
							~UpdateSimple();

							/// set the progress as a percentage
	bool					SetProgress(float percent);

private:
	Util::C_String			m_description;

};

/// a stepped progress bar. use this to update a stepped progress indicator, call Progress or
/// cast to progressI_ProgressBar for sub step updates
class UpdateStepped : public I_ProgressBar
{
public:
							/// constructor
							UpdateStepped( I_ProgressBar* pParent, const S32 noSteps, const Util::C_String &description);
							/// destructor
							~UpdateStepped();

	/// advance progress by one step
	bool                    Step();

	// recieves sub-step notify messages
	virtual bool			ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description );

public:
	S32						m_NoSteps;
	S32						m_CurrProgress;
	F32						m_CurrStepProgress;
	Util::C_String			m_description;
};

/// a progress bar that send progress updates to the parent process
/// for example for lux.exe to send it's progress to the SceneConverter
class UpdateNetwork : public I_ProgressBar
{
public:
							/// constructor
							UpdateNetwork();
							/// destructor
							~UpdateNetwork();

	// recieves sub-step notify messages
	virtual bool			ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description );

	// connect to a progress server
	bool					Connect();

public:
	Network::TCPSocket		*m_pSocket;
	U8						m_iLastSent;
};

/// a parent progress bar that tracks progress across a number of child tasks (child progress bars)
///
/// example usage:
///	Progress::UpdateMulti multi(pProgress, "Lighting");
///	Ut::ScopedPointer<Progress::I_ProgressBar> pLoading( multi.AddChild(426.f) );	// loading
///	Ut::ScopedPointer<Progress::I_ProgressBar> pSaving(multi.AddChild(95.f));	// saving
///
///	if (!g_pLuxCore->ReadScene(0, Pointer(pLoading)))
///		return false;
///
///	...
///
///	return true;
///
class UpdateMulti : public I_ProgressBar
{
public:
	/// contains one child progress bar
	class Item : public I_ProgressBar
	{
	public:
		friend class UpdateMulti;

		F32						m_RelativeTime;
		F32						m_ProgressPercent;

		Item( UpdateMulti* pMulti );
								/// receive progress notification from child
		bool					ProgressNotify( int i, const F32 percent, const Util::C_String &description );
	};

							/// constructor
							UpdateMulti( I_ProgressBar* pParent, const Util::C_String &description );
							/// destructor
							~UpdateMulti();

							/// add a child progress bar
	I_ProgressBar*			AddChild( const F32 relativeTime);
							/// retrieve a pointer to the given child progress bar
	I_ProgressBar*			operator [] ( const U32 index );
							/// re-calculate the progress
	bool					Update(int iChildHierarchyDepth, float fChildPercentage, const Util::C_String &description);
							
private:
	// array of children
	Ut::Vector<Item*>		m_Children;
	// description
	Util::C_String			m_description;
};

}; // namespace Progress

#endif
