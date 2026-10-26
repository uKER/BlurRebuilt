#ifndef GAMEFLOW_MESSAGES_H
#define GAMEFLOW_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

enum E_GameflowMessage
{
	E_GameflowMessage_StartGame = FOURCC_HI20('MGGF'),
	E_GameflowMessage_QuitLevel,
	E_GameflowMessage_Pause,
	E_GameflowMessage_Unpause,
	E_GameflowMessage_SorceryReloaded
};


} // namespace Game

#endif // GAMEFLOW_MESSAGES_H
