#include "AudioPch.h"

using namespace Audio;


AudioS32Convertor::AudioS32Convertor()
{
}


AudioS32Convertor::AudioS32Convertor(S32 s32)
{
	m_s32 = CommandLineApp::GetApp()->DoEndian(s32);
}


AudioS32Convertor::operator S32()
{
	return CommandLineApp::GetApp()->DoEndian(m_s32);
}


//S32* AudioS32Convertor::operator =(int s32)
//{
//	m_s32 = CommandLineApp::GetApp()->DoEndian(s32);
//	return &m_s32;
//}


//AudioS32Convertor* AudioS32Convertor::operator =(S32 s32)
//{
//	m_s32 = CommandLineApp::GetApp()->DoEndian(s32);
//	return this;
//}


//AudioS32Convertor AudioS32Convertor::operator =(const Audio::AudioS32Convertor audioS32Convertor)
//{
//	memcpy(&m_s32, &audioS32Convertor, sizeof(S32));
//
//	return *(this);
//}


//bool AudioS32Convertor::operator ==(AudioS32Convertor audioS32Convertor)
//{
//	return (m_s32 == audioS32Convertor.m_s32);
//}


AudioF32Convertor::AudioF32Convertor()
{
}


AudioF32Convertor::AudioF32Convertor(F32 f32)
{
	m_f32 = CommandLineApp::GetApp()->DoEndian(f32);
}


F32* AudioF32Convertor::operator =(F32 f32)
{
	m_f32 = CommandLineApp::GetApp()->DoEndian(f32);
	return &m_f32;
}


bool AudioF32Convertor::operator ==(AudioF32Convertor audioF32Convertor)
{
	return (m_f32 == audioF32Convertor.m_f32);
}


bool AudioF32Convertor::operator <=(F32 f32)
{
	return (CommandLineApp::GetApp()->DoEndian(m_f32) <= f32);
}


AudioF32Convertor::operator F32 ()
{
	return CommandLineApp::GetApp()->DoEndian(m_f32);
}


AudioS16Convertor::AudioS16Convertor()
{
}


AudioS16Convertor::AudioS16Convertor(S16 s16)
{
	m_s16 = s16;
}


S16* AudioS16Convertor::operator =(S16 s16)
{
	m_s16 = CommandLineApp::GetApp()->DoEndian(s16);
	return &m_s16;
}


AudioS16Convertor::operator S16()
{
	return CommandLineApp::GetApp()->DoEndian(m_s16);
}


AudioU16Convertor::AudioU16Convertor()
{
}


AudioU16Convertor::AudioU16Convertor(U16 u16)
{
	m_u16 = CommandLineApp::GetApp()->DoEndian(u16);
}


U16* AudioU16Convertor::operator =(U16 u16)
{
	m_u16 = CommandLineApp::GetApp()->DoEndian(u16);
	return &m_u16;
}


bool AudioU16Convertor::operator ==(AudioU16Convertor audioU16Convertor)
{
	return (m_u16 == audioU16Convertor.m_u16);
}


U16 AudioU16Convertor::operator *(U16 u16Operand)
{
	return u16Operand * CommandLineApp::GetApp()->DoEndian(m_u16);
}


bool AudioU16Convertor::operator <(U16 u16Operand)
{
	return (CommandLineApp::GetApp()->DoEndian(m_u16) < u16Operand);
}


void AudioU16Convertor::operator ++(int)
{
	m_u16 = CommandLineApp::GetApp()->DoEndian((U16)(CommandLineApp::GetApp()->DoEndian(m_u16) + 1));
}


AudioU16Convertor::operator AudioS32Convertor()
{
	return CommandLineApp::GetApp()->DoEndian((S32)(CommandLineApp::GetApp()->DoEndian(m_u16)));
}
