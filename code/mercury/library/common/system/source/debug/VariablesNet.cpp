#include "SystemPch.h"
#include <Core/Core.h>
#include <System/Debug/Debug.h>
#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include <System/Debug/VariablesNet.h>

#if defined(PLATFORM_PS3)
#undef _stricmp
#define _stricmp stricmp
#define _snprintf snprintf
#endif

DebugVarFileCallback gGameLoadDebugVariables = 0;
DebugVarFileCallback gGameSaveDebugVariables = 0;

#ifndef CFG_GOLD

// TODO: Split the update stuff from the network stuff for persisting variables

extern bool gDebugVariablesListDirty;
extern bool gDebugVariablesDirty;

//
// Consoles
//

struct DebugConsole
{
	char					name[32];
	char					prompt[16];
	Debug::ConsoleCallback	callback;
	void*					context;

public:
	DebugConsole() :
		callback(0),
		context(0)
	{
		*name	= '\0';
		*prompt	= '\0';
	}
};

const int kMaxDebugConsoles = 8;

int				gDebugConsolesCount = 0;
DebugConsole	gDebugConsoles[kMaxDebugConsoles];

DebugConsole* DebugConsole_Find(const char* name)
{
	for(int i = 0; i < gDebugConsolesCount; i++)
	{
		if (_stricmp(gDebugConsoles[i].name, name) == 0)
			return &gDebugConsoles[i];
	}

	return 0;
}

DebugConsole* DebugConsole_Add(const char* name)
{
	if (DebugConsole* con = DebugConsole_Find(name))
		return con;

	if (gDebugConsolesCount >= kMaxDebugConsoles)
		return 0;

	DebugConsole* con = &gDebugConsoles[gDebugConsolesCount++];

	_snprintf(con->name, sizeof(con->name), "%s", name);

	return con;
}

//
// Channels
//

namespace Debug
{
	void AddDvBuffer(const char* channel, ArteryU32 severity, const char* text);
	void SendDvBuffer(ArteryConnection con);
}

struct DebugLogChannel
{
	const char*		name;
	Debug::Severity	reportSeverity;

public:
	DebugLogChannel() :
		name(0),
		reportSeverity(Debug::kSeverityNormal)
	{
	}

	~DebugLogChannel()
	{
		if (name != 0)
		{
			free((void*)name);
		}
	}
};

const int kDebugLogMaxChannels = 128;

int				gDebugLogChannelCount;
DebugLogChannel	gDebugLogChannels[kDebugLogMaxChannels];

Debug::DebugLogCallback gDebugLog_ChannelCallback;

DebugLogChannel* DebugLog_FindChannel(const char* name)
{
	for(int i = 0; i < gDebugLogChannelCount; i++)
	{
		DebugLogChannel* ch = &gDebugLogChannels[i];

		if (_stricmp(ch->name, name) == 0)
			return ch;
	}

	return 0;
}

bool DebugLog_AddChannel(const char* name, Debug::Severity reportSeverity)
{
	if (DebugLogChannel* ch = DebugLog_FindChannel(name))
	{
		ch->reportSeverity = reportSeverity;
		return true;
	}

	if (gDebugLogChannelCount >= kDebugLogMaxChannels)
		return false;

	DebugLogChannel* ch = &gDebugLogChannels[gDebugLogChannelCount++];

	ch->name			= strdup(name);
	ch->reportSeverity	= reportSeverity;

	return true;
}

//
// List message
//

