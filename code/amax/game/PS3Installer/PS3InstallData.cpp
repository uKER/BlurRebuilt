#if 1
#include <stdafx.h>
#include <Core/Threads/Atomic.h>
#include <Framework/Application/CoreSystems.h>
#include <cell/l10n.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Game/UILocaliser.h>
#include <System/time/Time.h>
#include <system/time/scopedtimer.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sys/paths.h>
#include <PakFs/PakFs.h>
#include <PakFs/PakHelpers.h>
#include "PS3InstallData.h"
#include <ui/Graphics/Localisation/Loc.h>
#include <GameSystems.h>
#include <Game\Achievements\AchievementSystem.h>

const U32 gPS3_HDD_FULL_INDEX[] = 
{
// This is a table of offsets into the table below to locate
// the appropriate language
	13,		// English
	176,	// French
	362,	// German
	563,	// Italian
	734,	// Japanese
	825,	// Polish
	977,	// Russian
	1123	// Spanish
}; 

const WCHAR gPS3_HDD_FULL_TEXT[]=
{
	0xFEFF, 0x0045,  0x004E, 0x0047,  0x004C, 0x0049,  0x0053, 0x0048,	//  [...E.N.G.L.I.S.H]  00000000- 
	0x0020, 0x0020,  0x0020, 0x0020,  0x003A, 0x0054,  0x0068, 0x0065,	//  [. . . . .:.T.h.e]  00000001- 
	0x0072, 0x0065,  0x0020, 0x0069,  0x0073, 0x0020,  0x006E, 0x006F,	//  [.r.e. .i.s. .n.o]  00000002- 
	0x0074, 0x0020,  0x0065, 0x006E,  0x006F, 0x0075,  0x0067, 0x0068,	//  [.t. .e.n.o.u.g.h]  00000003- 
	0x0020, 0x0061,  0x0076, 0x0061,  0x0069, 0x006C,  0x0061, 0x0062,	//  [. .a.v.a.i.l.a.b]  00000004- 
	0x006C, 0x0065,  0x0020, 0x0073,  0x0070, 0x0061,  0x0063, 0x0065,	//  [.l.e. .s.p.a.c.e]  00000005- 
	0x0020, 0x0069,  0x006E, 0x0020,  0x0074, 0x0068,  0x0065, 0x0020,	//  [. .i.n. .t.h.e. ]  00000006- 
	0x0048, 0x0044,  0x0044, 0x002E,  0x0020, 0x0054,  0x006F, 0x0020,	//  [.H.D.D... .T.o. ]  00000007- 
	0x0070, 0x006C,  0x0061, 0x0079,  0x0020, 0x0074,  0x0068, 0x0065,	//  [.p.l.a.y. .t.h.e]  00000008- 
	0x0020, 0x0067,  0x0061, 0x006D,  0x0065, 0x002C,  0x0020, 0x0061,	//  [. .g.a.m.e.,. .a]  00000009- 
	0x0074, 0x0020,  0x006C, 0x0065,  0x0061, 0x0073,  0x0074, 0x0020,	//  [.t. .l.e.a.s.t. ]  0000000A- 
	0x0025, 0x0064,  0x004D, 0x0042,  0x0020, 0x006D,  0x006F, 0x0072,	//  [.%.d.M.B. .m.o.r]  0000000B- 
	0x0065, 0x0020,  0x0073, 0x0070,  0x0061, 0x0063,  0x0065, 0x0020,	//  [.e. .s.p.a.c.e. ]  0000000C- 
	0x0069, 0x0073,  0x0020, 0x0072,  0x0065, 0x0071,  0x0075, 0x0069,	//  [.i.s. .r.e.q.u.i]  0000000D- 
	0x0072, 0x0065,  0x0064, 0x002E,  0x0020, 0x0045,  0x0078, 0x0069,	//  [.r.e.d... .E.x.i]  0000000E- 
	0x0074, 0x0020,  0x0074, 0x0068,  0x0065, 0x0020,  0x0067, 0x0061,	//  [.t. .t.h.e. .g.a]  0000000F- 
																								 
	0x006D, 0x0065,  0x0020, 0x0061,  0x006E, 0x0064,  0x0020, 0x006F,	//  [.m.e. .a.n.d. .o]  00000010- 
	0x0062, 0x0074,  0x0061, 0x0069,  0x006E, 0x0020,  0x0074, 0x0068,	//  [.b.t.a.i.n. .t.h]  00000011- 
	0x0065, 0x0020,  0x006E, 0x0065,  0x0063, 0x0065,  0x0073, 0x0073,	//  [.e. .n.e.c.e.s.s]  00000012- 
	0x0061, 0x0072,  0x0079, 0x0020,  0x0073, 0x0070,  0x0061, 0x0063,	//  [.a.r.y. .s.p.a.c]  00000013- 
	0x0065, 0x002E,  0x000D, 0x0000,  0x0046, 0x0052,  0x0045, 0x004E,	//  [.e.......F.R.E.N]  00000014- 
	0x0043, 0x0048,  0x0020, 0x0020,  0x0020, 0x0020,  0x0020, 0x003A,	//  [.C.H. . . . . .:]  00000015- 
	0x0050, 0x0061,  0x0073, 0x0020,  0x0061, 0x0073,  0x0073, 0x0065,	//  [.P.a.s. .a.s.s.e]  00000016- 
	0x007A, 0x0020,  0x0064, 0x0027,  0x0065, 0x0073,  0x0070, 0x0061,	//  [.z. .d.'.e.s.p.a]  00000017- 
	0x0063, 0x0065,  0x0020, 0x006C,  0x0069, 0x0062,  0x0072, 0x0065,	//  [.c.e. .l.i.b.r.e]  00000018- 
	0x0020, 0x0073,  0x0075, 0x0072,  0x0020, 0x006C,  0x0065, 0x0020,	//  [. .s.u.r. .l.e. ]  00000019- 
	0x0064, 0x0069,  0x0073, 0x0071,  0x0075, 0x0065,  0x0020, 0x0064,	//  [.d.i.s.q.u.e. .d]  0000001A- 
	0x0075, 0x0072,  0x002E, 0x0020,  0x0050, 0x006F,  0x0075, 0x0072,	//  [.u.r... .P.o.u.r]  0000001B- 
	0x0020, 0x006A,  0x006F, 0x0075,  0x0065, 0x0072,  0x0020, 0x00E0,	//  [. .j.o.u.e.r. ..]  0000001C- 
	0x0020, 0x0042,  0x006C, 0x0075,  0x0072, 0x002C,  0x0020, 0x0061,	//  [. .B.l.u.r.,. .a]  0000001D- 
	0x0075, 0x0020,  0x006D, 0x006F,  0x0069, 0x006E,  0x0073, 0x0020,	//  [.u. .m.o.i.n.s. ]  0000001E- 
	0x0025, 0x0064,  0x004D, 0x0042,  0x0020, 0x0064,  0x0027, 0x0065,	//  [.%.d.M.B. .d.'.e]  0000001F- 
																								 
	0x0073, 0x0070,  0x0061, 0x0063,  0x0065, 0x0020,  0x006C, 0x0069,	//  [.s.p.a.c.e. .l.i]  00000020- 
	0x0062, 0x0072,  0x0065, 0x0020,  0x0073, 0x006F,  0x006E, 0x0074,	//  [.b.r.e. .s.o.n.t]  00000021- 
	0x0020, 0x006E,  0x00E9, 0x0063,  0x0065, 0x0073,  0x0073, 0x0061,	//  [. .n...c.e.s.s.a]  00000022- 
	0x0069, 0x0072,  0x0065, 0x0073,  0x002E, 0x0020,  0x0051, 0x0075,	//  [.i.r.e.s... .Q.u]  00000023- 
	0x0069, 0x0074,  0x0074, 0x0065,  0x007A, 0x0020,  0x006C, 0x0065,	//  [.i.t.t.e.z. .l.e]  00000024- 
	0x0020, 0x006A,  0x0065, 0x0075,  0x0020, 0x0065,  0x0074, 0x0020,	//  [. .j.e.u. .e.t. ]  00000025- 
	0x006C, 0x0069,  0x0062, 0x00E9,  0x0072, 0x0065,  0x007A, 0x0020,	//  [.l.i.b...r.e.z. ]  00000026- 
	0x0064, 0x0065,  0x0020, 0x006C,  0x0027, 0x0065,  0x0073, 0x0070,	//  [.d.e. .l.'.e.s.p]  00000027- 
	0x0061, 0x0063,  0x0065, 0x0020,  0x006C, 0x0069,  0x0062, 0x0072,	//  [.a.c.e. .l.i.b.r]  00000028- 
	0x0065, 0x0020,  0x0073, 0x0075,  0x0072, 0x0020,  0x006C, 0x0065,	//  [.e. .s.u.r. .l.e]  00000029- 
	0x0020, 0x0064,  0x0069, 0x0073,  0x0071, 0x0075,  0x0065, 0x0020,	//  [. .d.i.s.q.u.e. ]  0000002A- 
	0x0064, 0x0075,  0x0072, 0x002E,  0x0000, 0x0000,  0x0047, 0x0045,	//  [.d.u.r.......G.E]  0000002B- 
	0x0052, 0x004D,  0x0041, 0x004E,  0x0020, 0x0020,  0x0020, 0x0020,	//  [.R.M.A.N. . . . ]  0000002C- 
	0x0020, 0x003A,  0x004E, 0x0069,  0x0063, 0x0068,  0x0074, 0x0020,	//  [. .:.N.i.c.h.t. ]  0000002D- 
	0x0067, 0x0065,  0x006E, 0x0075,  0x0067, 0x0020,  0x0066, 0x0072,	//  [.g.e.n.u.g. .f.r]  0000002E- 
	0x0065, 0x0069,  0x0065, 0x0072,  0x0020, 0x0053,  0x0070, 0x0065,	//  [.e.i.e.r. .S.p.e]  0000002F- 
																								 
	0x0069, 0x0063,  0x0068, 0x0065,  0x0072, 0x0070,  0x006C, 0x0061,	//  [.i.c.h.e.r.p.l.a]  00000030- 
	0x0074, 0x007A,  0x0020, 0x0061,  0x0075, 0x0066,  0x0020, 0x0064,	//  [.t.z. .a.u.f. .d]  00000031- 
	0x0065, 0x0072,  0x0020, 0x0046,  0x0065, 0x0073,  0x0074, 0x0070,	//  [.e.r. .F.e.s.t.p]  00000032- 
	0x006C, 0x0061,  0x0074, 0x0074,  0x0065, 0x002E,  0x0020, 0x005A,	//  [.l.a.t.t.e... .Z]  00000033- 
	0x0075, 0x006D,  0x0020, 0x0053,  0x0070, 0x0069,  0x0065, 0x006C,	//  [.u.m. .S.p.i.e.l]  00000034- 
	0x0065, 0x006E,  0x0020, 0x0077,  0x0065, 0x0072,  0x0064, 0x0065,	//  [.e.n. .w.e.r.d.e]  00000035- 
	0x006E, 0x0020,  0x006D, 0x0069,  0x006E, 0x0064,  0x0065, 0x0073,	//  [.n. .m.i.n.d.e.s]  00000036- 
	0x0074, 0x0065,  0x006E, 0x0073,  0x0020, 0x0025,  0x0064, 0x0020,	//  [.t.e.n.s. .%.d. ]  00000037- 
	0x004D, 0x0042,  0x0020, 0x006D,  0x0065, 0x0068,  0x0072, 0x0020,	//  [.M.B. .m.e.h.r. ]  00000038- 
	0x0066, 0x0072,  0x0065, 0x0069,  0x0065, 0x0072,  0x0020, 0x0053,	//  [.f.r.e.i.e.r. .S]  00000039- 
	0x0070, 0x0065,  0x0069, 0x0063,  0x0068, 0x0065,  0x0072, 0x0070,	//  [.p.e.i.c.h.e.r.p]  0000003A- 
	0x006C, 0x0061,  0x0074, 0x007A,  0x0020, 0x0062,  0x0065, 0x006E,	//  [.l.a.t.z. .b.e.n]  0000003B- 
	0x00F6, 0x0074,  0x0069, 0x0067,  0x0074, 0x002E,  0x0020, 0x0042,	//  [...t.i.g.t... .B]  0000003C- 
	0x0065, 0x0065,  0x006E, 0x0064,  0x0065, 0x0020,  0x0064, 0x0061,	//  [.e.e.n.d.e. .d.a]  0000003D- 
	0x0073, 0x0020,  0x0053, 0x0070,  0x0069, 0x0065,  0x006C, 0x0020,	//  [.s. .S.p.i.e.l. ]  0000003E- 
	0x0075, 0x006E,  0x0064, 0x0020,  0x0073, 0x006F,  0x0072, 0x0067,	//  [.u.n.d. .s.o.r.g]  0000003F- 
																								 
	0x0065, 0x0020,  0x0066, 0x00FC,  0x0072, 0x0020,  0x0064, 0x0065,	//  [.e. .f...r. .d.e]  00000040- 
	0x006E, 0x0020,  0x0062, 0x0065,  0x006E, 0x00F6,  0x0074, 0x0069,	//  [.n. .b.e.n...t.i]  00000041- 
	0x0067, 0x0074,  0x0065, 0x006E,  0x0020, 0x0053,  0x0070, 0x0065,	//  [.g.t.e.n. .S.p.e]  00000042- 
	0x0069, 0x0063,  0x0068, 0x0065,  0x0072, 0x0070,  0x006C, 0x0061,	//  [.i.c.h.e.r.p.l.a]  00000043- 
	0x0074, 0x007A,  0x002E, 0x0000,  0x0000, 0x0000,  0x0000, 0x0049,	//  [.t.z...........I]  00000044- 
	0x0054, 0x0041,  0x004C, 0x0049,  0x0041, 0x004E,  0x0020, 0x0020,	//  [.T.A.L.I.A.N. . ]  00000045- 
	0x0020, 0x0020,  0x003A, 0x0053,  0x0070, 0x0061,  0x007A, 0x0069,	//  [. . .:.S.p.a.z.i]  00000046- 
	0x006F, 0x0020,  0x006C, 0x0069,  0x0062, 0x0065,  0x0072, 0x006F,	//  [.o. .l.i.b.e.r.o]  00000047- 
	0x0020, 0x0069,  0x006E, 0x0073,  0x0075, 0x0066,  0x0066, 0x0069,	//  [. .i.n.s.u.f.f.i]  00000048- 
	0x0063, 0x0069,  0x0065, 0x006E,  0x0074, 0x0065,  0x0020, 0x0073,	//  [.c.i.e.n.t.e. .s]  00000049- 
	0x0075, 0x006C,  0x0020, 0x0064,  0x0069, 0x0073,  0x0063, 0x006F,	//  [.u.l. .d.i.s.c.o]  0000004A- 
	0x0020, 0x0066,  0x0069, 0x0073,  0x0073, 0x006F,  0x002E, 0x0020,	//  [. .f.i.s.s.o... ]  0000004B- 
	0x0050, 0x0065,  0x0072, 0x0020,  0x0067, 0x0069,  0x006F, 0x0063,	//  [.P.e.r. .g.i.o.c]  0000004C- 
	0x0061, 0x0072,  0x0065, 0x002C,  0x0020, 0x0073,  0x006F, 0x006E,	//  [.a.r.e.,. .s.o.n]  0000004D- 
	0x006F, 0x0020,  0x006E, 0x0065,  0x0063, 0x0065,  0x0073, 0x0073,	//  [.o. .n.e.c.e.s.s]  0000004E- 
	0x0061, 0x0072,  0x0069, 0x0020,  0x0061, 0x006C,  0x006D, 0x0065,	//  [.a.r.i. .a.l.m.e]  0000004F- 
																								 
	0x006E, 0x006F,  0x0020, 0x0075,  0x006C, 0x0074,  0x0065, 0x0072,	//  [.n.o. .u.l.t.e.r]  00000050- 
	0x0069, 0x006F,  0x0072, 0x0069,  0x0020, 0x0025,  0x0064, 0x0020,	//  [.i.o.r.i. .%.d. ]  00000051- 
	0x004D, 0x0042,  0x0020, 0x0064,  0x0069, 0x0020,  0x0073, 0x0070,	//  [.M.B. .d.i. .s.p]  00000052- 
	0x0061, 0x007A,  0x0069, 0x006F,  0x002E, 0x0020,  0x0055, 0x0073,	//  [.a.z.i.o... .U.s]  00000053- 
	0x0063, 0x0069,  0x0072, 0x0065,  0x0020, 0x0064,  0x0061, 0x006C,	//  [.c.i.r.e. .d.a.l]  00000054- 
	0x0020, 0x0067,  0x0069, 0x006F,  0x0063, 0x006F,  0x0020, 0x0065,	//  [. .g.i.o.c.o. .e]  00000055- 
	0x0020, 0x006C,  0x0069, 0x0062,  0x0065, 0x0072,  0x0061, 0x0072,	//  [. .l.i.b.e.r.a.r]  00000056- 
	0x0065, 0x0020,  0x006C, 0x006F,  0x0020, 0x0073,  0x0070, 0x0061,	//  [.e. .l.o. .s.p.a]  00000057- 
	0x007A, 0x0069,  0x006F, 0x0020,  0x006E, 0x0065,  0x0063, 0x0065,	//  [.z.i.o. .n.e.c.e]  00000058- 
	0x0073, 0x0073,  0x0061, 0x0072,  0x0069, 0x006F,  0x002E, 0x000A,	//  [.s.s.a.r.i.o....]  00000059- 
	0x0000, 0x0000,  0x004A, 0x0041,  0x0050, 0x0041,  0x004E, 0x0045,	//  [.....J.A.P.A.N.E]  0000005A- 
	0x0053, 0x0045,  0x0020, 0x0020,  0x0020, 0x003A,  0x0048, 0x0044,	//  [.S.E. . . .:.H.D]  0000005B- 
	0x0044, 0x306B,  0x5341, 0x5206,  0x306A, 0x7A7A,  0x304D, 0x5BB9,	//  [.D0kSAR.0jzz0M[.]  0000005C- 
	0x91CF, 0x304C,  0x3042, 0x308A,  0x307E, 0x305B,  0x3093, 0x3002,	//  [..0L0B0.0~0[0.0.]  0000005D- 
	0x30B2, 0x30FC,  0x30E0, 0x306E,  0x30D7, 0x30EC,  0x30A4, 0x306B,	//  [0.0.0.0n0.0.0.0k]  0000005E- 
	0x306F, 0x6700,  0x4F4E, 0x3067,  0x3082, 0x3042,  0x3068, 0x0025,	//  [0og.ON0g0.0B0h.%]  0000005F- 
																								 
	0x0064, 0x004D,  0x0042, 0x306E,  0x7A7A, 0x304D,  0x5BB9, 0x91CF,	//  [.d.M.B0nzz0M[...]  00000060- 
	0x304C, 0x5FC5,  0x8981, 0x3067,  0x3059, 0x3002,  0x4E00, 0x65E6,	//  [0L_...0g0Y0.N.e.]  00000061- 
	0x30B2, 0x30FC,  0x30E0, 0x3092,  0x7D42, 0x4E86,  0x3057, 0x3001,	//  [0.0.0.0.}BN.0W0.]  00000062- 
	0x5FC5, 0x8981,  0x306A, 0x7A7A,  0x304D, 0x5BB9,  0x91CF, 0x3092,	//  [_...0jzz0M[...0.]  00000063- 
	0x78BA, 0x4FDD,  0x3057, 0x3066,  0x304F, 0x3060,  0x3055, 0x3044,	//  [x.O.0W0f0O0`0U0D]  00000064- 
	0x3002, 0x0000,  0x0000, 0x0000,  0x0000, 0x0050,  0x004F, 0x004C,	//  [0..........P.O.L]  00000065- 
	0x0049, 0x0053,  0x0048, 0x0020,  0x0020, 0x0020,  0x0020, 0x0020,	//  [.I.S.H. . . . . ]  00000066- 
	0x003A, 0x004E,  0x0061, 0x0020,  0x0064, 0x0079,  0x0073, 0x006B,	//  [.:.N.a. .d.y.s.k]  00000067- 
	0x0075, 0x0020,  0x0074, 0x0077,  0x0061, 0x0072,  0x0064, 0x0079,	//  [.u. .t.w.a.r.d.y]  00000068- 
	0x006D, 0x0020,  0x006A, 0x0065,  0x0073, 0x0074,  0x0020, 0x007A,	//  [.m. .j.e.s.t. .z]  00000069- 
	0x0061, 0x0020,  0x006D, 0x0061,  0x0142, 0x006F,  0x0020, 0x006D,	//  [.a. .m.a.B.o. .m]  0000006A- 
	0x0069, 0x0065,  0x006A, 0x0073,  0x0063, 0x0061,  0x002E, 0x0020,	//  [.i.e.j.s.c.a... ]  0000006B- 
	0x0050, 0x006F,  0x0074, 0x0072,  0x007A, 0x0065,  0x0062, 0x0075,	//  [.P.o.t.r.z.e.b.u]  0000006C- 
	0x006A, 0x0065,  0x0073, 0x007A,  0x0020, 0x0063,  0x006F, 0x0020,	//  [.j.e.s.z. .c.o. ]  0000006D- 
	0x006E, 0x0061,  0x006A, 0x006D,  0x006E, 0x0069,  0x0065, 0x006A,	//  [.n.a.j.m.n.i.e.j]  0000006E- 
	0x0020, 0x0025,  0x0064, 0x004D,  0x0042, 0x002E,  0x0020, 0x0057,	//  [. .%.d.M.B... .W]  0000006F- 
																								 
	0x0079, 0x006A,  0x0064, 0x017A,  0x0020, 0x007A,  0x0020, 0x0067,	//  [.y.j.d.z. .z. .g]  00000070- 
	0x0072, 0x0079,  0x0020, 0x0069,  0x0020, 0x0077,  0x0072, 0x00F3,	//  [.r.y. .i. .w.r..]  00000071- 
	0x0107, 0x0020,  0x0064, 0x006F,  0x0020, 0x006E,  0x0069, 0x0065,	//  [... .d.o. .n.i.e]  00000072- 
	0x006A, 0x002C,  0x0020, 0x0067,  0x0064, 0x0079,  0x0020, 0x007A,	//  [.j.,. .g.d.y. .z]  00000073- 
	0x0077, 0x006F,  0x006C, 0x006E,  0x0069, 0x0073,  0x007A, 0x0020,	//  [.w.o.l.n.i.s.z. ]  00000074- 
	0x006D, 0x0069,  0x0065, 0x006A,  0x0073, 0x0063,  0x0065, 0x0020,	//  [.m.i.e.j.s.c.e. ]  00000075- 
	0x006E, 0x0061,  0x0020, 0x0064,  0x0079, 0x0073,  0x006B, 0x0075,	//  [.n.a. .d.y.s.k.u]  00000076- 
	0x0020, 0x0074,  0x0077, 0x0061,  0x0072, 0x0064,  0x0079, 0x006D,	//  [. .t.w.a.r.d.y.m]  00000077- 
	0x002E, 0x0000,  0x0000, 0x0000,  0x0000, 0x0052,  0x0055, 0x0053,	//  [...........R.U.S]  00000078- 
	0x0053, 0x0049,  0x0041, 0x004E,  0x0020, 0x0020,  0x0020, 0x0020,	//  [.S.I.A.N. . . . ]  00000079- 
	0x003A, 0x041D,  0x0435, 0x0434,  0x043E, 0x0441,  0x0442, 0x0430,	//  [.:...5.4.>.A.B.0]  0000007A- 
	0x0442, 0x043E,  0x0447, 0x043D,  0x043E, 0x0020,  0x0441, 0x0432,	//  [.B.>.G.=.>. .A.2]  0000007B- 
	0x043E, 0x0431,  0x043E, 0x0434,  0x043D, 0x043E,  0x0433, 0x043E,	//  [.>.1.>.4.=.>.3.>]  0000007C- 
	0x0020, 0x043C,  0x0435, 0x0441,  0x0442, 0x0430,  0x0020, 0x043D,	//  [. .<.5.A.B.0. .=]  0000007D- 
	0x0430, 0x0020,  0x0436, 0x0435,  0x0441, 0x0442,  0x043A, 0x043E,	//  [.0. .6.5.A.B.:.>]  0000007E- 
	0x043C, 0x0020,  0x0434, 0x0438,  0x0441, 0x043A,  0x0435, 0x002E,	//  [.<. .4.8.A.:.5..]  0000007F- 
																								 
	0x0020, 0x0414,  0x043B, 0x044F,  0x0020, 0x0438,  0x0433, 0x0440,	//  [. ...;.O. .8.3.@]  00000080- 
	0x044B, 0x0020,  0x0442, 0x0440,  0x0435, 0x0431,  0x0443, 0x0435,	//  [.K. .B.@.5.1.C.5]  00000081- 
	0x0442, 0x0441,  0x044F, 0x0020,  0x043A, 0x0430,  0x043A, 0x0020,	//  [.B.A.O. .:.0.:. ]  00000082- 
	0x043C, 0x0438,  0x043D, 0x0438,  0x043C, 0x0443,  0x043C, 0x0020,	//  [.<.8.=.8.<.C.<. ]  00000083- 
	0x0025, 0x0064,  0x0020, 0x041C,  0x0431, 0x002E,  0x0020, 0x0417,	//  [.%.d. ...1... ..]  00000084- 
	0x0430, 0x043A,  0x0440, 0x043E,  0x0439, 0x0442,  0x0435, 0x0020,	//  [.0.:.@.>.9.B.5. ]  00000085- 
	0x0438, 0x0433,  0x0440, 0x0443,  0x0020, 0x0438,  0x0020, 0x043E,	//  [.8.3.@.C. .8. .>]  00000086- 
	0x0441, 0x0432,  0x043E, 0x0431,  0x043E, 0x0434,  0x0438, 0x0442,	//  [.A.2.>.1.>.4.8.B]  00000087- 
	0x0435, 0x0020,  0x043D, 0x0435,  0x043E, 0x0431,  0x0445, 0x043E,	//  [.5. .=.5.>.1.E.>]  00000088- 
	0x0434, 0x0438,  0x043C, 0x043E,  0x0435, 0x0020,  0x043C, 0x0435,	//  [.4.8.<.>.5. .<.5]  00000089- 
	0x0441, 0x0442,  0x043E, 0x002E,  0x0020, 0x0000,  0x0000, 0x0053,	//  [.A.B.>... .....S]  0000008A- 
	0x0050, 0x0041,  0x004E, 0x0049,  0x0053, 0x0048,  0x0020, 0x0020,	//  [.P.A.N.I.S.H. . ]  0000008B- 
	0x0020, 0x0020,  0x003A, 0x004E,  0x006F, 0x0020,  0x0068, 0x0061,	//  [. . .:.N.o. .h.a]  0000008C- 
	0x0079, 0x0020,  0x0065, 0x0073,  0x0070, 0x0061,  0x0063, 0x0069,	//  [.y. .e.s.p.a.c.i]  0000008D- 
	0x006F, 0x0020,  0x006C, 0x0069,  0x0062, 0x0072,  0x0065, 0x0020,	//  [.o. .l.i.b.r.e. ]  0000008E- 
	0x0073, 0x0075,  0x0066, 0x0069,  0x0063, 0x0069,  0x0065, 0x006E,	//  [.s.u.f.i.c.i.e.n]  0000008F- 
																								 
	0x0074, 0x0065,  0x0020, 0x0065,  0x006E, 0x0020,  0x0065, 0x006C,	//  [.t.e. .e.n. .e.l]  00000090- 
	0x0020, 0x0064,  0x0069, 0x0073,  0x0063, 0x006F,  0x0020, 0x0064,	//  [. .d.i.s.c.o. .d]  00000091- 
	0x0075, 0x0072,  0x006F, 0x002E,  0x0020, 0x0050,  0x0061, 0x0072,	//  [.u.r.o... .P.a.r]  00000092- 
	0x0061, 0x0020,  0x006A, 0x0075,  0x0067, 0x0061,  0x0072, 0x0020,	//  [.a. .j.u.g.a.r. ]  00000093- 
	0x006E, 0x0065,  0x0063, 0x0065,  0x0073, 0x0069,  0x0074, 0x0061,	//  [.n.e.c.e.s.i.t.a]  00000094- 
	0x0073, 0x0020,  0x0061, 0x006C,  0x0020, 0x006D,  0x0065, 0x006E,	//  [.s. .a.l. .m.e.n]  00000095- 
	0x006F, 0x0073,  0x0020, 0x0025,  0x0064, 0x0020,  0x004D, 0x0042,	//  [.o.s. .%.d. .M.B]  00000096- 
	0x0020, 0x006D,  0x00E1, 0x0073,  0x0020, 0x0064,  0x0065, 0x0020,	//  [. .m...s. .d.e. ]  00000097- 
	0x0065, 0x0073,  0x0070, 0x0061,  0x0063, 0x0069,  0x006F, 0x002E,	//  [.e.s.p.a.c.i.o..]  00000098- 
	0x0020, 0x0053,  0x0061, 0x006C,  0x0020, 0x0064,  0x0065, 0x006C,	//  [. .S.a.l. .d.e.l]  00000099- 
	0x0020, 0x006A,  0x0075, 0x0065,  0x0067, 0x006F,  0x0020, 0x0079,	//  [. .j.u.e.g.o. .y]  0000009A- 
	0x0020, 0x0063,  0x006F, 0x006E,  0x0073, 0x0069,  0x0067, 0x0075,	//  [. .c.o.n.s.i.g.u]  0000009B- 
	0x0065, 0x0020,  0x0065, 0x006C,  0x0020, 0x0065,  0x0073, 0x0070,	//  [.e. .e.l. .e.s.p]  0000009C- 
	0x0061, 0x0063,  0x0069, 0x006F,  0x0020, 0x006E,  0x0065, 0x0063,	//  [.a.c.i.o. .n.e.c]  0000009D- 
	0x0065, 0x0073,  0x0061, 0x0072,  0x0069, 0x006F,  0x002E, 0x000A,	//  [.e.s.a.r.i.o....]  0000009E- 
	0x0000, 0x0000														//  [....            ]  0000009F- 
};

