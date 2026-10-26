
#include "stdafx.h"
#include <camera2/Feed.h>
#include <camera2/FeedList.h>

namespace Cam2
{

// A FeedList allows you to have a list of Feeds and set the camera object on all of them in one go

void C_FeedList::Add(I_Feed* cam)
{
	m_list.PushBack(cam);
}

void C_FeedList::Remove(I_Feed* cam)
{
	Ut::Vector<I_Feed *>::Iterator iter = m_list.Begin();
	
	while (iter != m_list.End())
	{
		if (*iter == cam)
			iter = m_list.Erase(iter);
		else
			++iter;
	}
}

void C_FeedList::Clear()
{
	m_list.Clear();
}

void C_FeedList::SetObject(C_CamObj& obj)
{
	for (Ut::Vector<I_Feed *>::Iterator iter = m_list.Begin(); iter != m_list.End(); ++iter)
	{
		(*iter)->SetObject(obj);
	}
}

void C_FeedList::Update(F32 period)
{
	for (Ut::Vector<I_Feed *>::Iterator iter = m_list.Begin(); iter != m_list.End(); ++iter)
	{
		(*iter)->Update(period);
	}
}

int C_FeedList::Size()
{
	return m_list.Size();
}

I_Feed* C_FeedList::operator[] (int i)
{
	return m_list[i];
}

void C_FeedList::DeleteAll()
{
	Ut::Vector<I_Feed *>::Iterator iter = m_list.Begin();
	
	while (iter != m_list.End())
	{
		delete *iter;
		iter = m_list.Erase(iter);
	}
}

} // End of namespace