void DvmList(ArteryConnection& con)
{
	ArteryMessageBuilder b;

	if (!b.Write(kDvmVarList))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;

	bool failed = false;

	for(Debug::Iterator it; it && !failed; ++it)
	{
		if (it->Flags() & Debug::kVfHide)
			continue;

		ArteryU32 tag = 0;

		switch(it->Type())
		{
			case Debug::kVtBool:		tag = kDvtBool;		break;
			case Debug::kVtInt:			tag = kDvtInt;		break;
			case Debug::kVtFloat:		tag = kDvtFloat;	break;
			case Debug::kVtEnum:		tag = kDvtEnum;		break;
			case Debug::kVtString:		tag = kDvtString;	break;
			case Debug::kVtFunction:	tag = kDvtFunction;	break;
			case Debug::kVtVector2:		tag = kDvtVector2;	break;
			case Debug::kVtVector3:		tag = kDvtVector3;	break;
			case Debug::kVtVector4:		tag = kDvtVector4;	break;
			case Debug::kVtColour:		tag = kDvtColour;	break;
		}

		if (!tag)
			continue;

		if (!b.Write(tag))
		{
			failed = true;
			break;
		}

		if (!b.Write(it->Name()))
		{
			failed = true;
			break;
		}

		if (!b.Write(it->Description()))
		{
			failed = true;
			break;
		}

		if (!b.Write((ArteryU32)it->Revision()))
		{
			failed = true;
			break;
		}

		switch(it->Type())
		{
			case Debug::kVtBool:
			{
				Debug::Bool& var = (Debug::Bool&)*it;

				if (!b.Write((ArteryI32)var.Value()))
					failed = true;

				if (!b.Write((ArteryI32)var.Default()))
					failed = true;
			}
			break;

			case Debug::kVtInt:
			{
				Debug::Int& var = (Debug::Int&)*it;

				if (!b.Write((ArteryI32)var.Value()))
					failed = true;

				ArteryU32 flags = 0;

				if (var.Flags() & Debug::kVfRange)
					flags |= kDfRange;

				if (!b.Write(flags))
					failed = true;

				if (!b.Write((ArteryI32)var.Low()))
					failed = true;

				if (!b.Write((ArteryI32)var.High()))
					failed = true;

				if (!b.Write((ArteryI32)var.Default()))
					failed = true;
			}
			break;

			case Debug::kVtFloat:
			{
				Debug::Float& var = (Debug::Float&)*it;

				if (!b.Write((ArteryF32)var.Value()))
					failed = true;

				ArteryU32 flags = 0;

				if (var.Flags() & Debug::kVfRange)
					flags |= kDfRange;

				if (!b.Write(flags))
					failed = true;

				if (!b.Write((ArteryF32)var.Low()))
					failed = true;

				if (!b.Write((ArteryF32)var.High()))
					failed = true;

				if (!b.Write((ArteryF32)var.Default()))
					failed = true;
			}
			break;

			case Debug::kVtEnum:
			{
				Debug::EnumBase& var = (Debug::EnumBase&)*it;

				if (!b.Write(var.ValueAsName()))
					failed = true;

				ArteryU32 count = 0;

				for(Debug::OptionIterator it(&var); it; ++it)
					count++;

				if (!b.Write(count))
					failed = true;

				for(Debug::OptionIterator it(&var); it; ++it)
				{
					if (!b.Write(it->name))
						failed = true;
				}

				if (!b.Write(var.DefaultAsName()))
					failed = true;
			}
			break;

			case Debug::kVtString:
			{
				Debug::String& var = (Debug::String&)*it;

				if (!b.Write(var.Value()))
					failed = true;

				if (!b.Write(var.Default()))
					failed = true;
			}
			break;

			case Debug::kVtFunction:
			{
				Debug::Function& var = (Debug::Function&)*it;
			}
			break;

			case Debug::kVtVector2:
			{
				Debug::Vector2& var = (Debug::Vector2&)*it;

				if (!b.Write((ArteryF32)var->vx))
					failed = true;

				if (!b.Write((ArteryF32)var->vy))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vx))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vy))
					failed = true;
			}
			break;

			case Debug::kVtVector3:
			{
				Debug::Vector3& var = (Debug::Vector3&)*it;

				if (!b.Write((ArteryF32)var->vx))
					failed = true;

				if (!b.Write((ArteryF32)var->vy))
					failed = true;

				if (!b.Write((ArteryF32)var->vz))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vx))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vy))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vz))
					failed = true;
			}
			break;

			case Debug::kVtVector4:
			{
				Debug::Vector4& var = (Debug::Vector4&)*it;

				if (!b.Write((ArteryF32)var->vx))
					failed = true;

				if (!b.Write((ArteryF32)var->vy))
					failed = true;

				if (!b.Write((ArteryF32)var->vz))
					failed = true;

				if (!b.Write((ArteryF32)var->vw))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vx))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vy))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vz))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().vw))
					failed = true;
			}
			break;

			case Debug::kVtColour:
			{
				Debug::Colour& var = (Debug::Colour&)*it;

				if (!b.Write((ArteryF32)var->r))
					failed = true;

				if (!b.Write((ArteryF32)var->g))
					failed = true;

				if (!b.Write((ArteryF32)var->b))
					failed = true;

				if (!b.Write((ArteryF32)var->a))
					failed = true;

				if (!b.Write((ArteryF32)var.Gamma()))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().r))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().g))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().b))
					failed = true;

				if (!b.Write((ArteryF32)var.Default().a))
					failed = true;
			}
			break;
		}
	}

	if (!failed && b.Write(kDvtEnd))
	{
		ArteryMessage msg;

		if (b.Build(msg))
			con.Send(msg);
	}
}