static const WCHAR* GetHDDFullString()
{
	UI::E_UILanguage language = UI::C_UILocaliser::Instance().ReadLanguage( UI::C_UILocaliser::Instance().ReadRegion() );

	const WCHAR* dialogtext = NULL;
	switch( language )
	{
		default:
		case UI::E_UILanguage_English:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 0 ]];
			break;
		case UI::E_UILanguage_French:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 1 ]];
			break;
		case UI::E_UILanguage_German:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 2 ]];
			break;
		case UI::E_UILanguage_Italian:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 3 ]];
			break;
		case UI::E_UILanguage_Japanese:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 4 ]];
			break;
		case UI::E_UILanguage_Polish:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 5 ]];
			break;
		case UI::E_UILanguage_Russian:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 6 ]];
			break;
		case UI::E_UILanguage_Spanish:
			dialogtext = &gPS3_HDD_FULL_TEXT[gPS3_HDD_FULL_INDEX[ 7 ]];
			break;
	}

	return dialogtext;
}

Debug::Bool gDisablePS3Installer("Playstation3.Installer.Disable", false, Debug::Persist());
extern bool gDisablePS3InstallerFromCode; // Used by code to ensure installer disabled - i.e. automation
bool gAbortPS3Installer = false;
//#define INSTALL_DEBUG_INFO
#define DISABLE_ASYNC_HACK

