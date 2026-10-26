#include "ActorSystemPch.h"

#include <actor_system/ActorCreationList.h>

#include <btl/containers/Algorithm.h>


namespace Actor
{

/**
@class C_ActorCreationStatus
@ingroup actor_group_main
@param maxTasks maximum number of tasks to process in one chunk, or 0 for unlimited
*/

C_ActorCreationStatus::C_ActorCreationStatus(const U32 maxTasks)
	: m_Phase(E_Phase_Begin)
	, m_MaxTasks(maxTasks)
	, m_NumTasks(0)
	, m_StartTask(0)
{
}

void C_ActorCreationStatus::Begin(const E_Phase defaultPhase)
{
	if (m_Phase == E_Phase_Begin)
	{
		m_Phase = defaultPhase;
		m_StartTask = 0;
	}

	m_NumTasks = m_MaxTasks;
}

void C_ActorCreationStatus::Update(const U32 numTasks, U32& o_StartIndex, U32& o_EndIndex)
{
	BIZ_ASSERT(m_Phase != E_Phase_Begin);
	BIZ_ASSERT(m_Phase != E_Phase_End);

	o_StartIndex = m_StartTask;
	o_EndIndex = (m_MaxTasks > 0) ? Maths::Min(m_StartTask + m_NumTasks, numTasks) : numTasks;

	if (o_EndIndex < numTasks)
	{
		m_StartTask = o_EndIndex;
		m_NumTasks = 0;
	}
	else
	{
		m_Phase = static_cast<E_Phase>(m_Phase + 1);
		m_StartTask = 0;
		m_NumTasks = (m_MaxTasks > 0) ? m_NumTasks - (o_EndIndex - o_StartIndex) : 0;
	}
}






/**
@struct C_ActorCreationConfig
@ingroup actor_group_main
*/

C_ActorCreationConfig::C_ActorCreationConfig() :
	m_OnFail(E_OnFail_Continue)
{
	// intentionally blank
}





/**
@class I_ActorCreationList
@ingroup actor_group_main

Implementations should provide creation data for a number of actors, allowing
these actors to be created at the same time.
*/

I_ActorCreationList::I_ActorCreationList()
{
	// intentionally blank
}

I_ActorCreationList::~I_ActorCreationList()
{
	// intentionally blank
}





/**
@class C_ActorCreationListSingle
@ingroup actor_group_main

Allows a single actor to be created using the I_ActorCreationList interface.
*/

C_ActorCreationListSingle::C_ActorCreationListSingle(const C_ActorCreationData::C_DataRef& creator) :
	I_ActorCreationList(), 
	m_Creator(creator)
{
	// intentionally blank
}

C_ActorCreationListSingle::~C_ActorCreationListSingle()
{
	// intentionally blank
}

U32 C_ActorCreationListSingle::CalcNumCreators() const
{
	return 1;
}

C_ActorCreationData::C_DataRef C_ActorCreationListSingle::GetCreator(const U32 index) const
{
	return (index == 0) ? m_Creator : C_ActorCreationData::C_DataRef();
}





/**
@class C_ActorCreationListMultiple
@ingroup actor_group_main

Allows a number of actors to be created using I_ActorCreationList.
*/

C_ActorCreationListMultiple::C_ActorCreationListMultiple() :
	I_ActorCreationList(), 
	m_Creators()
{
	// intentionally blank
}

C_ActorCreationListMultiple::~C_ActorCreationListMultiple()
{
	// intentionally blank
}

void C_ActorCreationListMultiple::ReserveActorCreationListItems(const U32 count)
{
	if (count > (U32)m_Creators.Capacity())
		m_Creators.Reserve(count);
}

void C_ActorCreationListMultiple::AddActorCreationListItem(const C_ActorCreationData::C_DataRef& creator)
{
	m_Creators.PushBack(creator);
}

U32 C_ActorCreationListMultiple::CalcNumCreators() const
{
	return m_Creators.Size();
}

C_ActorCreationData::C_DataRef C_ActorCreationListMultiple::GetCreator(const U32 index) const
{
	return (index < (U32)m_Creators.Size()) ? m_Creators[index] : C_ActorCreationData::C_DataRef();
}





/**
@class C_ActorCreationListComposite
@ingroup actor_group_main

Allows several I_ActorCreationList objects to be composed into a single list
for simultaneous creation.
*/

C_ActorCreationListComposite::C_ActorCreationListComposite() :
	I_ActorCreationList(), 
	m_Contents()
{
	// intentionally blank
}

C_ActorCreationListComposite::~C_ActorCreationListComposite()
{
	// intentionally blank
}

void C_ActorCreationListComposite::AddCreationListComposite(const I_ActorCreationList* pContents)
{
	if (pContents != NULL)
		m_Contents.PushBack(pContents);
}

void C_ActorCreationListComposite::RemoveCreationListComposite(const I_ActorCreationList* pContents)
{
	C_Contents::Iterator it = Btl::Find(m_Contents, pContents);
	if (it != m_Contents.End())
		m_Contents.Erase(it);
}

U32 C_ActorCreationListComposite::CalcNumCreators() const
{
	U32 count = 0;

	for (C_Contents::ConstIterator it = m_Contents.Begin(); it != m_Contents.End(); ++it)
		count += (*it)->CalcNumCreators();

	return count;
}

C_ActorCreationData::C_DataRef C_ActorCreationListComposite::GetCreator(const U32 index) const
{
	U32 offset = index;

	for (C_Contents::ConstIterator it = m_Contents.Begin(); it != m_Contents.End(); ++it)
	{
		const U32 count = (*it)->CalcNumCreators();

		if (offset < count)
			return (*it)->GetCreator(offset);

		offset -= count;
	}

	return C_ActorCreationData::C_DataRef();
}

} // namespace Actor