void DvmListChannels(ArteryConnection& con)
{
	ArteryMessageBuilder b;

	if (!b.Write(kDvmDebugLogRegister))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;

	bool failed = false;

	for(int i = 0; i < gDebugLogChannelCount; i++)
	{
		DebugLogChannel* ch = &gDebugLogChannels[i];

		if (!b.Write(ch->name))
			return;

		if (!b.Write(ArteryU32(ch->reportSeverity)))
			return;
	}

	if (!failed)
	{
		ArteryMessage msg;

		if (b.Build(msg))
			con.Send(msg);
	}
}

void DvmListConsoles(ArteryConnection& con)
{
	ArteryMessageBuilder b;

	if (!b.Write(kDvmConsolePrompt))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;

	bool failed = false;

	for(int i = 0; i < gDebugConsolesCount; i++)
	{
		DebugConsole* ch = &gDebugConsoles[i];

		if (!b.Write(ch->name))
			return;

		if (!b.Write(ch->prompt))
			return;
	}

	if (!failed)
	{
		ArteryMessage msg;

		if (b.Build(msg))
			con.Send(msg);
	}
}

//
// List set
//

void DvmSet(ArteryConnection& con, ArteryMessageReader& r, ArteryU32 ver)
{
	const char* name;

	if (!r.Read(name))
		return;

	Debug::VarBase* vb = Debug::FindByName(name);

	if (!vb)
		return;

	ArteryU32 typeTag;

	if (!r.Read(typeTag))
		return;

	switch(typeTag)
	{
		case kDvtBool:
		{
			if (vb->Type() != Debug::kVtBool)
				return;

			ArteryI32 value;

			if (!r.Read(value))
				return;

			((Debug::Bool*)vb)->SetValue(value != 0);
		}
		break;

		case kDvtInt:
		{
			if (vb->Type() != Debug::kVtInt)
				return;

			ArteryI32 value;

			if (!r.Read(value))
				return;

			((Debug::Int*)vb)->SetValue(value);
		}
		break;

		case kDvtFloat:
		{
			if (vb->Type() != Debug::kVtFloat)
				return;

			ArteryF32 value;

			if (!r.Read(value))
				return;

			((Debug::Float*)vb)->SetValue(value);
		}
		break;

		case kDvtEnum:
		{
			if (vb->Type() != Debug::kVtEnum)
				return;

			const char* value;

			if (!r.Read(value))
				return;

			((Debug::EnumBase*)vb)->SetValueByName(value);
		}
		break;

		case kDvtString:
		{
			if (vb->Type() != Debug::kVtString)
				return;

			const char* value;

			if (!r.Read(value))
				return;

			((Debug::String*)vb)->SetValue(value);
		}
		break;

		case kDvtFunction:
		{
			if (vb->Type() != Debug::kVtFunction)
				return;

			((Debug::Function*)vb)->Call();
		}
		break;

		case kDvtVector2:
		{
			if (vb->Type() != Debug::kVtVector2)
				return;

			ArteryF32 vx, vy;

			if (!r.Read(vx) || !r.Read(vy))
				return;

			((Debug::Vector2*)vb)->SetValue(vx, vy);
		}
		break;

		case kDvtVector: // Backwards compat.
		case kDvtVector3:
		{
			if (vb->Type() != Debug::kVtVector3)
				return;

			ArteryF32 vx, vy, vz;

			if (!r.Read(vx) || !r.Read(vy) || !r.Read(vz))
				return;

			((Debug::Vector3*)vb)->SetValue(vx, vy, vz);
		}
		break;

		case kDvtVector4:
		{
			if (vb->Type() != Debug::kVtVector4)
				return;

			ArteryF32 vx, vy, vz, vw;

			if (!r.Read(vx) || !r.Read(vy) || !r.Read(vz) || !r.Read(vw))
				return;

			((Debug::Vector4*)vb)->SetValue(vx, vy, vz, vw);
		}
		break;

		case kDvtColour:
		{
			if (vb->Type() != Debug::kVtColour)
				return;

			ArteryF32 cr, cg, cb, ca;

			if (!r.Read(cr) || !r.Read(cg) || !r.Read(cb) || !r.Read(ca))
				return;

			((Debug::Colour*)vb)->SetValue(cr, cg, cb, ca);
		}
		break;

		vb->SetDirty();
	}

	vb->DoNotify();
}