bool IsPS3InstallerDisabled()
{
	return gDisablePS3Installer || gDisablePS3InstallerFromCode;
}

#ifdef DISABLE_ASYNC_HACK
extern Debug::Bool gSystemIoDisableAsyncFile;

//HACK: prevent fios locking kit during copy.
class C_AutoAsyncFileDisable
{
public:
	C_AutoAsyncFileDisable()
	{
		m_previousAsyncFileState = gSystemIoDisableAsyncFile;
		gSystemIoDisableAsyncFile = true;
#ifdef INSTALL_DEBUG_INFO
		printf("Push C_AutoAsyncFileDisable AsyncFile : %s\n", gSystemIoDisableAsyncFile ? "true" : "false");
#endif
	}

	~C_AutoAsyncFileDisable()
	{
		gSystemIoDisableAsyncFile = m_previousAsyncFileState;
#ifdef INSTALL_DEBUG_INFO
		printf("Pop C_AutoAsyncFileDisable AsyncFile : %s\n", gSystemIoDisableAsyncFile ? "true" : "false");
#endif
	}

	bool m_previousAsyncFileState;
};

#endif

//For param.sfo?
#define GAME_DATA_SIZE_FUDGE 3072

// Comment me out to turn off PS3 install data
#define _PS3_INSTALLER_ENABLED

// Buffer Sizes
#define NR_COPY_BUFFERS 64
#define COPY_BUFFER_SIZE (128 * 1024)

// Times are in uSeconds
#define READ_THREAD_SLEEP_TIME 2800
#define WRITE_THREAD_SLEEP_TIME 250000
#define WRITE_THREAD_INTER_BLOCK_SLEEP_TIME 500
// Current Install Procedure
// 1. Copy Icons to create the HDD Game Data.
// 2. Copy The ATVI + Bizarre Movies
// 3. Copy Attract Movie (whilst Playing the Movies ATVI + Bizarre)
// 4. Copy Rest of Data ( Whilst Playing the Attract Movie )

#define _USE_BIZARRE_FILE_SYSTEM_
#define _USE_PACK_FILE_INSTALL_
// Use this to test out of HDD Space with debug
//#define HDD_LOW_SPACE
#define HDD_FREE_AMOUNT_KB (1 * 1024)

typedef struct
{
#ifdef _USE_BIZARRE_FILE_SYSTEM_
	Sys::I_File* pSourceFile;
	Sys::I_File* pDestFile;
#endif
	int				sourceFd;
	int				destFd;
	char*			copyBuffers[ NR_COPY_BUFFERS ];
	volatile U32 	bufferUsedSize[ NR_COPY_BUFFERS ];
	U32 			bufferIndex[ NR_COPY_BUFFERS ];
	U32 			sourceFileSize;
	U32 			bytesRead;
	U32 			bytesWritten;
	U32 			bufferCount;
#ifdef _USE_PACK_FILE_INSTALL_
	Sys::I_FileSystem* fileSystem;
#endif
} COPY_BUFFER_INFO;

MT::Mutex	gReadWriteMutex;

extern MT::Event AudioStreamingEvent;
C_PS3InstallData gGameInstaller;
void CheckForSpaceThread( U64 param );

// Initialise Variables
C_PS3InstallData::C_PS3InstallData()
{
	m_Initialised = false;
	SetInitialised( false );
	m_FinishedCopying = true;
	mCopyProgressBytes = 0;
	mGameState = eGameState_UnInitialised;
	m_CopyInProgress = false;
	mHasCheckedForFreeSpace = false;
	mHasCheckedForFreeSpaceUI = false;
	mNotEnoughFreeSpace = false;
	mDiscEjected = false;
	mDiscIsIn = true;
	mbInstallationComplete = false;
	mUiInitialised = false;
	SetThrottle( 0 );
	mAbort = false;
	mShowInstallMessage = false;

	mpPakFs = NULL;
#ifdef USE_NEW_THINGY_MAJIG
//	mpPakFsCompressed = NULL;
	mpPakFsRaw = NULL;
	mpPakFsCompressedLoc = NULL;
	mpPakFsRawLoc = NULL;
#endif
}

bool C_PS3InstallData::IsCopyInProgress()
{ 
	if (IsPS3InstallerDisabled())
		return false;
	if (mHasCheckedForFreeSpace) 
		return m_CopyInProgress;					
	else 
		return true;	
}
S64	C_PS3InstallData::GetRequiredHDDSpaceKB()
{ 
	if (IsPS3InstallerDisabled())
		return 0;
	return mRequiredHDDSpaceKB;
}

// Remove Items From the Vector & Close The Pak-File
bool C_PS3InstallData::ReleaseResources()
{
	m_FileCopyList.Destroy();
	SAFE_DELETE_PTR(mpPakFs);
#ifdef USE_NEW_THINGY_MAJIG
//	SAFE_DELETE_PTR(mpPakFsCompressed	);
	SAFE_DELETE_PTR(mpPakFsRaw			);
	SAFE_DELETE_PTR(mpPakFsCompressedLoc);
	SAFE_DELETE_PTR(mpPakFsRawLoc		);
#endif
	return true;
}

bool C_PS3InstallData::ShowInstallingMessage()
{ 
	return mShowInstallMessage;
}

// Callback for PS3 System Message  - Insufficient Space
void InsufficientSpaceCallBack(int buttonType,void *userData)
{
	U32* uData = (U32*)(U32)userData;
	switch( buttonType ) 
	{
		case CELL_MSGDIALOG_BUTTON_YES:
		{
			// Processing for positive response ("Ok")
			*uData = C_PS3InstallData::eDIALOG_ACTION_ERROR;
			break;
		}
	}
}

// Get The Singleton - Instance of the Installer
C_PS3InstallData& C_PS3InstallData::Instance()
{
	return gGameInstaller;
}

void C_PS3InstallData::SetThrottle( U32 throttleMs )
{
	if (throttleMs > 2000) throttleMs = 2000;
	mThrottleUs = throttleMs * 1000;
	if (mThrottleUs == 0) mThrottleUs = 1;
}

// Reading Thread Reads from the source (Blu-Ray or /app_home)
void readingThread( U64 param )
{
	COPY_BUFFER_INFO* copyBufferInfo = (COPY_BUFFER_INFO*)param;

	float progressDelta = ( (float)COPY_BUFFER_SIZE / (float)copyBufferInfo->sourceFileSize) * 100.0f;
	float progressTotal = 0.0f;
	U32 prevProgress = 0;
	
#ifdef INSTALL_DEBUG_INFO
	int readThisTime = 0;
	int readThisTotal = 0;
#endif
	while ( copyBufferInfo->bytesRead != copyBufferInfo->sourceFileSize )
	{
		if (C_PS3InstallData::Instance().GetAbort()) break;
		U32 bytesRemaining = (copyBufferInfo->sourceFileSize - copyBufferInfo->bytesRead);
		U32 bytesToRead =  bytesRemaining < COPY_BUFFER_SIZE ? bytesRemaining : COPY_BUFFER_SIZE;
		bool bufferFilled = false;
		while (!bufferFilled)
		{
			if (C_PS3InstallData::Instance().GetAbort()) break;
			for (U32 i = 0 ; i < NR_COPY_BUFFERS ; i++)
			{
				gReadWriteMutex.Wait();
				if ( copyBufferInfo->bufferUsedSize[i] == 0 )
				{
					gReadWriteMutex.Release();
					// Fill it!
#ifdef _USE_BIZARRE_FILE_SYSTEM_
					sys_timer_usleep( C_PS3InstallData::Instance().GetThrottle() );

					copyBufferInfo->pSourceFile->Read( copyBufferInfo->copyBuffers[i], bytesToRead );
					sys_timer_usleep( READ_THREAD_SLEEP_TIME );
#else
					cellFsRead( copyBufferInfo->sourceFd, copyBufferInfo->copyBuffers[i], bytesToRead, NULL );
#endif
					gReadWriteMutex.Wait();
					copyBufferInfo->bytesRead += bytesToRead;
					copyBufferInfo->bufferIndex[i] = copyBufferInfo->bufferCount++;
					copyBufferInfo->bufferUsedSize[i] = bytesToRead;
					bytesRemaining = (copyBufferInfo->sourceFileSize - copyBufferInfo->bytesRead);
					bytesToRead =  bytesRemaining < COPY_BUFFER_SIZE ? bytesRemaining : COPY_BUFFER_SIZE;
					gReadWriteMutex.Release();
					bufferFilled = true;

					//Update global copy progress.
					C_PS3InstallData::Instance().IncrementGameDataCopyProgressBytes(bytesToRead);
#ifdef INSTALL_DEBUG_INFO
					readThisTime++;
					readThisTotal++;
#endif					
				}
				else
				{
					gReadWriteMutex.Release();
					sys_timer_usleep( READ_THREAD_SLEEP_TIME );
				}
			}

#ifdef INSTALL_DEBUG_INFO
			if (bufferFilled)
			{
//				printf("readThisTime %d readThisTotal %d\n",readThisTime,readThisTotal);
				readThisTime = 0;
			}
#endif
			if (C_PS3InstallData::Instance().GetAbort()) break;

		}
		
	}
	sys_ppu_thread_exit(0);
}

