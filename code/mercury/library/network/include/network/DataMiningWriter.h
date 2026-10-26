// ***************************************************************
//  DataRecorder   version:  1.0   ·  date: 05/08/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Data miniing writer for storing all player actions to the 
//			server so we can track what players are doing post launch.
//			Uses demonware so we can only use if we have a 
//			connection to the demonware lobby service.
// ***************************************************************

#ifndef __NETWORK_DATA_MINING_WRITER_H__
#define __NETWORK_DATA_MINING_WRITER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Containers/list.h>
#include <Btl/Containers/vector.h>
#include <Network/DemonwareTask.h>


namespace NetPlay
{
	// forward declerations
	class C_DemonwareTask;

	/// Buffer used internally by the C_DataMiningWriter object.
	/** game has no need to create objects of this type.
	 */
	class C_DataMiningBuffer : public Btl::ListElementDouble<C_DataMiningBuffer>
	{
	public:
		C_DataMiningBuffer() {m_Used = 0;}
		virtual ~C_DataMiningBuffer() {};
		void Destroy(void) {m_Used = 0;}
	public:
		 Btl::Vector<char, Btl::VectorParameters_Array<1024> > m_Data;
		 U32 m_Used;
	};

	typedef Btl::List<C_DataMiningBuffer> DataMiningBufferList;



	/// Stores multiple game events and writes them to the data mining server as a big batch.
	/** We can then request the data back after launch to evaluate what players did in the game.
	 *	This requires a valid connection to demonware, so if using on xbox30 you will need an LSP to authenticate 
	 *	to the demonware lobby gateway before it will function.
	 */
	class C_DataMiningWriter
	{
	public :
		C_DataMiningWriter();
		virtual ~C_DataMiningWriter();

		/// Initialise dynamic resources.
		/** @param numBuffers how many buffers we wish to allocate
		 *	@return true if all dynalic resources got allocated
		 */
		bool Initialise(const U32 numBuffers);

		/// Are we currently writing the buffers to the server.
		bool IsWriting(void) const;

		/// Add a string  to the buffer for uploading to the server later.
		/** @param str null terminated string for uploading
		 *	@return true if the string was added to a buffer. 
		 */
		bool Add(const char *str); 

		/// start the process of uploadiing the current buffers to the server.
		/** @see Poll()
		 *	@return true if the asynchronous process of writing the data buffers the the server was started.
		 */
		bool WriteToServer(void);

		/// Call once per frame to pump the process of uploading to the server.
		void Poll(void);

		/// Close the object down and release any dynamic resources.
		void Shutdown(void);

	private:
		C_DataMiningWriter(const C_DataMiningWriter& original);				// private copy constructor
		C_DataMiningWriter& operator = (const C_DataMiningWriter rhs);		// private un coded assignment operator
		void Flush(void);

	private:
		bool m_Initialised;
		bool m_Writing;
		C_DataMiningBuffer* m_pStorage;
		U32 m_NumBuffersAllocated;
		DataMiningBufferList m_InUseList;
		DataMiningBufferList m_FreeList; 
		bdRemoteTaskRef m_AsyncTask;
	};
}

#endif
