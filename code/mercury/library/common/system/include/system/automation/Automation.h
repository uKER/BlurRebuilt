#ifndef _AUTOMATION_H_
#define _AUTOMATION_H_

#ifdef _MSC_VER
#pragma once
#endif

#include "system/artery/Artery.h"
#include "btl/containers/list.h"

class ArteryMessageReader;

namespace Automation
{
	typedef Util::C_String ProblemString;

	typedef void (*BroadcastMessageClassCallback)(const Automation::ProblemString& n, int i, const Automation::ProblemString& area);

	class AutomationStatus
	{
	private:
		struct Item
		{
			Util::C_String6	name;
			int				value;
		};

		Btl::List<Item>		values;

	public:
		void Add(Util::C_String6 name, int value);
	};

	class CommandContext
	{
	protected:
		ArteryU32				m_commandID;
		const char*				m_command;
		ArteryMessageReader&	m_msgReader;
		ArteryConnection		m_connection;

	public:
		CommandContext(ArteryConnection con, ArteryU32 commandID, const char* command, ArteryMessageReader& mr) : m_connection(con), m_commandID(commandID), m_command(command), m_msgReader(mr)
		{
		}

		ArteryU32 ID() const
		{
			return m_commandID;
		}

		const char* Command() const
		{
			return m_command;
		}

		const bool IsCommand(const char* compareString) const
		{
			return (stricmp(m_command, compareString) == 0);
		}

		ArteryMessageReader& MessageReader()
		{
			return m_msgReader;
		}

		void SingleReply(bool value);
		void SingleReply(int value);
		void SingleReply(float value);
		void SingleReply(const char* message);
	};

	typedef bool (*Callback)(CommandContext& commandContext);

	int InitialiseNetworkConnection(void);
	void RegisterCallback(Callback messageCallback);

	bool IsAutomationInProgress();

	void Assert(const char* condition, const char* file, int line, const char *func, const char* msg);
	void Notify(const char* message);
	void Problem(const char* message);

	void NetworkUpdate();

	void StopRunning(const char* message);

	void SendProblemFoundMsg(const ProblemString& name, const Automation::ProblemString& area);
	void SendProblemFoundMsg(const ProblemString& name, int index, const Automation::ProblemString& area);

	void SetBroadcastMessageClass(BroadcastMessageClassCallback func);
}

#endif // _AUTOMATION_H_