// Writing thread to the Game-Data HDD
void writingThread( U64 param )
{
	if (C_PS3InstallData::Instance().GetAbort()) return;
	COPY_BUFFER_INFO* copyBufferInfo = (COPY_BUFFER_INFO*)param;
	U32 writeBufferIndex = 0;
#ifdef INSTALL_DEBUG_INFO
	int writeThisTime = 0;
	int writeThisTotal = 0;
#endif
	while ( copyBufferInfo->bytesWritten != copyBufferInfo->sourceFileSize )
	{
		if (C_PS3InstallData::Instance().GetAbort()) break;
		bool bufferWritten = false;
		for (U32 i = 0 ; i < NR_COPY_BUFFERS; i++)
		{
			if (C_PS3InstallData::Instance().GetAbort()) break;
			gReadWriteMutex.Wait();
			if ((copyBufferInfo->bufferIndex[i] == writeBufferIndex) && (copyBufferInfo->bufferUsedSize[i] > 0))
			{
				gReadWriteMutex.Release();

#ifdef _USE_BIZARRE_FILE_SYSTEM_
				sys_timer_usleep( C_PS3InstallData::Instance().GetThrottle() );

				copyBufferInfo->pDestFile->Write( copyBufferInfo->copyBuffers[i], copyBufferInfo->bufferUsedSize[i] );
				sys_timer_usleep( WRITE_THREAD_INTER_BLOCK_SLEEP_TIME );
#else
				U64 nrWritten = 0;
				int retwrite = cellFsWrite( copyBufferInfo->destFd, copyBufferInfo->copyBuffers[i], copyBufferInfo->bufferUsedSize[i], &nrWritten );
#endif
				gReadWriteMutex.Wait();
				copyBufferInfo->bytesWritten += copyBufferInfo->bufferUsedSize[i];
				copyBufferInfo->bufferUsedSize[i] = 0;
				copyBufferInfo->bufferIndex[i] = 0;
				writeBufferIndex++;
				bufferWritten = true;
#ifdef INSTALL_DEBUG_INFO
				writeThisTime++;
				writeThisTotal++;
#endif
				gReadWriteMutex.Release();
			}
			else
			{
				gReadWriteMutex.Release();
				sys_timer_usleep( WRITE_THREAD_INTER_BLOCK_SLEEP_TIME );
			}
			
		}
		if (C_PS3InstallData::Instance().GetAbort()) break;
		if (!bufferWritten)
		{
			int throttle = C_PS3InstallData::Instance().GetThrottle();
			if (throttle>1) throttle = 10;
			sys_timer_usleep( WRITE_THREAD_SLEEP_TIME * throttle );
		}
#ifdef INSTALL_DEBUG_INFO
		else
		{
//			printf("writeThisTime %d writeThisTotal %d\n",writeThisTime,writeThisTotal);
			writeThisTime = 0;
		}
#endif
	}
	sys_timer_usleep( WRITE_THREAD_SLEEP_TIME );
	sys_ppu_thread_exit(0);
}

void C_PS3InstallData::SignalStartCopy()
{
	m_SignalStart.Signal();
}

void C_PS3InstallData::SignalStartWait()
{
	m_SignalStart.Wait();
}

// Process the list of install files
void copyingThread( U64 param )
{
	bool remountPakLayersFromHDD = false;

// Make sure we're not having more than 1 copying process running
	C_PS3InstallData::Instance().GetGameDataCopyMutex().Wait();
	{
	#ifdef DISABLE_ASYNC_HACK
		C_AutoAsyncFileDisable autoDisableAsync;
	#endif
		using namespace Ut;
		// Wait for movie to start
	//	C_PS3InstallData::Instance().SignalStartWait();
		// Allocate 3MB Container for HDD
		C_PS3InstallData::Instance().SetCopyInProgress( true );
		C_PS3InstallData::Instance().SetFinishedCopying( false );

		unsigned int messageType = CELL_MSGDIALOG_TYPE_PROGRESSBAR_DOUBLE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON | 
									CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_NONE | CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL;

		// Pop Items Off List
		for (S32 x = 0 ; x < C_PS3InstallData::Instance().GetFileList().Size() ; x++)
		{
			while( C_PS3InstallData::Instance().GetFileList()[x].mCopyInProgressFlag) 
			{ 
				if (C_PS3InstallData::Instance().GetAbort()) break;
				sys_timer_usleep(28000); 
			}
		
			if (C_PS3InstallData::Instance().GetFileList()[x].mCopyPendingFlag)
			{
				Sys::C_FilePath sourcefname = C_PS3InstallData::Instance().GetFileList()[x].mSourceFilename;
				Sys::C_FilePath destfname = C_PS3InstallData::Instance().GetFileList()[x].mDestFileName;
				bool remap = C_PS3InstallData::Instance().GetFileList()[x].mRemapFile;

				C_PS3InstallData::Instance().GetFileListItem(x)->SetCopyPendingFlag( false );
				C_PS3InstallData::Instance().GetFileListItem(x)->SetCopyInProgressFlag( true );
				bool copyResult = C_PS3InstallData::Instance().CopyFile( C_PS3InstallData::Instance().GetFileListItem(x)->mFileSystem, sourcefname.Chars(), destfname.Chars() );
	#ifdef INSTALL_DEBUG_INFO
				printf("Copying %s\n",sourcefname.Chars());
	#endif
				C_PS3InstallData::Instance().GetFileListItem(x)->SetCopyInProgressFlag( false );

				// If it's the last in list, let the Audio RIP
				if ( C_PS3InstallData::Instance().GetFileList()[C_PS3InstallData::Instance().GetFileList().Size() -1].mSourceFilename == C_PS3InstallData::Instance().GetFileListItem(x)->mSourceFilename )
				{
					AudioStreamingEvent.Signal();
					C_PS3InstallData::Instance().SignalInstallComplete( true );
					// Free resources
					C_PS3InstallData::Instance().ReleaseResources();

					remountPakLayersFromHDD = true;
				}
			}
		}
		
		C_PS3InstallData::Instance().SetFinishedCopying();
		C_PS3InstallData::Instance().SetCopyInProgress( false );
		C_PS3InstallData::Instance().SetCurrentCopyStatus( C_PS3InstallData::Instance().eCurrentCopy_Idle );

		C_PS3InstallData::Instance().GetGameDataCopyMutex().Release();
	#ifdef INSTALL_DEBUG_INFO
		printf("Finished Copying\n");
	#endif
	}
	if (remountPakLayersFromHDD)
		Game::C_GameSystems::Instance().RemountPakLayers();
	sys_ppu_thread_exit(0);
}

// Force create a directory tree from a path
void C_PS3InstallData::ForceDirectories( const char *DirectoryPath )
{
	// Does the directory exist
	int fd;
	unsigned int pos;
	int dirCount = 0;
	int result = 0;
	char *dir = new char [strlen(DirectoryPath)+1];
	strcpy( dir, DirectoryPath );
	char titleId[10];

	strcpy(titleId,PlatformPS3::g_ps3_titleId);
	
	for(U32 x = 0; x < strlen( titleId ) ; x++)
	{
		if (titleId[x] >= 'A' && titleId[x] <= 'Z') titleId[x] += 0x20;
	}
	char* titlePos = strstr(dir, titleId);
	if (titlePos)
	{
		for(U32 x = 0; x < strlen( titleId ) ; x++)
		{
			if (titlePos[x] >= 'a' && titlePos[x] <= 'z') titlePos[x] -= 0x20;
		}
	}

	for (pos = 0 ; pos < strlen(DirectoryPath) ; pos++)
	{
		if (DirectoryPath[pos] == '/') 
		{
			dirCount++;
			if (dirCount > 2) // "/app_home/"
			{
				CellFsDirent dirEnt;
				std::uint64_t nread = 0;
				dir[pos] = 0;
				result = cellFsOpendir( dir, &fd );
				
				if ( result != CELL_FS_SUCCEEDED)
				{
					result = cellFsReaddir( fd, &dirEnt, &nread );
					if ((result != CELL_FS_SUCCEEDED) || (nread == 0))
					{
						// Make the folder
						if ((result = cellFsMkdir( dir, CELL_FS_DEFAULT_CREATE_MODE_1 )) != CELL_FS_SUCCEEDED)
						{
							//printf("cellFsMkdir failed code %x\n",result);
						}
					}
				}
				else
				{
					cellFsClosedir( fd );
				}
				dir[pos] = '/';
			}
		}
	}
}


// Copy a file, opens the source & dest files, initiates the reading and writing threads and waits on them, then closes the files
bool C_PS3InstallData::CopyFile( Sys::I_FileSystem* sourcefileSystem, const char *sourceName, const char *destName )
{
	if (mAbort) return false;

	Time::C_Time start = Time::GetCurrentTime();

	COPY_BUFFER_INFO copyBufferInfo;
	copyBufferInfo.pSourceFile = NULL;
	copyBufferInfo.pDestFile = NULL;

// If file matches already with one on HDD then don't copy

//	Sys::C_FilePath sourceFullPath = Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/" + sourceName;
	Sys::C_FilePath sourceFullPath = sourcefileSystem->GetWorkingDirectory() + "/" + sourceName;
	if (CheckFiles( sourcefileSystem, sourceFullPath.Chars() , destName )) return false;

	// Make sure the destination directory exists
	ForceDirectories( destName );

	Sys::C_FilePath sourcePath;
	Sys::C_FilePath destPath;
	sourcePath.SetFormatted( sourceName );
	destPath.SetFormatted( destName );

//	printf("PS3Installer:: copying [%s] to [%s]\n",sourceName, destName);
#ifdef _USE_BIZARRE_FILE_SYSTEM_
	copyBufferInfo.pSourceFile = sourcefileSystem->Open( sourcePath, Sys::E_FileAccess_Read );
	if (copyBufferInfo.pSourceFile)
	{
		Sys::C_OpenParams params;
		params.m_BufferSizeHint = 128 * 1024;
		copyBufferInfo.pDestFile = Sys::C_FileSystem::Instance().Open( destPath, Sys::E_FileAccess_Write, params );
	}
	if (!copyBufferInfo.pSourceFile || !copyBufferInfo.pDestFile)
	{
#ifndef CFG_GOLD
	if (!copyBufferInfo.pSourceFile)
	 	printf("PS3Install: Failed to Open %s For Reading\n",sourceName);
	if (!copyBufferInfo.pDestFile)
	 	printf("PS3Install: Failed to Open %s For Writing\n",destName);
	
#endif
		if (copyBufferInfo.pSourceFile) delete copyBufferInfo.pSourceFile;
		if (copyBufferInfo.pDestFile)   delete copyBufferInfo.pDestFile;
		return false;
	}
#else
	int srcret = cellFsOpen( sourceFullPath.Chars(), CELL_FS_O_RDONLY, &copyBufferInfo.sourceFd, NULL, 0 );
	int dstret = cellFsOpen( destName, CELL_FS_O_CREAT | CELL_FS_O_WRONLY, &copyBufferInfo.destFd,NULL, 0 );
	if ( srcret != CELL_FS_SUCCEEDED || dstret != CELL_FS_SUCCEEDED)
	{
		if (srcret == CELL_FS_SUCCEEDED) cellFsClose( copyBufferInfo.sourceFd );
		if (dstret == CELL_FS_SUCCEEDED) cellFsClose( copyBufferInfo.destFd );
#ifndef CFG_GOLD
		printf("Failed to copy %s\n",sourceFullPath.Chars());
#endif
		return false;
	}
#endif
	sys_ppu_thread_t readingThreadHandle;
	sys_ppu_thread_t writingThreadHandle;

	// Create Copy Buffers
	for (U32 i = 0 ; i < NR_COPY_BUFFERS ; i++)
	{
		//copyBufferInfo.copyBuffers[i] = new char[ COPY_BUFFER_SIZE ];	
		copyBufferInfo.copyBuffers[i] = (char*)BizNonTrackedMalloc(COPY_BUFFER_SIZE);
		copyBufferInfo.bufferUsedSize[i] = 0;
		copyBufferInfo.bufferIndex[i] = 0;
	}

#ifdef _USE_BIZARRE_FILE_SYSTEM_
	copyBufferInfo.sourceFileSize = copyBufferInfo.pSourceFile->Length();
#else
	CellFsStat sb;
	cellFsFstat( copyBufferInfo.sourceFd, &sb );
	copyBufferInfo.sourceFileSize = sb.st_size;
#endif
	copyBufferInfo.bytesRead = 0;
	copyBufferInfo.bytesWritten = 0;
	copyBufferInfo.bufferCount = 0;

	// Create Reading Thread
	int ret = sys_ppu_thread_create( &readingThreadHandle, readingThread, (U64)&copyBufferInfo, 1001, 64 * 1024, SYS_PPU_THREAD_CREATE_JOINABLE, "GameData Read Thread" );
	sys_timer_usleep( 15000);
	// Create Writing Thread
	ret = sys_ppu_thread_create( &writingThreadHandle, writingThread, (U64)&copyBufferInfo, 1001, 64 * 1024, SYS_PPU_THREAD_CREATE_JOINABLE, "GameData Write Thread" );

	uint64_t readResult;
	uint64_t writeResult;
	sys_ppu_thread_join( readingThreadHandle, &readResult );
	sys_ppu_thread_join( writingThreadHandle, &writeResult );
	
#ifdef _USE_BIZARRE_FILE_SYSTEM_
	delete copyBufferInfo.pSourceFile;
	delete copyBufferInfo.pDestFile;
#else
	int retcloses = cellFsClose( copyBufferInfo.sourceFd );
	int retclosed = cellFsClose( copyBufferInfo.destFd );
#endif
	for (U32 i = 0 ; i < NR_COPY_BUFFERS ; i++)
	{
		//delete [] copyBufferInfo.copyBuffers[i];
		BizNonTrackedFree(copyBufferInfo.copyBuffers[i]);
	}

#if 1
	CellFsUtimbuf fsUtimbuf;
	Sys::C_FileInfo sourceInfo;
	if (!sourcefileSystem->FileInfo( sourcePath, &sourceInfo ))
	{
#ifdef INSTALL_DEBUG_INFO
		printf("FileInfo Failed %s\n",sourcePath.Chars());
#endif
	}
	
	fsUtimbuf.actime = U64(sourceInfo.m_FileTime) / 1000000L;
	fsUtimbuf.modtime = U64(sourceInfo.m_FileTime) / 1000000L;
	ret = cellFsUtime( destName, &fsUtimbuf);
	if (ret != 0)
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellFsUtime Error %08x\n",ret);
#endif
	}

	
