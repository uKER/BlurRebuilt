// ***************************************************************
//  NetAllocated   version:  1.0   ·  date: 05/21/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	simple class to derrive from to implement an allocated
//			field.
// ***************************************************************

#ifndef __NET_ALLOCATED_H__
#define __NET_ALLOCATED_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace RacerNet
{
	/// used as a derrived class for tracking when we have flagged that the object is in use by the game.
	class C_NetAllocated
	{
	public:
		C_NetAllocated() : m_Available(true)
		{}
		virtual ~C_NetAllocated()
		{SetAvailable(false);}

		void SetAvailable(const bool a) {m_Available = a;}
		bool IsAvailable(void) const {return m_Available;}

	private:
		bool m_Available;
	};

} // end namespace RacerNet

#endif // __NET_ALLOCATED_H__
