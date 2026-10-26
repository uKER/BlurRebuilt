#ifndef DMAN_TTS_STRUCTURES_H
#define DMAN_TTS_STRUCTURES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

struct Voice
{
	Voice();

	Btl::String		speaker;
	S32				speed;
	U32				volume;

	static Voice	defaultVoice;
};
	
struct TTS
{
	Btl::String					path;
	Btl::String					fileName;
	Btl::Map<Character, Voice>	voices;
};

struct C_TTSDialogue
{
	Voice voice;
	Btl::String lines;
};

struct C_TTSConversation
{
	Btl::String filePath;
	Btl::Vector<C_TTSDialogue> dialogue;
};

typedef Btl::Map<Btl::String, C_TTSConversation> TTSConversations;

}

#endif