#else
	// Update Time of Destination File
	CellFsUtimbuf fsUtimbuf;
	CellFsStat sourceStat;
	cellFsStat( sourceFullPath.Chars(), &sourceStat );

	fsUtimbuf.actime = sourceStat.st_atime;
	fsUtimbuf.modtime = sourceStat.st_mtime;
	ret = cellFsUtime( destName, &fsUtimbuf);
	// Close Files
#endif

	Time::C_Time end = Time::GetCurrentTime();
	float sizeInMb = (float)(copyBufferInfo.sourceFileSize / (1024.0f * 1024.0f)) ;
	float timeInSec = (float)((float)(end-start) / 1000000.0f);
#ifdef INSTALL_DEBUG_INFO
	printf("copied [%s] [%d] [%8.2f MB/s]\n",sourceName,(U32)(end-start), sizeInMb / timeInSec );
#endif
	return true;
}



void C_PS3InstallData::SetInitialised( bool ok )
{
	m_InitialisedOk = ok;
}

bool C_PS3InstallData::InitialisedOk()
{
	return m_InitialisedOk;
}

bool C_PS3InstallData::HasFinishedCopying()
{
	return m_FinishedCopying;
}

void C_PS3InstallData::SetFinishedCopying( bool value )
{
	m_FinishedCopying = value ;
}

bool C_PS3InstallData::WaitForHDDChecks()
{
// Wait to get the go-ahead
	if (mHasCheckedForFreeSpace == false)
	{
		static sys_ppu_thread_t CheckForSpaceThreadHandle;
		Initialise();
		mFinishedWaitingForSpaceThread = false;
		mHddFreeSpace = mRequiredHDDSpaceKB = mRequiredSizeMB = 0;
		int ret = sys_ppu_thread_create( &CheckForSpaceThreadHandle, CheckForSpaceThread, 0, 1001, 64 * 1024, 0, "GameData CheckHDD Space" );
		
		while ( mHasCheckedForFreeSpace == false ) 
		{
#ifdef INSTALL_DEBUG_INFO
			printf("Waiting For Free Space Check To Complete\n");
#endif
			__sync();
			sys_timer_usleep( 1000 );
		}
	}
// if not enough space - abort copy
	if ( mNotEnoughFreeSpace ) return false;
	return true;
}

// Initiate the copying of the icons and the atvi & bizarre movies
bool C_PS3InstallData::CopyIcons()
{
#ifndef DEMO_VERSION
	if (!WaitForHDDChecks()) return false;
	bool b50Hz = Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ ? true : false;
	// Add Files to List
	AddFileToCopy( "disc_check" );
	AddFileToCopy( "memsaveps3/icon0.png" );
	AddFileToCopy( "memsaveps3/pic1_gamedata.png" );
	mCurrentCopy = eCurrentCopy_IntroMovie;
	C_PS3InstallData::Instance().SetCopyInProgress( false );
	StartCopyThread( true );
#endif
	return false;
}

// Copy the attract movie
bool C_PS3InstallData::CopyAttractMovie()
{
#ifndef DEMO_VERSION
	CopyIcons();
#endif
	return false;
}

bool C_PS3InstallData::InstallFolder( Sys::I_FileSystem* fileSystem, const Sys::C_FilePath& path_parameter )
{
	Sys::C_DirectoryInfo dirInfo;

	const bool success = fileSystem->DirectoryInfo(  path_parameter , &dirInfo);
	if(success)
	{
		for(U32 i = 0; i < dirInfo.NumFiles(); i++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				InstallFolder( fileSystem, fileInfo.m_FileName );
			}
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				Btl::String sourceName;
				
				sourceName = fileInfo.m_FileName.Chars();
				
				AddFileToCopy(sourceName.Chars());
//				printf("FILE,%s,size,%lld\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}		            
		}
	}    
}

// copy the main install data
bool C_PS3InstallData::CopyMainInstallData()
{
#ifndef DEMO_VERSION
	if (!WaitForHDDChecks()) return false;
	bool b50Hz = Display::Device()->GetRefreshRate() == Display::E_RefreshRate_50HZ ? true : false;
	// Add Files to List

	AddFileToCopy( "cache/shaders-speed.pak" );
	AddFileToCopy( "cache/gamedata_boot.pak" );
	AddFileToCopy( "cache/boot_rampak.pak" );
	AddFileToCopy( "cache/boot.pak" );
	AddFileToCopy( "cache/gamedata.pak" );
	AddFileToCopy( "cache/root.pak" );
	AddFileToCopy( "cache/lut_rampak.pak" );
	AddFileToCopy( "cache/root_streaming.pak" );
	
	if (mpPakFs)				
	{
		InstallFolder(mpPakFs, Sys::C_FilePath( "/audio/" ));
		InstallFolder(mpPakFs, Sys::C_FilePath( "/cache/" ));
	}

#ifdef USE_NEW_THINGY_MAJIG
//	if (mpPakFsCompressed)		InstallFolder(mpPakFsCompressed,	Sys::C_FilePath( "/") );
	if (mpPakFsRaw)				InstallFolder(mpPakFsRaw,			Sys::C_FilePath( "/") );
	if (mpPakFsCompressedLoc)	InstallFolder(mpPakFsCompressedLoc,	Sys::C_FilePath( "/") );
	if (mpPakFsRawLoc)			InstallFolder(mpPakFsRawLoc,		Sys::C_FilePath( "/") );
#endif

	mCurrentCopy = eCurrentCopy_MainData;
	C_PS3InstallData::Instance().SetCopyInProgress( true );
	StartCopyThread( false );
#endif

	return false;
}

// Adds a file to the list of files that can be copied
bool C_PS3InstallData::AddFileToList( Sys::I_FileSystem* fileSystem, const char *filename, const char *destfname, bool remapFile, bool isGameData, bool systemFile )
{
	C_PS3InstallDataItem newItem( fileSystem, filename, destfname, remapFile, isGameData, systemFile );
	m_FileCopyList.PushBack( newItem );
	return true;
}

// Adds a file to the list of files to copy
bool C_PS3InstallData::AddFileToCopy( const char *filename )
{
	for (S32 i = 0 ; i < m_FileCopyList.Size() ; i++)
	{
		if (!strcmp( m_FileCopyList[i].mSourceFilename.Chars(), filename ))
		{
			m_FileCopyList[i].mCopyPendingFlag = true;
			return true;
		}
	}
#ifdef INSTALL_DEBUG_INFO
	DebugBreak();
#endif
	return false;
}



bool C_PS3InstallData::StartCopyThread( bool createJoinable )
{
	// Create Reading Thread
	U64 joinFlag = createJoinable ? SYS_PPU_THREAD_CREATE_JOINABLE : 0;
	int ret = sys_ppu_thread_create( &copyingThreadHandle, copyingThread, (U64)0, 1001, 64 * 1024, createJoinable, "Game Data Copy Thread" );
	return true;
}

bool C_PS3InstallData::WaitForCopyToComplete()
{
	U64 retval;
	int ret = sys_ppu_thread_join( copyingThreadHandle, &retval );
	return true;
}

void GameDataThreadDelete(uint64_t arg)
{
	cellSaveDataDelete2( SYS_MEMORY_CONTAINER_ID_INVALID   );

	// Copy Icons
	C_PS3InstallData::Instance().SetGameState( C_PS3InstallData::Instance().eGameState_CheckForSpace );
	
	sys_ppu_thread_exit(0);
}


U64 C_PS3InstallData::GetGameDataSize( const Sys::C_FilePath& filePath )
{
#if 0
	return cellHddInfo.sizeKB + cellHddInfo.sysSizeKB;
#else
	U32 size = 0;
	Sys::I_FileSystem& fileSystem = Sys::C_FileSystem::Instance();
	Sys::C_DirectoryInfo dirInfo;

	const bool success = fileSystem.DirectoryInfo(  filePath + "\\" , &dirInfo);

	if(success)
	{
		for(U32 i = 0; i < dirInfo.NumFiles(); i++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
			// Recurse into folder
			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				size += GetGameDataSize( fileInfo.m_FileName );
//				printf("DIRECTORY,%s\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				size += (( fileInfo.m_FileSize + 1023 ) & ~1023);
//				printf("FILE,%s,size,%lld\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}		            
		}
	}    
	else
	{
//		printf("FAILED ON %s\n",filePath.Chars());
	}
	return size;
#endif
}

bool C_PS3InstallData::CheckFiles( Sys::I_FileSystem* sourcefileSystem, const char* sourceFileName , const char* destFileName )
{
	Sys::C_FileInfo sourceInfo;
	Sys::C_FileInfo destInfo;
	Sys::C_FilePath srcfilePath;
	Sys::C_FilePath dstfilePath;
	srcfilePath.SetFormatted( sourceFileName );
	dstfilePath.SetFormatted( destFileName );
	
#ifdef INSTALL_DEBUG_INFO
	printf("checking source[%s] dest[%s] :",sourceFileName,destFileName);
#endif	
	while ( 1 )
	{
		if (!sourcefileSystem->FileInfo( srcfilePath, &sourceInfo )) break;
		if (!Sys::C_FileSystem::Instance().FileInfo( dstfilePath, &destInfo )) break;	
	

		if (destInfo.m_FileSize != sourceInfo.m_FileSize)
		{
#ifdef INSTALL_DEBUG_INFO
			printf("size(%llu)(%llu)", sourceInfo.m_FileSize, destInfo.m_FileSize);
#endif
			break;
		}

		// [Rog] Bug: 31759 - Fix for the file time stored in a pak file being incredibly accurate - where regular file system's are not
		if ((destInfo.m_FileTime / 1000000L) != (sourceInfo.m_FileTime / 1000000L) )
		{
#ifdef INSTALL_DEBUG_INFO
			printf("time(%llu)(%llu)", sourceInfo.m_FileTime, destInfo.m_FileTime);
#endif
			break; 
		}
#ifdef INSTALL_DEBUG_INFO
		printf("ok\n");
#endif
		return true;
	}


	// Are we copying the file right now?
	for (S32 i = 0 ; i < m_FileCopyList.Size() ; i++)
	{
		if (strstr( m_FileCopyList[i].mSourceFilename.Chars(), Misc::ExtractFilename(destFileName).Chars() ))
		{
			if (m_FileCopyList[i].mCopyInProgressFlag)
			{
#ifdef INSTALL_DEBUG_INFO
				printf("copying\n");
#endif
				return false;
			}
		}
	}

#ifdef INSTALL_DEBUG_INFO
	printf("different - deleting\n");
#endif
	Sys::C_FileSystem::Instance().DeleteFile( dstfilePath );
	return false;
}

void C_PS3InstallData::VerifyContentsOfGameData()
{
	// Compare Source & Dest, if size or date differs, delete cached version.
	for (U32 x = 0 ; x < m_FileCopyList.Size() ; x++)
	{
		Sys::I_FileSystem* sourcefileSystem = m_FileCopyList[x].mFileSystem;
		Sys::C_FilePath sourcefname = C_PS3InstallData::Instance().GetFileList()[x].mSourceFilename;
		Sys::C_FilePath destfname = C_PS3InstallData::Instance().GetFileList()[x].mDestFileName;	
		Sys::C_FilePath sourceFullPath = sourcefileSystem->GetWorkingDirectory() + "/" + sourcefname;

		CheckFiles( sourcefileSystem, sourceFullPath.Chars() , destfname.Chars() );
	}

	return;
}

U64 C_PS3InstallData::GetGameDataRequiredSize()
{
	U64 totalSize = 0;
	for (int i = 0 ; i < m_FileCopyList.Size() ; i++)
	{
		if (m_FileCopyList[i].mIsGameData)
		{
			//Sys::C_FileInfo sourceInfo;
			Sys::I_FileSystem* filesystem = m_FileCopyList[i].mFileSystem;
			Sys::C_FilePath sourceFullPath;
			Btl::String sourceFullPathString;// = filesystem->GetWorkingDirectory().Chars();
			sourceFullPathString += m_FileCopyList[i].mSourceFilename.Chars();
			sourceFullPath.SetFormatted( sourceFullPathString );

			{		
				totalSize += (( m_FileCopyList[i].mSourceFileSize + 1023 ) & ~1023);
			}
			
#ifdef INSTALL_DEBUG_INFO
			//printf("%s size %d\n",sourceFullPath.Chars(),(U32)sourceInfo.m_FileSize);
#endif
			
		}
	}
	// FFS
	U64 paramSFOSize = GAME_DATA_SIZE_FUDGE; // Not bigger than
	totalSize += paramSFOSize;
	return totalSize;
}