//
// Main message handler
//

void DvmMessage(ArteryConnection& con, const ArteryMessage& msg)
{
	ArteryMessageReader r(msg);

	ArteryU32 tag;
	ArteryU32 ver;

	if (!r.Read(tag) || !r.Read(ver))
		return;

	if (ver != Debug::kNetVerClient0)
		return;

	switch(tag)
	{
		case kDvmList:
			DvmList(con);
			DvmListChannels(con);
			DvmListConsoles(con);
		break;

		case kDvmSetVar:
			DvmSet(con, r, ver);
		break;

		case kDvmSetChannelSeverity:
		{
			if (gDebugLog_ChannelCallback)
			{
				const char*	name;
				ArteryU32	severity;

				if (r.Read(name) && r.Read(severity))
					gDebugLog_ChannelCallback(name, (Debug::Severity)severity);
			}
		}
		break;

		case kDvmConsoleCommand:
		{
			const char* name;
			const char* text;

			if (!r.Read(name) || !r.Read(text))
				return;

			if (DebugConsole* cons = DebugConsole_Find(name))
			{
				if (cons->callback)
				{
					Debug::ConsoleEvent ev;

					ev.name = name;
					ev.text = text;

					cons->callback(&ev, cons->context);
				}
			}
		}
		break;
	}
}

//
// Debug peer
//

struct DebugPeer
{
	ArteryConnection con;
};

const int kMaxDebugPeers = 4;

DebugPeer* gDebugPeers[kMaxDebugPeers];

ArteryLock gDebugPeerCs;

bool AddDebugPeer(DebugPeer* peer)
{
	bool result = false;

	gDebugPeerCs.Wait();

	Debug::SendDvBuffer(peer->con);

	for(int i = 0; i < kMaxDebugPeers; i++)
	{
		if (gDebugPeers[i] == 0)
		{
			gDebugPeers[i]	= peer;
			result			= true;

			break;
		}
	}

	gDebugPeerCs.Release();

	return result;
}

void RemoveDebugPeer(DebugPeer* peer)
{
	gDebugPeerCs.Wait();

	for(int i = 0; i < kMaxDebugPeers; i++)
	{
		if (gDebugPeers[i] == peer)
		{
			gDebugPeers[i] = 0;
			break;
		}
	}

	gDebugPeerCs.Release();
}

