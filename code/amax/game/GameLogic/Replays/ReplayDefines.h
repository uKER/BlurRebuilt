#ifndef REPLAY_DEFINES_H
#define REPLAY_DEFINES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

static const int	REPLAY_RACETIME_SCALER	=	10000;		//	Scaler used to multiply race time before casting as int to keep precision
static const float	REPLAY_START_TIME		=	0.0f;		//	Time into gameplay that we start recording (avoids car-settling when we spawn actors)

} // namespace Game


#endif // REPLAY_DEFINES_H