static U32 gGameSaveSize;
void SaveDataStatusLoadCallback(CellSaveDataCBResult *pResult, CellSaveDataStatGet *get, CellSaveDataStatSet *set)
{
	memset( set, 0x00, sizeof(*set) );
	memset( pResult, 0x00, sizeof(*pResult) );
	if(get->isNewData)
	{
		// Data Not exists
		gGameSaveSize = 0;
		pResult->result = CELL_SAVEDATA_CBRESULT_ERR_NODATA;
		return;
	}
	else
	{
		gGameSaveSize = get->sizeKB + get->sysSizeKB;
		pResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
	return;
}

void SaveDataFileOperationLoadCallback(CellSaveDataCBResult *pResult, CellSaveDataFileGet *get, CellSaveDataFileSet *set)
{

	memset( set, 0x00, sizeof(*set) );
	memset( pResult, 0x00, sizeof(*pResult) );
	pResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	return;
	
}

U64 C_PS3InstallData::GetCurrentSaveGameSize()
{
	char dirName[CELL_SAVEDATA_DIRNAME_SIZE];
	CellSaveDataSetBuf setBuf;
	// Set Dir Name

	sprintf(dirName,"%s-AUTO-",PlatformPS3::g_ps3_titleId);
	
	gGameSaveSize = 0;
	
	setBuf.fileListMax = 20;
	memset(setBuf.reserved, 0, sizeof(setBuf.reserved));
	setBuf.bufSize = sizeof(CellSaveDataFileStat) * setBuf.fileListMax;
	setBuf.buf = new char [setBuf.bufSize];

	while (GetXMBActiveState()) 
	{
#ifdef INSTALL_DEBUG_INFO
		printf("GetCurrentSaveGameSize Waiting for XMB To Close\n");
#endif
		__sync();
		sys_timer_usleep( mThrottleUs );
	}

	int ret = cellSaveDataAutoLoad2(	CELL_SAVEDATA_VERSION_CURRENT,
										dirName,
										CELL_SAVEDATA_ERRDIALOG_NONE,
										&setBuf,
										SaveDataStatusLoadCallback,
										SaveDataFileOperationLoadCallback,
										SYS_MEMORY_CONTAINER_ID_INVALID,
										(void *)0//MODE_AUTO_LOAD
									);
	
	delete [] (char *)setBuf.buf;
	return gGameSaveSize;	
}


void CheckForSpaceThread( U64 param )
{
#ifdef INSTALL_DEBUG_INFO
//	LOGINFO(g_LogInstall, "CheckForSpaceThread() needs to include game save size");
#endif
	if ( !C_PS3InstallData::Instance().GetHddFreeSpaceKB() )
	{
		uint32_t block_size = 0;
		uint64_t free_block_count = 0;
		int ret = cellFsGetFreeSize( SYS_DEV_HDD0, &block_size, &free_block_count );
		C_PS3InstallData::Instance().SetHddFreeSpaceKB( (block_size * free_block_count) / 1024 );
	}
	
	#ifdef HDD_LOW_SPACE
	C_PS3InstallData::Instance().mHddFreeSpace = HDD_FREE_AMOUNT_KB;
	#endif

	// Count How much is already in our GameData Area
	S64 hddGameDataUsedKB =	C_PS3InstallData::Instance().GetCurrentInstalledSizeKB();
	// How Much Is Needed For Game Data
	S64 requiredSizeKB = (C_PS3InstallData::Instance().GetGameDataRequiredSize() + 1023) / 1024;
	C_PS3InstallData::Instance().SetRequiredGameDataKB( requiredSizeKB );

	S64 currentGameSaveSizeKB = C_PS3InstallData::Instance().GetCurrentSaveGameSize();


	// save data size
	S64 requiredSaveDataKB = 0;
	
	Sys::C_DirectoryInfo dirInfo;
	const bool success = Sys::C_FileSystem::Instance().DirectoryInfo( Sys::C_FileSystem::Instance().GetWorkingDirectory() + "/memsaveps3/" , &dirInfo);
	int ps3_icon_sizes = 0;
	if(success)
	{
		for(U32 i = 0; i < dirInfo.NumFiles(); i++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
			if (Sys::ExtractFilename( fileInfo.m_FileName ) == "icon0.png") ps3_icon_sizes += fileInfo.m_FileSize;
			if (Sys::ExtractFilename( fileInfo.m_FileName ) == "pic1.png") ps3_icon_sizes += fileInfo.m_FileSize;
			//if (fileInfo.m_FileName == "snd0.at3") ps3_icon_sizes += fileInfo.m_FileSize;
			
		}
	}

	requiredSaveDataKB += ( (ps3_icon_sizes + 1023 ) / 1024 );	// ICON0
	requiredSaveDataKB += ( 256 );					// save file max size - may change - waiting on test for max size - then we'll double it

	requiredSaveDataKB -= currentGameSaveSizeKB;	// Take away what's already saved

	C_PS3InstallData::Instance().SetRequiredSaveDataKB( requiredSaveDataKB );

	requiredSizeKB += requiredSaveDataKB;
	while (C_PS3InstallData::Instance().GetXMBActiveState()) 
	{
#ifdef INSTALL_DEBUG_INFO
		printf("CheckForSpaceThread Waiting for XMB To Close - Before Trophy\n");
#endif
		__sync();
		sys_timer_usleep( C_PS3InstallData::Instance().GetThrottle() );
	}
	S64 sTrophySizeKB = ( Game::C_AchievementSystem::Instance().GetRequiredDiskSpace() + 1023 ) / 1024;
	requiredSizeKB += sTrophySizeKB;

	C_PS3InstallData::Instance().SetRequiredHDDSpaceKB( requiredSizeKB - hddGameDataUsedKB );
	if (C_PS3InstallData::Instance().GetRequiredHDDSpaceKB() < 0) C_PS3InstallData::Instance().SetRequiredHDDSpaceKB( 0 );
	C_PS3InstallData::Instance().SetRequiredSizeMB( (C_PS3InstallData::Instance().GetRequiredHDDSpaceKB() + 1023) / 1024 );
	
	
	
#ifdef INSTALL_DEBUG_INFO
	printf("hddGameDataUsedKB       %lld\n",hddGameDataUsedKB);
	printf("mCurrentSaveGameSizeKB  %lld\n",C_PS3InstallData::Instance().GetCurrentSaveGameSizeKB());
	printf("requiredSizeKB          %lld\n",requiredSizeKB);
	printf("requiredHDDSpaceKB      %lld\n",C_PS3InstallData::Instance().GetRequiredHDDSpaceKB());
	printf("requiredSizeMB          %lld\n",C_PS3InstallData::Instance().GetRequiredSizeMB());
#endif

	// If no install needed, signal audio mutex.
	S64 sRequiredGameDataKB = C_PS3InstallData::Instance().GetRequiredGameDataKB();
	S64 sCurrentInstalledSizeKB = C_PS3InstallData::Instance().GetCurrentInstalledSizeKB();
	if ( sRequiredGameDataKB - sCurrentInstalledSizeKB == 0)
	{
		AudioStreamingEvent.Signal();
		 C_PS3InstallData::Instance().SignalInstallComplete( true );
	}

										
	S64 GetHddFreeSpaceKB		= C_PS3InstallData::Instance().GetHddFreeSpaceKB();
	S64 RequiredHDDSpaceKB	= C_PS3InstallData::Instance().GetRequiredHDDSpaceKB();
	
	if ( GetHddFreeSpaceKB < RequiredHDDSpaceKB )
	{
		S64 reqd = RequiredHDDSpaceKB - GetHddFreeSpaceKB;
		if ( reqd < 0 ) reqd = 0;
		C_PS3InstallData::Instance().SetRequiredSizeMB( (reqd + 1023) / 1024 );
		
		C_PS3InstallData::Instance().SetNotEnoughFreeSpace( true );
	}
	else
		C_PS3InstallData::Instance().SetNotEnoughFreeSpace( false );

	C_PS3InstallData::Instance().SetHasCheckedForFreeSpace( true );
	C_PS3InstallData::Instance().SetFinishedWaitingForSpaceThread( true );
	
	sys_ppu_thread_exit(0);
}


void CheckContentsThread( U64 param )
{
	C_PS3InstallData::Instance().VerifyContentsOfGameData();
	C_PS3InstallData::Instance().SetFinishedWaitingForContentsThread( true );
	sys_ppu_thread_exit(0);
}

void C_PS3InstallData::Update()
{
//	Time::C_ScopedTimer at("C_PS3InstallData::Update()");

	if ( !mUiInitialised )
		return;

#ifndef _PS3_INSTALLER_ENABLED
	return;
#endif

	if (IsPS3InstallerDisabled()) 
	{
		AudioStreamingEvent.Signal();
		mbInstallationComplete = true;
		mHasCheckedForFreeSpace = true;
		mHasCheckedForFreeSpaceUI = true;
		SetInstallingMessage( false );
		return;
	}

#if 0
	if (IsCopyInProgress())
	{
		if (Display::FontRender())
		{
			Display::FontRender()->Draw("Installing Data",1024.f/2.f, 768.f/2.f, 20, 20, Maths::RGBA(255, 255, 255, 255), Display::E_FontRenderAlign_Ct);
		}
	}
#endif

#ifndef DEMO_VERSION
// Make Sure We're Initialised
	Initialise();
	static sys_ppu_thread_t CheckForSpaceThreadHandle;
	static sys_ppu_thread_t CheckContentsThreadHandle;
	switch (mGameState)
	{

		case eGameState_UnInitialised:
		{
			if ( Initialise() )
			{
				mGameState = eGameState_ValidateHDDContents;
				mFinishedWaitingForSpaceThread = true;
			}
			break;
		}

		case eGameState_ValidateHDDContents:
		{
			mFinishedWaitingForContentsThread = false;
			int ret = sys_ppu_thread_create( &CheckContentsThreadHandle, CheckContentsThread, 0, 1001, 64 * 1024, 0, "GameData CheckHDD Contents" );
			mGameState = eGameState_ValidateHDDContentsWaiting;
			break;
		}
		case eGameState_ValidateHDDContentsWaiting:
		{
			if (mFinishedWaitingForContentsThread == true)
			{
				mGameState = eGameState_CheckForSpace;
			}
			break;
		}

		case eGameState_CheckForSpace:
		{
			if (mHasCheckedForFreeSpace)
			{
				mGameState = eGameState_CheckForSpaceWaiting;
			}
			else
			{
				mFinishedWaitingForSpaceThread = false;
				mHddFreeSpace = mRequiredHDDSpaceKB = mRequiredSizeMB = 0;
				int ret = sys_ppu_thread_create( &CheckForSpaceThreadHandle, CheckForSpaceThread, 0, 1001, 64 * 1024, 0, "GameData CheckHDD Space" );
				mGameState = eGameState_CheckForSpaceWaiting;
			}
			break;
		}

		case eGameState_CheckForSpaceWaiting:
		{
			if (mFinishedWaitingForSpaceThread)
			{
#ifdef INSTALL_DEBUG_INFO
				printf("mHddFreeSpace %lld mRequiredHDDSpaceKB %lld\n",mHddFreeSpace,mRequiredHDDSpaceKB );
#endif
				if ( mHddFreeSpace < mRequiredHDDSpaceKB )
				{
					mNotEnoughFreeSpace = true;
					mHasCheckedForFreeSpace = true;
					mHasCheckedForFreeSpaceUI = true;
#if 1
					int ret = 0;
					WCHAR buff[CELL_MSGDIALOG_STRING_SIZE];
					U32 requiredSpaceMB = GetRequiredSizeMB();

					swprintf( buff, CELL_MSGDIALOG_STRING_SIZE, GetHDDFullString(), requiredSpaceMB );

					size_t len_ucs2 = wcslen(buff);
					size_t len_utf8 = CELL_MSGDIALOG_STRING_SIZE;
					static uint8_t utf8[CELL_MSGDIALOG_STRING_SIZE];

					L10nResult L10result = UCS2stoUTF8s( (uint16_t*)buff, &len_ucs2, utf8, &len_utf8);
					if(L10result == ConversionOK)
					{
						retryOutOfHddSpaceMessage:
						ret = cellMsgDialogOpen2( CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON , 
													(char*)utf8,
													InsufficientSpaceCallBack, (void*)&m_DialogAction, NULL );

					}
#endif
					
					mGameState = eGameState_NoHDDSpace;
				}
				else
				{
					mNotEnoughFreeSpace = false;
					mHasCheckedForFreeSpace = true;
					mHasCheckedForFreeSpaceUI = true;
					S64 installDiff = C_PS3InstallData::Instance().GetRequiredGameDataKB() - C_PS3InstallData::Instance().GetCurrentInstalledSizeKB();
					if (installDiff != 0)
					{
						SetInstallingMessage( true );
					}
					mGameState = eGameState_Idle;
				}
			}
			break;
		}
		
		case eGameState_NoHDDSpace:
		{
			if (m_DialogAction != eDIALOG_ACTION_UNANSWERED) 
			{
				mGameState = eGameState_Idle;
			}
			break;
		}
		case eGameState_DeleteSpace:
		{
			break;
		}
		case eGameState_CopyStage1:
		{
			// if we haven't checked for disk space, enter idle state
			if (( mHasCheckedForFreeSpace == false ) || ( mNotEnoughFreeSpace )) 
			{
				mGameState = eGameState_Idle;
				break;
			}
			// Copies Icons & Intro Movies

			// Copy Save Game Icons etc to area
			mGameState = eGameState_ValidateHDDContents;
			break;
		}
		case eGameState_Delay:
		{
			mDelayCount--;
			if (mDelayCount == 0) mGameState = mAfterDelayState;
		}
		case eGameState_Idle:
		{
			if (m_FinishedCopying)
			{
			}		
			break;
		}
	}
#endif
}

void C_PS3InstallData::WaitForSystemFiles()
{
	for (S32 x = 0 ; x < C_PS3InstallData::Instance().GetFileList().Size() ; x++)
	{
		if (GetFileList()[x].mSystemFile)
		{
			while (GetFileList()[x].mCopyPendingFlag)
			{
				sys_timer_usleep( 15 * 1000 );
			}
		}
	}	
}


void C_PS3InstallData::AddFolderToList( Sys::I_FileSystem* fileSystem, Btl::String sourcefolder, Btl::String destfolder )
{
	if (!fileSystem) return;

	Sys::C_DirectoryInfo dirInfo;

	const bool success = fileSystem->DirectoryInfo( sourcefolder + "\\" , &dirInfo);

	if(success)
	{
		for(U32 i = 0; i < dirInfo.NumFiles(); i++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
			// Recurse into folder
			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				AddFolderToList( fileSystem, Btl::String( fileInfo.m_FileName.Chars() ), destfolder );
//				printf("DIRECTORY,%s\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				Btl::String destName;
				Btl::String sourceName;
				destName = mDataUsrdirPath;
				destName += Sys::RemoveTrailingSlash( Sys::C_FilePath( destfolder ) ).Chars();
				destName += Btl::String( fileInfo.m_FileName.Chars() );
				
				sourceName = fileInfo.m_FileName.Chars();
				
				AddFileToList( fileSystem,	sourceName.Chars(), destName.Chars(), true );
//				printf("FILE,%s,size,%lld\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}		            
		}
	}    
}


void C_PS3InstallData::UninstallFolder( Sys::I_FileSystem* fileSystem, Btl::String sourcefolder, Btl::String destfolder )
{
// Where files exist in the source folder,  delete them from the dest folder.
	if (!fileSystem) return;

	Sys::C_DirectoryInfo dirInfo;

	const bool success = fileSystem->DirectoryInfo( sourcefolder + "\\" , &dirInfo);

	if(success)
	{
		for(U32 i = 0; i < dirInfo.NumFiles(); i++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo( i );
			// Recurse into folder
			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				UninstallFolder( fileSystem, Btl::String( fileInfo.m_FileName.Chars() ), destfolder );
//				printf("DIRECTORY,%s\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				Btl::String destName;
				Btl::String sourceName;
				destName = mDataUsrdirPath;
				destName += Sys::RemoveTrailingSlash( Sys::C_FilePath( destfolder ) ).Chars();
				destName += Btl::String( fileInfo.m_FileName.Chars() );
				
				sourceName = fileInfo.m_FileName.Chars();
				
				Sys::C_FileSystem::Instance().DeleteFile( destName.Chars() );
//				printf("FILE,%s,size,%lld\n",fileInfo.m_FileName.Chars(),fileInfo.m_FileSize);
			}		            
		}
	}    
}