void DebugVariablesNetProc(ArteryConnection con, void* context)
{
	DebugPeer dp;
	dp.con = con;

	if (AddDebugPeer(&dp))
	{
		ArteryWaitContext ctx;

		while(con.IsConnected())
			con.Wait(ctx);

		RemoveDebugPeer(&dp);
	}
}

const int kDebugSaveTime = 30;

int gDebugSaveTicks = 300;

bool gDebugNetworkStarted = false;

void Debug::NetworkStart()
{
	gDebugNetworkStarted = true;

	//MemoryBarrier();
	gDebugPeerCs.Wait();
	gDebugPeerCs.Release();

	if (gGameLoadDebugVariables)
		gGameLoadDebugVariables();

	ArteryRegister(kDebugVarNetId, DebugVariablesNetProc, 0);
}

void Debug::NetworkUpdate()
{
	if (!gDebugNetworkStarted)
		return;

	// Update debug variables from network

	bool hasPeers = false;

	gDebugPeerCs.Wait();

	for(int i = 0; i < kMaxDebugPeers; i++)
	{
		DebugPeer* peer = gDebugPeers[i];

		if (!peer || !peer->con.IsConnected())
			continue;

		hasPeers = true;

		for(;;)
		{
			ArteryMessage msg;

			if (!peer->con.TryReceive(msg))
				break;

			DvmMessage(peer->con, msg);
		}
	}

	gDebugPeerCs.Release();

	// Update revision numbers for debug variables, and build update packet for remote peers

	bool hasDirty	= false;
	bool needToSave	= false;

	if (gDebugVariablesDirty)
	{
		for(Debug::Iterator it; it; ++it)
		{
			if (it->_flags & kVfDirty)
			{
				hasDirty = true; 

				if (it->_flags & kVfPersist)
					needToSave = true;
			}
		}

		gDebugVariablesDirty = false;
	}

	if (needToSave && (gDebugSaveTicks <= 0))
		gDebugSaveTicks = kDebugSaveTime;

	if ((gDebugSaveTicks > 0) && (--gDebugSaveTicks <= 0))
	{
		if (gGameSaveDebugVariables)
			gGameSaveDebugVariables();
	}

	if (hasDirty)
	{
		if (hasPeers)
		{
			ArteryMessageBuilder b;

			if (b.Write(kDvmUpdatedVars) && b.Write(ArteryU32(Debug::kNetVerServer2)))
			{
				bool failed = false;

				for(Debug::Iterator it; it; ++it)
				{
					Debug::VarBase* var = it;

					if ((var->_flags & kVfDirty) == 0)
						continue;

					var->_flags &= ~kVfDirty;
					var->_revision++;

					if (it->Flags() & Debug::kVfHide)
						continue;

					ArteryU32 tag = 0;

					switch(it->Type())
					{
						case Debug::kVtBool:		tag = kDvtBool;		break;
						case Debug::kVtInt:			tag = kDvtInt;		break;
						case Debug::kVtFloat:		tag = kDvtFloat;	break;
						case Debug::kVtEnum:		tag = kDvtEnum;		break;
						case Debug::kVtString:		tag = kDvtString;	break;
						case Debug::kVtFunction:	tag = kDvtFunction;	break;
						case Debug::kVtVector2:		tag = kDvtVector2;	break;
						case Debug::kVtVector3:		tag = kDvtVector3;	break;
						case Debug::kVtVector4:		tag = kDvtVector4;	break;
						case Debug::kVtColour:		tag = kDvtColour;	break;
					}

					if (!tag)
						continue;

					if (!b.Write(tag))
					{
						failed = true;
						break;
					}

					if (!b.Write(it->Name()))
					{
						failed = true;
						break;
					}

					if (!b.Write((ArteryU32)it->Revision()))
					{
						failed = true;
						break;
					}

					switch(it->Type())
					{
						case Debug::kVtBool:
						{
							Debug::Bool& var = (Debug::Bool&)*it;

							if (!b.Write((ArteryI32)var.Value()))
								failed = true;
						}
						break;

						case Debug::kVtInt:
						{
							Debug::Int& var = (Debug::Int&)*it;

							if (!b.Write((ArteryI32)var.Value()))
								failed = true;
						}
						break;

						case Debug::kVtFloat:
						{
							Debug::Float& var = (Debug::Float&)*it;

							if (!b.Write((ArteryF32)var.Value()))
								failed = true;
						}
						break;

						case Debug::kVtEnum:
						{
							Debug::EnumBase& var = (Debug::EnumBase&)*it;

							if (!b.Write(var.ValueAsName()))
								failed = true;
						}
						break;

						case Debug::kVtString:
						{
							Debug::String& var = (Debug::String&)*it;

							if (!b.Write(var.Value()))
								failed = true;
						}
						break;

						case Debug::kVtFunction:
						{
							Debug::Function& var = (Debug::Function&)*it;
						}
						break;

						case Debug::kVtVector2:
						{
							Debug::Vector2& var = (Debug::Vector2&)*it;

							if (!b.Write((ArteryF32)var->vx))
								failed = true;

							if (!b.Write((ArteryF32)var->vy))
								failed = true;
						}
						break;

						case Debug::kVtVector3:
						{
							Debug::Vector3& var = (Debug::Vector3&)*it;

							if (!b.Write((ArteryF32)var->vx))
								failed = true;

							if (!b.Write((ArteryF32)var->vy))
								failed = true;

							if (!b.Write((ArteryF32)var->vz))
								failed = true;
						}
						break;

						case Debug::kVtVector4:
						{
							Debug::Vector4& var = (Debug::Vector4&)*it;

							if (!b.Write((ArteryF32)var->vx))
								failed = true;

							if (!b.Write((ArteryF32)var->vy))
								failed = true;

							if (!b.Write((ArteryF32)var->vz))
								failed = true;

							if (!b.Write((ArteryF32)var->vw))
								failed = true;
						}
						break;

						case Debug::kVtColour:
						{
							Debug::Colour& var = (Debug::Colour&)*it;

							if (!b.Write((ArteryF32)var->r))
								failed = true;

							if (!b.Write((ArteryF32)var->g))
								failed = true;

							if (!b.Write((ArteryF32)var->b))
								failed = true;

							if (!b.Write((ArteryF32)var->a))
								failed = true;
						}
						break;
					}
				}

				if (!failed && b.Write(kDvtEnd))
				{
					ArteryMessage m;

					if (b.Build(m))
					{
						gDebugPeerCs.Wait();

						for(int i = 0; i < kMaxDebugPeers; i++)
						{
							DebugPeer* peer = gDebugPeers[i];

							if (!peer)
								continue;

							peer->con.Send(m);
						}

						gDebugPeerCs.Release();
					}
				}
			}
		}
		else
		{
			for(Debug::Iterator it; it; ++it)
			{
				Debug::VarBase* var = it;

				if ((var->_flags & kVfDirty) == 0)
					continue;

				var->_flags &= ~kVfDirty;
				var->_revision++;
			}
		}
	}

	if (gDebugVariablesListDirty)
	{
		if (hasPeers)
		{
			gDebugPeerCs.Wait();

			for(int i = 0; i < kMaxDebugPeers; i++)
			{
				DebugPeer* peer = gDebugPeers[i];

				if (!peer)
					continue;

				DvmList(peer->con);
			}

			gDebugPeerCs.Release();
		}

		gDebugVariablesListDirty = false;
	}
}

