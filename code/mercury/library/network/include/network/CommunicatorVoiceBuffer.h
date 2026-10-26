// ***************************************************************
//  CommunicatorVoiceBuffer   version:  1.0   ·  date: 07/15/2008
//  -------------------------------------------------------------
//  
//  -------------------------------------------------------------
//  Copyright (C) Bizarre Creations 2008 - All Rights Reserved
// ***************************************************************
// author:	David Worswick
// purpose:	Buffer for storing encoded voice data before its sent 
//			to peers in chunks instead of individual bits.
//			Cuts down on bandowdth wastage.
// ***************************************************************

#ifndef __COMMUNICATOR_VOICE_BUFFER_H__
#define __COMMUNICATOR_VOICE_BUFFER_H__

#ifdef _MSC_VER
#pragma once
#endif


namespace Communicator
{
	/// Storage class for mic encoded voice data before it's sent out to peers.
	class C_VoiceBuffer
	{
	public:
		explicit C_VoiceBuffer();
		virtual ~C_VoiceBuffer();

		/// Its possible to get the OS to write drirectly into this buffer from the mic if required.
		friend class C_Core;

		/// Call once to allocate dynamic resources.
		/** @param bufferSize size in bytes of the voice buffer we wish to allocate.
		 *	@return true if the buffer was initialised successfully.
		 */
		bool Initialise(const U32 bufferSize);

		/// If the buffer initialised and ready for use.
		bool IsInitialised(void) const {return m_Initialised;}

		/// Deallocate dynamic resources and close the buffer.
		void Shutdown(void);

		/// Add voice data to the internal buffer.
		/**	If the data to be appended can't fit within the internal buffer then it will be discarded.
		 *	@param pData pointer to the encoded voice data to append.
		 *	@param dataSize number of bytes of voice data to append.
		 *	@return true if the voice was added to the buffer.
		 */
		bool Append(const U8 *pData, const U32 dataSize);

		/// Add voice data to the internal buffer.
		/**	If the data to be appended can't fit within the internal buffer then it will be discarded.
		 *	@param buffer copy voice data from this buffer into the internal one.
		 *	@return true if the voice was added to the buffer.
		 */
		bool Append(const C_VoiceBuffer& buffer);

		/// Clear out the contents of the buffer.
		void Flush(void);

		/// Access to the currently encoded voice buffer.
		U8* Data(void) const {return m_pData;}

		/// Get the current number of bytes in the buffer.
		U32	Size(void) const {return m_CurrentSize;}

		/// Get the maximun number of bytes we can store in the buffer.
		U32 Capacity(void) const {return m_AllocatedSize;}

	private:
		C_VoiceBuffer(const C_VoiceBuffer& original);			// private copy constructor
		C_VoiceBuffer& operator = (const C_VoiceBuffer rhs);	// private un coded assignment operator

	private:
		bool m_Initialised;
		U32 m_AllocatedSize;
		U32 m_CurrentSize;
		U8 *m_pData;
	};
}

#endif // __COMMUNICATOR_VOICE_BUFFER_H__