void C_PS3InstallData::RemoveUnusedLanguageFiles( const char *usedLang )
{
	const char *languagePaks[] = { "en", "fr", "de", "it", "es", "hu", "ru", "pl", "cs", "sc", "tc", "ja", "ko" };

	// Determine which language is already installed
	const nrLanguages = sizeof(languagePaks) / sizeof(languagePaks[0]);

	bool foundLanguage = false;
	for (U32 x = 0 ; x < nrLanguages ; x++)
	{
		Sys::C_FileInfo fileInfo;
		Sys::C_FilePath languagePath = mDataUsrdirPath;
		languagePath += "/";
		languagePath += languagePaks[x];

		if (Sys::C_FileSystem::Instance().FileInfo( languagePath, &fileInfo ))
		{
			// This is the currently installed language, so Open the pak and delete all the installed files 
			// Only if its different
			if (!memcmp(languagePaks[x],usedLang,2)) return;
			LogicalFs::C_PakFs*	pPakFs = NULL;
			Sys::C_FilePath installPakPath = "/install/install_raw_";
			installPakPath += languagePaks[x];
			installPakPath += ".pak";

			Sys::C_FileInfo fileInfo;
			if (Sys::GetDefaultFileSystem()->FileInfo( installPakPath.Chars(), &fileInfo ))
			{
				pPakFs = LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), installPakPath);
				UninstallFolder( pPakFs, "/", "/" );
			}			
			// Do this last incase reset is performed during this that way we have a better chance of 
			// cleaning up any partially uninstalled files.
			Sys::C_FileSystem::Instance().DeleteFile( languagePath );
			// Close The Pak
			SAFE_DELETE_PTR( pPakFs );

			// This is the currently installed language, so Open the pak and delete all the installed files 
			// Only if its different
			installPakPath = "/install/install_compressed_";
			installPakPath += languagePaks[x];
			installPakPath += ".pak";
			if (Sys::GetDefaultFileSystem()->FileInfo( installPakPath.Chars(), &fileInfo ))
			{
				pPakFs = LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), installPakPath);
				UninstallFolder( pPakFs, "/", "/" );
			}
			// Do this last incase reset is performed during this that way we have a better chance of 
			// cleaning up any partially uninstalled files.
			Sys::C_FileSystem::Instance().DeleteFile( languagePath );
			// Close The Pak
			SAFE_DELETE_PTR( pPakFs );

			foundLanguage = true;
			break;
		}
	}
	// If there is no language file then uninstall all of them 	
	if (!foundLanguage)
	{
		for (U32 x = 0 ; x < nrLanguages ; x++)
		{
			Sys::C_FileInfo fileInfo;
			Sys::C_FilePath languagePath = mDataUsrdirPath;
			languagePath += "/";
			languagePath += languagePaks[x];

			LogicalFs::C_PakFs*	pPakFs;
			Sys::C_FilePath installPakPath = "/install/install_raw_";
			installPakPath += languagePaks[x];
			installPakPath += ".pak";
			if (Sys::GetDefaultFileSystem()->FileInfo( installPakPath.Chars(), &fileInfo ))
			{
				pPakFs = LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), installPakPath);
					
				if (pPakFs)
				{
					UninstallFolder( pPakFs, "/", "/" );
					// Close The Pak
					SAFE_DELETE_PTR( pPakFs );
				}
			}

			installPakPath = "/install/install_compressed_";
			installPakPath += languagePaks[x];
			installPakPath += ".pak";
			if (Sys::GetDefaultFileSystem()->FileInfo( installPakPath.Chars(), &fileInfo ))
			{
				pPakFs = LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), installPakPath);
					
				if (pPakFs)
				{
					UninstallFolder( pPakFs, "/", "/" );
					// Close The Pak
					SAFE_DELETE_PTR( pPakFs );
				}
			}

		}
	}
	
}


int get_parametar( void )
{
	int ret = CELL_GAME_RET_OK ;

	int value = 0 ;
	int id_list[] = 
	{
		CELL_GAME_PARAMID_PARENTAL_LEVEL,
		CELL_GAME_PARAMID_RESOLUTION,
		CELL_GAME_PARAMID_SOUND_FORMAT
	} ;
	const char *id_name_list[] = 
	{
		"PARENTAL_LEVEL",
		"RESOLUTION",
		"SOUND_FORMAT"
	} ;
	int cnt = sizeof(id_list)/sizeof(int);
	int i = 0 ;
	for(i=0 ; i < cnt ; i++){
		value = 0 ;
		ret = cellGameGetParamInt( id_list[i], &value );
		if(ret < 0)
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameGetParamInt() Error id:[%s] : 0x%x\n", id_name_list[i], ret);
#endif
		}
		else 
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameGetParamInt() OK id:[%s] value = [%d]\n", id_name_list[i], value);
#endif
		}
	}

	char buf[128] ;
	int id_list2[] = {
		CELL_GAME_PARAMID_TITLE,			
		CELL_GAME_PARAMID_TITLE_DEFAULT,	
		CELL_GAME_PARAMID_TITLE_JAPANESE,	
		CELL_GAME_PARAMID_TITLE_ENGLISH,	
		CELL_GAME_PARAMID_TITLE_FRENCH,		
		CELL_GAME_PARAMID_TITLE_SPANISH,	
		CELL_GAME_PARAMID_TITLE_GERMAN,		
		CELL_GAME_PARAMID_TITLE_ITALIAN,	
		CELL_GAME_PARAMID_TITLE_DUTCH,		
		CELL_GAME_PARAMID_TITLE_PORTUGUESE,	
		CELL_GAME_PARAMID_TITLE_RUSSIAN,	
		CELL_GAME_PARAMID_TITLE_KOREAN,		
		CELL_GAME_PARAMID_TITLE_CHINESE_T,	
		CELL_GAME_PARAMID_TITLE_CHINESE_S,	
		CELL_GAME_PARAMID_TITLE_FINNISH,	
		CELL_GAME_PARAMID_TITLE_SWEDISH,	
		CELL_GAME_PARAMID_TITLE_DANISH,		
		CELL_GAME_PARAMID_TITLE_NORWEGIAN,	
		CELL_GAME_PARAMID_TITLE_ID,			
		CELL_GAME_PARAMID_VERSION,			
	} ;
	const char *id_name_list2[] = {
		"TITLE_CURRENT",	
		"TITLE_DEFAULT",	
		"TITLE_JAPANESE",	
		"TITLE_ENGLISH",	
		"TITLE_FRENCH",		
		"TITLE_SPANISH",	
		"TITLE_GERMAN",		
		"TITLE_ITALIAN",	
		"TITLE_DUTCH",		
		"TITLE_PORTUGUESE",	
		"TITLE_RUSSIAN",	
		"TITLE_KOREAN",		
		"TITLE_CHINESE_T",	
		"TITLE_CHINESE_S",	
		"TITLE_FINNISH",	
		"TITLE_SWEDISH",	
		"TITLE_DANISH",		
		"TITLE_NORWEGIAN",	
		"TITLE_ID",			
		"VERSION",			
	} ;
	cnt = sizeof(id_list2)/sizeof(int);
	for(i=0 ; i < cnt ; i++)
	{
		memset(buf, 0x00, 128);
		ret = cellGameGetParamString( id_list2[i], buf, 128 );
		if(ret < 0)
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameGetParamString() Error id:[%s] : 0x%x\n", id_name_list2[i], ret);
#endif
		}
		else 
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameGetParamString() OK id:[%s] string = [%s]\n", id_name_list2[i], buf);
#endif
		}
	}

	return ret ;
}

int set_parametar( void )
{
	int ret = CELL_GAME_RET_OK ;

	int id_list2[] = 
	{
		CELL_GAME_PARAMID_TITLE_DEFAULT,	
		CELL_GAME_PARAMID_TITLE_JAPANESE,	
		CELL_GAME_PARAMID_TITLE_ENGLISH,	
		CELL_GAME_PARAMID_TITLE_FRENCH,		
		CELL_GAME_PARAMID_TITLE_SPANISH,	
		CELL_GAME_PARAMID_TITLE_GERMAN,		
		CELL_GAME_PARAMID_TITLE_ITALIAN,	
		CELL_GAME_PARAMID_TITLE_DUTCH,		
		CELL_GAME_PARAMID_TITLE_PORTUGUESE,	
		CELL_GAME_PARAMID_TITLE_RUSSIAN,	
		CELL_GAME_PARAMID_TITLE_KOREAN,		
		CELL_GAME_PARAMID_TITLE_CHINESE_T,	
		CELL_GAME_PARAMID_TITLE_CHINESE_S,	
		CELL_GAME_PARAMID_TITLE_FINNISH,	
		CELL_GAME_PARAMID_TITLE_SWEDISH,	
		CELL_GAME_PARAMID_TITLE_DANISH,		
		CELL_GAME_PARAMID_TITLE_NORWEGIAN,	
	} ;
	const char *buf_list2[] = 
	{
		"Blur",			//SAMPLE_PARAMSFO_TITLE_DEFAULT,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_JAPANESE,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_ENGLISH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_FRENCH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_SPANISH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_GERMAN,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_ITALIAN,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_DUTCH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_PORTUGUESE,	
		"Blur",			//SAMPLE_PARAMSFO_TITLE_RUSSIAN,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_KOREAN,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_CHINESE_T,	
		"Blur",			//SAMPLE_PARAMSFO_TITLE_CHINESE_S,	
		"Blur",			//SAMPLE_PARAMSFO_TITLE_FINNISH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_SWEDISH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_DANISH,		
		"Blur",			//SAMPLE_PARAMSFO_TITLE_NORWEGIAN,	
	} ;
	const char *id_name_list2[] = {
		"TITLE_DEFAULT",	
		"TITLE_JAPANESE",	
		"TITLE_ENGLISH",	
		"TITLE_FRENCH",		
		"TITLE_SPANISH",	
		"TITLE_GERMAN",		
		"TITLE_ITALIAN",	
		"TITLE_DUTCH",		
		"TITLE_PORTUGUESE",	
		"TITLE_RUSSIAN",	
		"TITLE_KOREAN",		
		"TITLE_CHINESE_T",	
		"TITLE_CHINESE_S",	
		"TITLE_FINNISH",	
		"TITLE_SWEDISH",	
		"TITLE_DANISH",		
		"TITLE_NORWEGIAN",	
	} ;
	int cnt = sizeof(id_list2)/sizeof(int);
	int i = 0 ;
	for(i=0 ; i < cnt ; i++)
	{
		ret = cellGameSetParamString( id_list2[i], buf_list2[i] );
		if(ret < 0)
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameSetParamString() Error id:[%s] : 0x%x\n", id_name_list2[i], ret);
#endif
		} 
		else
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameSetParamString() OK id:[%s]\n", id_name_list2[i]);
#endif
		}
	}

	return ret ;
}

bool C_PS3InstallData::Initialise()
{
#ifndef _PS3_INSTALLER_ENABLED
	return true;
#endif

	if (IsPS3InstallerDisabled())
		return true;
	if (gAbortPS3Installer)
		return false;
	if (m_Initialised)
		return true;
#ifdef DEMO_VERSION
	AudioStreamingEvent.Signal();
	mbInstallationComplete = true;
	return true;
#endif
	if ( C_PS3InstallData::Instance().IsSuspended() )
	{
		return false;
	}

	CellGameContentSize size;
	memset(&size, 0x00, sizeof(CellGameContentSize));
	int ret = cellGameDataCheck( CELL_GAME_GAMETYPE_GAMEDATA, PlatformPS3::g_ps3_titleId, &size );
	if(ret < 0)
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellGameDataCheck() Error: 0x%x\n", ret);
#endif
	}

	if(ret != CELL_GAME_RET_NONE)
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellGameDataCheck() OK [data exist]\n");
#endif
		ret = get_parametar() ;
		if(size.sizeKB == CELL_GAME_SIZEKB_NOTCALC)
		{
			int sizeKB = 0;
			ret = cellGameGetSizeKB( &sizeKB ) ;
			if(ret < 0)
			{
#ifdef INSTALL_DEBUG_INFO
				printf("cellGameGetSizeKB() Error: 0x%x\n", ret);
#endif
			}
			else 
			{
				size.sizeKB = sizeKB ;
			}
			mCurrentInstalledSizeKB = sizeKB;
		}