void Debug::RegisterDebugLogCallback(DebugLogCallback callback)
{
	gDebugLog_ChannelCallback = callback;
}

bool Debug::RegisterDebugLogChannel(const char* name, Severity reportSeverity)
{
	if (!DebugLog_AddChannel(name, reportSeverity))
		return false;

	if (!gDebugNetworkStarted)
		return true;

	// TODO: Buffer messages and send in a lump?

	ArteryMessageBuilder b;

	if (!b.Write(kDvmDebugLogRegister))
		return false;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return false;
		
	if (!b.Write(name))
		return false;

	if (!b.Write(ArteryU32(reportSeverity)))
		return false;

	ArteryMessage m;

	if (b.Build(m))
	{
		gDebugPeerCs.Wait();

		for(int i = 0; i < kMaxDebugPeers; i++)
		{
			DebugPeer* peer = gDebugPeers[i];

			if (!peer)
				continue;

			peer->con.Send(m);
		}

		gDebugPeerCs.Release();
	}

	return true;
}

void Debug::DebugLogLn(const char* channel, Severity severity, const char* text)
{
	AddDvBuffer(channel, severity, text);

	if (!gDebugNetworkStarted)
		return;

	ArteryMessageBuilder b;

	if (!b.Write(kDvmDebugLog))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;

	if (!b.Write(channel))
		return;

	if (!b.Write(ArteryU32(severity)))
		return;

	if (!b.Write(text))
		return;

	ArteryMessage m;

	if (b.Build(m))
	{
		gDebugPeerCs.Wait();

		for(int i = 0; i < kMaxDebugPeers; i++)
		{
			DebugPeer* peer = gDebugPeers[i];

			if (!peer)
				continue;

			peer->con.Send(m);
		}

		gDebugPeerCs.Release();
	}
}

