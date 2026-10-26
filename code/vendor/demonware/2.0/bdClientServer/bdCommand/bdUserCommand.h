// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to represent a user command.

#ifndef BD_USER_COMMAND_H
#define BD_USER_COMMAND_H

#include <bdCore/bdCore.h>


/// A class that represents a user command. 
/// A user command represents the actions performed by a player. Define your 
/// own subclass that represents user actions and register it using the 
/// BD_REGISTER_COMMAND_CLASS macro. 
class bdUserCommand
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdUserCommand(const bdUByte8 msec = 0);

		/// Destructor.
		virtual ~bdUserCommand();

		/// Specifies the amount of time a user command accounts for.
		/// \param msec[in]	The amount of time in milliseconds this command 
		///					accounts for.
		void setTime(const bdUByte8 msec);

		/// Specifies the amount of time a user command accounts for.
		/// \param sec[in]	The amount of time in seconds this command 
		///					accounts for.
		void setTimeInSeconds(const bdFloat32 sec);

		/// Retrieves the amount of time this user command accounts for.
		/// \return The amount of time in milliseconds this command 
		///			accounts for.
		bdUByte8 getTime() const;

		/// Retrieves the amount of time this user command accounts for.
		/// \return	The amount of time in seconds this command accounts for.
		bdFloat32 getTimeInSeconds() const;

		/// Clears the contents of this user command.
		virtual void clear();

		/// Writes the contents of a user command into a bitbuffer
		/// \param buffer[out]			The bitbuffer into which this user 
		///								command is written.
		/// \param previousCommand[in]	A pointer to a previous command to 
		///								delta compress against.
		virtual void serialize(bdBitBufferRef buffer,							   
							   bdUserCommand *previousCommand);

		/// Reads the contents of a user command from a bitbuffer
		/// \param buffer[in]	The bitbuffer from which the user command is
		///						read.
		virtual bdBool deserialize(bdBitBufferRef buffer);

	protected:
		
		/// The amount of time in milliseconds that this user command accounts 
		/// for.
		bdUByte8 m_msec;
};

#endif // BD_USER_COMMAND_H
