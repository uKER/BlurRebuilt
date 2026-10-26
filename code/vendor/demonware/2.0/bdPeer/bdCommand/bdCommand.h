// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A base class to for bdPlayer commands.

#ifndef BD_COMMAND_H
#define BD_COMMAND_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdCommand);

/// A base class to for commands issued by bdPlayers.
/// Derive from this class and add data to describe a players inputs/actions.
/// bdCommand::serialize and bdCommand::deserialize should be overloaded
/// to read and write the extra data in the derived class. Use the 
/// BD_REGISTER_COMAND_CLASS macro to register a derived bdCommand class with 
/// the bdCommandFactory.
///
/// The sending of commands from between peers will constitute 99.9% of all 
/// network traffic, so the smaller the serialized state of your commands the 
/// better.
class bdCommand : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		/// Default constructor.
		bdCommand();

		/// Virtual destructor.
		virtual ~bdCommand();

		/// Write the command data into a bdBitBuffer.
		/// \param buffer[in, out] The bdBitBuffer into which to write our 
		///		data.
		virtual void serialize(bdBitBufferRef buffer);

		/// Set the state of this bdCommand from a previously serialized state.
		/// \param buffer[in] The bdBitBuffer from which to read our state.
		/// \return 
		/// - True if the deserialization was successful.
		/// - False otherwise,
		virtual bdBool deserialize(bdBitBufferRef buffer);

};

#endif // BD_COMMAND_H
