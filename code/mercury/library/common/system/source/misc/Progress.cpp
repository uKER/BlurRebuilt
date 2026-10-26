#include "SystemPch.h"
#include <core/Core.h>
#include <core/Containers.h>
#include <system/Misc/Progress.h>
#include <core/Threads.h>
#include <system/network.h>

namespace Progress
{

I_ProgressBar::I_ProgressBar( I_ProgressBar* pParent ) :
	m_pParent( pParent )
{
}

I_ProgressBar::~I_ProgressBar()
{
}

/// -- simple progress bar

/// constructor
UpdateSimple::UpdateSimple( I_ProgressBar* pParent, const Util::C_String &description) :
	I_ProgressBar( pParent ),
	m_description( description )
{
	SetProgress(0.0f);
}

/// destructor
UpdateSimple::~UpdateSimple()
{
	SetProgress(100.0f);
}

/// set the progress as a percentage
bool UpdateSimple::SetProgress(float percent)
{
	if(m_pParent)
	{
		return m_pParent->ProgressNotify(0, percent, m_description);
	}
	else
		return true;
}


/// -- stepped progress bar

/// constructor
UpdateStepped::UpdateStepped( I_ProgressBar* pParent, const S32 noSteps, const Util::C_String &description ) :
	I_ProgressBar( pParent ),
    m_NoSteps( noSteps ),
	m_description( description ),
    m_CurrProgress( 0 ),
    m_CurrStepProgress( 0 )
{
}

/// desctructor
UpdateStepped::~UpdateStepped()
{
	if(m_pParent)
	{
		m_pParent->ProgressNotify( 0, 100.0f, m_description );
	}
}

/// advance the progress by one step
bool UpdateStepped::Step()
{
	m_CurrStepProgress = 0.0f;
	m_CurrProgress++;
	if( m_CurrProgress > m_NoSteps)
		m_CurrProgress = m_NoSteps;

	// notify the parent of the progress update
    if(m_pParent)
    {
		F32 progress = ((F32)m_CurrProgress * 100.0f / (F32)m_NoSteps) + (m_CurrStepProgress / (F32)m_NoSteps);
		if(progress > 100.0f)
			progress = 100.0f;
		return m_pParent->ProgressNotify( 0, progress, m_description );
    }
	else
		return true;
}

// recieves sub-step notify messages
bool UpdateStepped::ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description )
{
	if(iHierarchyDepth == 0)
	{
		m_CurrStepProgress = percent / 100.0f;

		// notify the parent of the progress update
		if(m_pParent)
		{
			F32 progress = (100.f * (m_CurrProgress + m_CurrStepProgress)) / static_cast<F32>(m_NoSteps);
			if(progress > 100.0f)
				progress = 100.0f;
			// send update for parent progress bar
			return m_pParent->ProgressNotify( 0, progress, description );
		}
		else
			return true;
	}
	else
	{
		if(m_pParent)
		{
			// send update for child progress bar
			//m_pParent->ProgressNotify( iHierarchyDepth+1, percent, m_description + "." + description );
			return m_pParent->ProgressNotify( iHierarchyDepth+1, percent, description );
		}
		else
			return true;
	}
}


/// -- multi progress bar

/// constructor
UpdateMulti::UpdateMulti( I_ProgressBar* pParent, const Util::C_String &description ) :
	I_ProgressBar( pParent ),
	m_description( description )
{
}

/// destructor
UpdateMulti::~UpdateMulti()
{
	if(m_pParent)
	{
		m_pParent->ProgressNotify( 0, 100.0f, m_description );
	}
}

/// retrieve a pointer to the given child progress bar
I_ProgressBar* UpdateMulti::operator [] ( const U32 index )
{
	return m_Children[index];
}

/// add a child progress bar
I_ProgressBar *UpdateMulti::AddChild( const F32 relativeTime )
{
	Item *p = new Item(this);
	m_Children.PushBack( p );
	p->m_RelativeTime = relativeTime;
	p->m_ProgressPercent = 0;
	return p;
}

UpdateMulti::Item::Item(UpdateMulti* pMulti) :
	I_ProgressBar(pMulti)
{
}

/// receive progress update from child progress bar
bool UpdateMulti::Item::ProgressNotify(int i, const F32 percent, const Util::C_String &description)
{
	m_ProgressPercent = percent;

	return ((UpdateMulti*)m_pParent)->Update(i, percent, description);
}

/// one of the children called this to update our progress
/// calculate new progress and send to m_pParent
bool UpdateMulti::Update(int iChildHierarchyDepth, float fChildPercentage, const Util::C_String &description)
{
	F32 progress = 0;
	F32 totalRelativeTime = 0;

	for (int i=0; i<m_Children.Size(); i++)
	{		 
		progress += m_Children[i]->m_ProgressPercent * m_Children[i]->m_RelativeTime;
		totalRelativeTime += m_Children[i]->m_RelativeTime;
	}

	progress = progress / totalRelativeTime;

	bool bRtn = true;

    if (m_pParent)
    {
		if(iChildHierarchyDepth == 0)
		{
			/// send update for parent progress bar
			if (!m_pParent->ProgressNotify( 0, progress, m_description))
				bRtn = false;
		}

		/// send update for child progress bar
		//m_pParent->ProgressNotify( iChildHierarchyDepth+1, fChildPercentage, m_description + "." + description);
		if (!m_pParent->ProgressNotify( iChildHierarchyDepth+1, fChildPercentage, description))
			bRtn = false;

		return bRtn;
    }
	else
		return true;
}

/// a progress bar that send progress updates to the parent process
/// for example for lux.exe to send it's progress to the SceneConverter

/// constructor
UpdateNetwork::UpdateNetwork()
	: m_iLastSent(0)
	, m_pSocket(NULL)
	, I_ProgressBar(NULL)
{
}

/// destructor
UpdateNetwork::~UpdateNetwork()
{
	if (m_pSocket)
		delete m_pSocket;
}

/// connect to a progress server
bool UpdateNetwork::Connect()
{
	int iPort = 8004;

	// try to connect to the specified address + port
	 m_pSocket = new Network::TCPSocket();

	if (!m_pSocket->Connect( Network::Address( "127.0.0.1" ), iPort ))
	{
		delete m_pSocket;
		m_pSocket = NULL;
		return true;
	}

	return true;
}

// recieves sub-step notify messages
bool UpdateNetwork::ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description )
{
	// send the progress across the network
	if (m_pSocket && iHierarchyDepth==0)
	{
		U8 x = (U8)(percent*(255.f/100.f));
		if (x != m_iLastSent)
		{
			m_iLastSent = x;
			m_pSocket->Send(&x, 1);
		}
		return true;
	}

	return true;
}

}; // namespace Progress