void Debug::ConsoleLn(const char* name, const char* text)
{
	if (!gDebugNetworkStarted)
		return;

	// TODO: Buffer messages and send in a lump?

	ArteryMessageBuilder b;

	if (!b.Write(kDvmConsoleOutput))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;
		
	if (!b.Write(name))
		return;

	if (!b.Write(text))
		return;

	ArteryMessage m;

	if (b.Build(m))
	{
		gDebugPeerCs.Wait();

		for(int i = 0; i < kMaxDebugPeers; i++)
		{
			DebugPeer* peer = gDebugPeers[i];

			if (!peer)
				continue;

			peer->con.Send(m);
		}

		gDebugPeerCs.Release();
	}
}

void Debug::ConsolePrompt(const char* name, const char* prompt)
{
	if (!gDebugNetworkStarted)
		return;

	DebugConsole* cons = DebugConsole_Find(name);

	if (!cons)
		return;

	_snprintf(cons->prompt, sizeof(cons->prompt), "%s", prompt);

	// TODO: Buffer messages and send in a lump?

	ArteryMessageBuilder b;

	if (!b.Write(kDvmConsolePrompt))
		return;

	if (!b.Write(ArteryU32(Debug::kNetVerServer2)))
		return;
		
	if (!b.Write(name))
		return;

	if (!b.Write(prompt))
		return;

	ArteryMessage m;

	if (b.Build(m))
	{
		gDebugPeerCs.Wait();

		for(int i = 0; i < kMaxDebugPeers; i++)
		{
			DebugPeer* peer = gDebugPeers[i];

			if (!peer)
				continue;

			peer->con.Send(m);
		}

		gDebugPeerCs.Release();
	}
}

void Debug::RegisterConsole(const char* name, ConsoleCallback callback, void* context)
{
	DebugConsole* cons = DebugConsole_Add(name);

	if (!cons)
		return;

	cons->callback	= callback;
	cons->context	= context;

	ConsolePrompt(name, ">");
}

#else // CFG_GOLD

void Debug::NetworkStart()
{
}

void Debug::NetworkUpdate()
{
}

void Debug::RegisterDebugLogCallback(DebugLogCallback callback)
{
}

bool Debug::RegisterDebugLogChannel(const char* name, Severity reportSeverity)
{
	return false;
}

void Debug::DebugLogLn(const char* channel, Severity severity, const char* text)
{
}

void Debug::ConsoleLn(const char* channel, const char* text)
{
}

void Debug::ConsolePrompt(const char* name, const char* prompt)
{
}

void Debug::RegisterConsole(const char* name, ConsoleCallback callback, void* context)
{
}

#endif // CFG_GOLD
