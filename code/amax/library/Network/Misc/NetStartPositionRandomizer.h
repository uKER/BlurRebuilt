// ***************************************************************
//  NetStartPositionRandomizer   version:  1.0   ·  date: 07/07/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Given a nuumber of vehicles generates a the start
//			position list in random order.
// ***************************************************************

#ifndef __NET_START_POSITION_RANDOMIZER_H__
#define __NET_START_POSITION_RANDOMIZER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Core.h>
#include <Btl/Containers/vector.h>

namespace RacerNet
{
	/// Used to create a set of random start positions.
	/** @param maxStarters maximum number of vehicles we will ever try and create start positions for.
	 */
	template <U32 maxStarters> 
	class C_NetStartPositionRandomizer
	{
	public:
		C_NetStartPositionRandomizer();
		virtual ~C_NetStartPositionRandomizer();  

		/// Generate the random start positions.
		/** @param numStarters how many vehicles are starting the race.
		 *	@param seed value to use as our seed the the random number generator
		 *	@return number of start positions calculated.
		 */
		U32 Randomize(const U32 numStarters, const U32 seed);
		
		Btl::Vector<S32, Btl::VectorParameters_Static<maxStarters> > m_Data;
	};

	// constructor
	template <U32 maxStarters> 
	C_NetStartPositionRandomizer<maxStarters>::C_NetStartPositionRandomizer()
	{
		m_Data.Clear();
	}

	// destructor
	template <U32 maxStarters>
	C_NetStartPositionRandomizer<maxStarters>::~C_NetStartPositionRandomizer()
	{}

	template <U32 maxStarters>
	U32 C_NetStartPositionRandomizer<maxStarters>::Randomize(const U32 numStarters, const U32 seed)
	{
		m_Data.Clear();

		if (0 == numStarters || numStarters > maxStarters)
			return 0;

		Btl::Vector<S32, Btl::VectorParameters_Static<maxStarters> > m_BaseData;
		for (U32 i=0; i<numStarters; i++)
			m_BaseData.PushBack(i);

		srand(seed);

		while (m_BaseData.Size() > 0)
		{
			const S32 index = rand()%(m_BaseData.Size());
			m_Data.PushBack(m_BaseData[index]);
			m_BaseData.EraseByIndex(index);
		}

		return (U32)m_Data.Size();
	}

} // end namespace RacerNet

#endif // __NET_START_POSITION_RANDOMIZER_H__
