#ifndef __NET_PLAYER_VOICE_H__
#define __NET_PLAYER_VOICE_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "../Misc/NetDefines.h"
#include "../Misc/NetAllocated.h"
#include <Network/CommunicatorVoiceBuffer.h>

namespace RacerNet
{
	// forward declerations.
	class C_NetMessage;

	/// An object for storing encoded communicator voice data before sending to a remote peer.
	class C_NetPlayerVoice : public C_NetAllocated
	{
	public:
		C_NetPlayerVoice();
		virtual ~C_NetPlayerVoice();

		/// Allocate dynamic resources requird for operation.
		/** @param requiredSize the number of bytes that we wish to allocate for the voice buffer.
		 *	@return true if the resources were allocated.
		 */
		bool Initialise(const U32 requiredSize);

		/// has the voice buffer seen initialised. 
		bool IsInitialisef(void) const {return m_Initialised;}

		/// Release the dynamic resources.
		/** @see Initialise()
		 */
		void Shutdown(void);

		/// Totally clear the contents of the voice buffer.
		/** Any pending messages will be lost.
		 */
		void Flush(void);

		/// Add encoded voice to the internal buffer.
		/**	@param pBuffer pointer to the voice data to copy into the buffer.
		 *	@param bufferSize number of bytes to copy.
		 *	@return true if the data was successfully copied into the buffer.
		 */
		bool Append(const U8* pBuffer, const U32 bufferSize);

		/// How many bytes of voice data are buffered up.
		U32	Size(void) const {return m_Buffer.Size();} 

		/// Attempt to write any buffered voice to the network message
		/** @param msg correctly configured network message.
		 *	@return true if the voice data was copied or false if not.
		 */
		bool WriteToMessage(C_NetMessage& msg);

	private:
		bool m_Initialised;
		Communicator::C_VoiceBuffer m_Buffer;
	};

	typedef Btl::Vector<C_NetPlayerVoice, Btl::VectorParameters_Array<NET_MAX_LOCAL_PLAYERS> > NetLocalVoiceArray;
	typedef Btl::Vector<NetLocalVoiceArray, Btl::VectorParameters_Array<NET_MAX_VEHICLES-1> > NetVoiceArray;

} // end namespace RacerNet

#endif // __NET_PLAYER_VOICE_H__
