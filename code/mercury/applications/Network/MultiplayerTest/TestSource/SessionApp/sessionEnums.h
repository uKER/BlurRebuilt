#ifndef NETGAMEENUMS_H
#define NETGAMEENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netState/stateEnums.h" 

#define CASE_PER_CLIENT(X) case X##_##0: case X##_##1: case X##_##2: case X##_##3: case X##_##4: case X##_##5: case X##_##6: case X##_##7: case X##_##8: case X##_##9: case X##_##10: case X##_##11: case X##_##12: case X##_##13: case X##_##14: case X##_##15: case X##_##16: case X##_##17: case X##_##18: case X##_##19: case X##_##20: case X##_##21: case X##_##22: case X##_##23:
#define ENUM_PER_CLIENT(X) X##_##0, X##_##1, X##_##2, X##_##3, X##_##4, X##_##5, X##_##6, X##_##7, X##_##8, X##_##9, X##_##10, X##_##11, X##_##12, X##_##13, X##_##14, X##_##15, X##_##16, X##_##17, X##_##18, X##_##19, X##_##20, X##_##21, X##_##22, X##_##23,

enum E_NetEnt
{
	E_NetEnt_None = E_NetEnt_Engine_Count+1,
	E_NetEnt_HostSessionState,
	ENUM_PER_CLIENT(E_NetEnt_PeerSessionState)

};

#endif //NETGAMEENUMS_H