#ifdef INSTALL_DEBUG_INFO
		printf("  hddFreeSizeKB = [%d] sizeKB = [%d] sysSizeKB = [%d]\n", size.hddFreeSizeKB, size.sizeKB, size.sysSizeKB); 
#endif
	} 
	else
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellGameDataCheck() OK [new data]\n");
#endif
		mCurrentInstalledSizeKB = 0;

		char tmp_contentInfoPath[CELL_GAME_PATH_MAX];
		char tmp_usrdirPath[CELL_GAME_PATH_MAX];
		CellGameSetInitParams init;
		memset(&init, 0x00, sizeof(init));

		// get the system language
		S32 sLang = CELL_SYSUTIL_LANG_ENGLISH;
		cellSysutilGetSystemParamInt( CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &sLang );
		char* title;

		switch( sLang )
		{
			default:
			case CELL_SYSUTIL_LANG_ENGLISH:
				title = "Blur Game Data";
			break;

			case CELL_SYSUTIL_LANG_FRENCH:
				title = "Blur Données de jeu";
			break;

			case CELL_SYSUTIL_LANG_ITALIAN:
				title = "Blur Dati di gioco";
			break;

			case CELL_SYSUTIL_LANG_SPANISH:
				title = "Blur Datos de juego";
			break;
		}

		size_t len_utf8 = CELL_GAME_SYSP_TITLE_SIZE-1;
		size_t len_ucs2 = (size_t)strlen(title);
		SBCSstoUTF8s( (uint8_t*)title, &len_ucs2, (uint8_t*)init.title, &len_utf8, L10N_CODEPAGE_1252);
//		strncpy(init.title,   SAMPLE_CREATE_PARAMSFO_TITLE, CELL_GAME_SYSP_TITLE_SIZE - 1) ;
		strncpy(init.titleId, PlatformPS3::g_ps3_titleId, CELL_GAME_SYSP_TITLEID_SIZE - 1) ;
		strncpy(init.version, "01.00", CELL_GAME_SYSP_VERSION_SIZE - 1) ;
		ret = cellGameCreateGameData( &init, tmp_contentInfoPath, tmp_usrdirPath ) ;
		if(ret < 0)
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameCreateGameData() Error: 0x%x\n", ret);
#endif
		}
		else 
		{
#ifdef INSTALL_DEBUG_INFO
			printf("cellGameCreateGameData() OK\n");
#endif
//			saveSampleGameData(tmp_contentInfoPath, tmp_usrdirPath);
			set_parametar() ;
		}
	}	

	ret = cellGameContentPermit( mDataContentInfoPath, mDataUsrdirPath ) ;
	if(ret < 0)
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellGameContentPermit() Error: 0x%x\n", ret);
#endif
		//Evil hack to disable installer constantly firing up when game data corrupted.
		//Game will continue to run off disc.
		gDisablePS3Installer = true;
		return ret ;
	}
	else 
	{
#ifdef INSTALL_DEBUG_INFO
		printf("cellGameContentPermit() OK\n");
		printf("  contentInfoPath:[%s]\n", mDataContentInfoPath);
		printf("  usrdirPath:[%s]\n", mDataUsrdirPath);
#endif
	}
	m_Initialised = true;
	


// because gamedata is not exclusive to installed data
// the installed size taken ( total gamedata in use ) is 
// not appropriate for a basis of how much data is installed.
// therefore we must count only installed files. 

	AddAllFilesToList();

	for (U32 x = 0 ; x < m_FileCopyList.Size() ; x++)
	{
		Sys::I_FileSystem* sourcefileSystem = m_FileCopyList[x].mFileSystem;
		Sys::C_FilePath sourcefname			= C_PS3InstallData::Instance().GetFileList()[x].mSourceFilename;
		Sys::C_FilePath destfname			= C_PS3InstallData::Instance().GetFileList()[x].mDestFileName;	
		Sys::C_FilePath sourceFullPath		= sourcefileSystem->GetWorkingDirectory() + "/" + sourcefname;
		Sys::C_FileInfo destInfo;
		Sys::C_FileInfo sourceInfo;

		if (Sys::C_FileSystem::Instance().FileInfo( destfname, &destInfo ))
		{
			C_PS3InstallData::Instance().GetFileList()[x].SetDestFileSize( destInfo.m_FileSize );
			C_PS3InstallData::Instance().GetFileList()[x].SetDestFileTime( destInfo.m_FileTime );
		}
		if (sourcefileSystem->FileInfo( sourceFullPath, &sourceInfo ))
		{
			C_PS3InstallData::Instance().GetFileList()[x].SetSourceFileSize( sourceInfo.m_FileSize );
			C_PS3InstallData::Instance().GetFileList()[x].SetSourceFileTime( sourceInfo.m_FileTime );
		}
	}

	mCurrentInstalledSizeKB = 0;
	mTotalNrKBytesToCopy = 0;
	
	for (U32 x = 0 ; x < m_FileCopyList.Size() ; x++)
	{
		Sys::I_FileSystem* sourcefileSystem = m_FileCopyList[x].mFileSystem;
		Sys::C_FilePath sourcefname			= C_PS3InstallData::Instance().GetFileList()[x].mSourceFilename;
		Sys::C_FilePath destfname			= C_PS3InstallData::Instance().GetFileList()[x].mDestFileName;	
		Sys::C_FilePath sourceFullPath		= sourcefileSystem->GetWorkingDirectory() + "/" + sourcefname;


		bool added = C_PS3InstallData::Instance().GetFileList()[x].mDestFileSize > 0;
		mCurrentInstalledSizeKB += ((C_PS3InstallData::Instance().GetFileList()[x].mDestFileSize + 1023) & ~1023);

		bool same = true;
		if (C_PS3InstallData::Instance().GetFileList()[x].mDestFileSize != C_PS3InstallData::Instance().GetFileList()[x].mSourceFileSize) same = false;
		else if ((C_PS3InstallData::Instance().GetFileList()[x].mDestFileTime / 1000000L) != (C_PS3InstallData::Instance().GetFileList()[x].mSourceFileTime / 1000000L)) same = false; 
		if (!same)
		{
			mTotalNrKBytesToCopy += ((C_PS3InstallData::Instance().GetFileList()[x].mSourceFileSize + 1023) & ~1023);
			if (added) mCurrentInstalledSizeKB -= ((C_PS3InstallData::Instance().GetFileList()[x].mDestFileSize + 1023) & ~1023);
		}

	}	
	mTotalNrKBytesToCopy += 1023;
	mTotalNrKBytesToCopy /= 1024;
	mCurrentInstalledSizeKB += GAME_DATA_SIZE_FUDGE;
	mCurrentInstalledSizeKB += 1023;
	mCurrentInstalledSizeKB /= 1024;
	
	return true;
}

void C_PS3InstallData::IncrementGameDataCopyProgressBytes(S64 bytes)
{
	MT::AtomicAdd((volatile U64*)&mCopyProgressBytes, bytes);
}

S64 C_PS3InstallData::GetGameDataCopyProgressBytes() const
{
	return MT::AtomicGet((volatile U64*)&mCopyProgressBytes);
}

float C_PS3InstallData::GetProgressTotal()
{
	if (!IsCopyInProgress())
		return 0.f;
//This creates a progress fraction offset from where we'd last copied up to.
//	S64 thisSessionProgressKB = (GetGameDataCopyProgressBytes() + 1023) / 1024;
//	S64 totalProgressKB = mCurrentInstalledSizeKB + thisSessionProgressKB + GAME_DATA_SIZE_FUDGE;
//	F32 frac = (F32)totalProgressKB / (F32)mRequiredGameDataKB;

//This yields a progress fraction only considering the work we're currently doing to update
//our current installation.
	S64 thisSessionProgressKB = ((GetGameDataCopyProgressBytes() + 1023) / 1024) + mCurrentInstalledSizeKB;
	S64 thisSessionmRequiredGameDataKB = mRequiredGameDataKB ;
	S64 totalProgressKB = thisSessionProgressKB + GAME_DATA_SIZE_FUDGE;
	F32 frac = (F32)totalProgressKB / (F32)thisSessionmRequiredGameDataKB;

	if (frac > 1.f)
	{
		//This will happen if we had to delete a partially installed file along the way. So bytes read + original
		//installation size would be > required gamedata size. Not great, but it's
		//2am and it's an edge case in unfamiliar code.
		frac = 1.f;
	}
#ifdef INSTALL_DEBUG_INFO
//	printf("%i %i %.2f\n", (U32)totalProgressKB, (U32)thisSessionmRequiredGameDataKB, frac);
#endif
	return frac;
}




void C_PS3InstallData::AddAllFilesToList()
{
#ifdef DISABLE_ASYNC_HACK
	C_AutoAsyncFileDisable autoDisableAsync;
#endif

#ifndef DEMO_VERSION
	// Icon's First
	Btl::String contentInfoPath = mDataContentInfoPath;
	Btl::String gameDataPath	= mDataUsrdirPath;					
	// Open Any Installation pack files here
	// Determine Language & Open Appropriate Audio Pak File
	const char* pLanguageCode = UI::C_UILocaliser::Instance().GetLanguageCode();
	RemoveUnusedLanguageFiles( pLanguageCode );

	// Add File to Identify Language
	Btl::String languageIdFileName = mDataUsrdirPath;
	languageIdFileName += "/";
	languageIdFileName += pLanguageCode;
	AddFileToList( &Sys::C_FileSystem::Instance(), "disc_check", languageIdFileName.Chars() );

	Sys::C_FilePath installPakPath = "/install/install_";
	installPakPath += pLanguageCode;
	installPakPath += ".pak";



	AddFileToList( &Sys::C_FileSystem::Instance(), 	"memsaveps3/icon0.png",			Btl::String( contentInfoPath+ Btl::String( "/ICON0.PNG" ) ).Chars(), false, true, true);
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"memsaveps3/pic1_gamedata.png",	Btl::String( contentInfoPath + Btl::String( "/PIC1.PNG" )  ).Chars(), false, true, true);
//	AddFileToList( &Sys::C_FileSystem::Instance(), 	"ui/movies/ps3_install.bik",	Btl::String( gameDataPath + Btl::String( "/ui/movies/ps3_install.bik") ).Chars() );

	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/shaders-speed.pak",		Btl::String( gameDataPath + Btl::String( "/cache/shaders-speed.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/gamedata_boot.pak",		Btl::String( gameDataPath + Btl::String( "/cache/gamedata_boot.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/boot_rampak.pak",		Btl::String( gameDataPath + Btl::String( "/cache/boot_rampak.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/boot.pak",				Btl::String( gameDataPath + Btl::String( "/cache/boot.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/gamedata.pak",			Btl::String( gameDataPath + Btl::String( "/cache/gamedata.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/root.pak",				Btl::String( gameDataPath + Btl::String( "/cache/root.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/lut_rampak.pak",			Btl::String( gameDataPath + Btl::String( "/cache/lut_rampak.pak") ).Chars() );
	AddFileToList( &Sys::C_FileSystem::Instance(), 	"cache/root_streaming.pak",		Btl::String( gameDataPath + Btl::String( "/cache/root_streaming.pak") ).Chars() );

//	mpPakFs = LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), installPakPath );
//	AddFolderToList( mpPakFs, "/", "/" );


	Sys::C_FilePath pakNameCompressed		= "/install/install_compressed.pak";
	Sys::C_FilePath pakNameRaw				= "/install/install_raw.pak";
	Sys::C_FilePath pakNameCompressedLoc	= "/install/install_compressed_";
	Sys::C_FilePath pakNameRawLoc			= "/install/install_raw_";
	
	pakNameCompressedLoc += pLanguageCode;
	pakNameCompressedLoc += ".pak";
	pakNameRawLoc += pLanguageCode;
	pakNameRawLoc += ".pak";

	Sys::C_FileInfo fileInfo;

#ifdef USE_NEW_THINGY_MAJIG
//	if (Sys::GetDefaultFileSystem()->FileInfo( pakNameCompressed.Chars(),		&fileInfo ))
//	{
//		mpPakFsCompressed		= LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), pakNameCompressed );
//	}
	if (Sys::GetDefaultFileSystem()->FileInfo( pakNameRaw.Chars(),				&fileInfo ))
	{
		mpPakFsRaw				= LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), pakNameRaw );
	}
	if (Sys::GetDefaultFileSystem()->FileInfo( pakNameCompressedLoc.Chars(),	&fileInfo ))
	{
		mpPakFsCompressedLoc	= LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), pakNameCompressedLoc );
	}
	if (Sys::GetDefaultFileSystem()->FileInfo( pakNameRawLoc.Chars(),			&fileInfo ))
	{
		mpPakFsRawLoc			= LogicalFs::PakHelpers::OpenPak(*Sys::GetDefaultFileSystem(), pakNameRawLoc );
	}

//	if (mpPakFsCompressed	)	AddFolderToList( mpPakFsCompressed,		"/", "/" );
	if (mpPakFsRaw			)	AddFolderToList( mpPakFsRaw,			"/", "/" );
	if (mpPakFsCompressedLoc)	AddFolderToList( mpPakFsCompressedLoc,	"/", "/" );
	if (mpPakFsRawLoc		)	AddFolderToList( mpPakFsRawLoc,			"/", "/" );
#endif

#endif
}


#endif


