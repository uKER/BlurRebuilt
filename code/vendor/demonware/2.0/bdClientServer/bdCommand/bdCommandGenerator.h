// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An abstract base class used by bdClient to generate user commands.


#ifndef BD_COMMAND_GENERATOR_H
#define BD_COMMAND_GENERATOR_H

#include <bdCore/bdCore.h>

//#include <bdClientServer/bdPlayer/bdPlayer.h>

#include <bdClientServer/bdCommand/bdUserCommand.h>

BD_REFERENCE(bdPlayer);

/// An abstract base class used by bdClient to generate user commands.
/// An instance of bdCommandGenerator should be registered with
/// bdClient::setCommandGenerator in order that commands are generated.
/// The rate at which commands are generated can be set by
/// bdClient::setCommandInterval, the default interval is 0 in which
/// case bdCommandGenerator::generateCommand is called on each call to
/// bdClient::step, for each local bdPlayer.
///
/// No default implementation is provided as BitDemon has no knowledge
/// of the contents of commands derived from bdUserCommand, a warning
/// will be thrown if no such generator has been registered with the bdClient.
class bdCommandGenerator
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdCommandGenerator();

		/// Should be overloaded to generate a user command for a bdPlayer.
		/// It is possible not to generate a command by returning false,
		/// however if the replica the player is controlling implements
		/// bdReplica::extrapolate it is recommended that true is always
		/// returned, otherwise the command will not be sent to the server
		/// and the replica will stall, until the next command is issued.
		/// \param player[in]	The player for whom a command should
		///						be generated.
		/// \param command[in,out]	A pointer to an instance of the
		///							bdUserCommand that should receive the new
		///							command, if any. This is an instance of the
		///							command class registered using the
		///							BD_REGISTER_COMMAND_CLASS macro. This
		///							instance will have had bdUserCommand::clear
		///							called upon it, and the command time will
		///							be set.
		/// \return
		/// - True, if the command should be sent to the server.
		/// - False, if the command should not be sent to the server.
		virtual bdBool generateCommand(bdPlayerRef player,
									   bdUserCommand *const command) = 0;
};

#endif // BD_COMMAND_GENERATOR_H
