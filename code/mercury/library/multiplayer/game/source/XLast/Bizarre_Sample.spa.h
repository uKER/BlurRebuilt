////////////////////////////////////////////////////////////////////
//
// Bizarre_Sample.spa.h
//
// Auto-generated on Friday, 27 June 2008 at 16:59:58
// XLAST project version 1.0.73.0
// SPA Compiler version 2.0.6534.3
//
////////////////////////////////////////////////////////////////////

#ifndef __MERCURY_SAMPLE_SPA_H__
#define __MERCURY_SAMPLE_SPA_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Title info
//

#define TITLEID_MERCURY_SAMPLE                      0x534507DD

//
// Context ids
//
// These values are passed as the dwContextId to XUserSetContext.
//

#define CONTEXT_CTX_GAMEMAP                         7
#define CONTEXT_CTX_GAMERULE                        8

//
// Context values
//
// These values are passed as the dwContextValue to XUserSetContext.
//

// Values for CONTEXT_CTX_GAMEMAP

#define CONTEXT_CTX_GAMEMAP_MAP_DEFAULT             0
#define CONTEXT_CTX_GAMEMAP_MAP_1                   1
#define CONTEXT_CTX_GAMEMAP_MAP_2                   2
#define CONTEXT_CTX_GAMEMAP_MAP_3                   3
#define CONTEXT_CTX_GAMEMAP_MAP_4                   4
#define CONTEXT_CTX_GAMEMAP_MAP_5                   5
#define CONTEXT_CTX_GAMEMAP_MAP_6                   6
#define CONTEXT_CTX_GAMEMAP_MAP_7                   7
#define CONTEXT_CTX_GAMEMAP_MAP_8                   8
#define CONTEXT_CTX_GAMEMAP_MAP_9                   9
#define CONTEXT_CTX_GAMEMAP_MAP_10                  10

// Values for CONTEXT_CTX_GAMERULE

#define CONTEXT_CTX_GAMERULE_CTX_GAMERULE_CTF       0
#define CONTEXT_CTX_GAMERULE_CTX_GAMERULE_TDM       1
#define CONTEXT_CTX_GAMERULE_CTX_GAMERULE_OBJECTIVE 2
#define CONTEXT_CTX_GAMERULE_CTX_GAMERULE_ZONE      3

// Values for X_CONTEXT_PRESENCE

#define CONTEXT_PRESENCE_PRE_FRONTEND               0
#define CONTEXT_PRESENCE_PRE_CAMPAIGN               1
#define CONTEXT_PRESENCE_PRE_ONLINE_GAME            2
#define CONTEXT_PRESENCE_PRE_ONLINE_PARTY           3

// Values for X_CONTEXT_GAME_MODE

#define CONTEXT_GAME_MODE_PARTY                     0
#define CONTEXT_GAME_MODE_CTF                       1
#define CONTEXT_GAME_MODE_TDM                       2
#define CONTEXT_GAME_MODE_OBJECTIVE                 3
#define CONTEXT_GAME_MODE_ZONE                      4

//
// Property ids
//
// These values are passed as the dwPropertyId value to XUserSetProperty
// and as the dwPropertyId value in the XUSER_PROPERTY structure.
//

#define PROPERTY_MM_MAP                             0x10000001
#define PROPERTY_MM_MAXPLAYERS                      0x10000002
#define PROPERTY_MM_GAMETYPE                        0x10000003
#define PROPERTY_MM_PARTY                           0x10000004
#define PROPERTY_MM_GAMERULE                        0x10000005
#define PROPERTY_MM_FREESLOTS                       0x10000006
#define PROPERTY_MM_SKILLLOW                        0x10000007
#define PROPERTY_MM_SKILLHIGH                       0x10000008
#define PROPERTY_LBC_PLAYERRANK                     0x1000000C
#define PROPERTY_SCOREBOARD_RATING                  0x20000009
#define PROPERTY_LBC_EXPERIENCE_RATING              0x2000000A
#define PROPERTY_LBC_POINTS                         0x2000000B
#define PROPERTY_LBC_GAMESPLAYED                    0x2000000D
#define PROPERTY_LBC_GAMESWON                       0x2000000E
#define PROPERTY_LBC_OBJECTIVES                     0x2000000F
#define PROPERTY_LBC_KILLS                          0x20000010
#define PROPERTY_LBC_ASSISTS                        0x20000011
#define PROPERTY_LBC_DEATHS                         0x20000012
#define PROPERTY_LBC_HEADSHOTS                      0x20000013
#define PROPERTY_LBC_WEAPONKILLS00_KNIVES           0x20000014
#define PROPERTY_LBC_WEAPONKILLS01_PISTOLS          0x20000015
#define PROPERTY_LBC_WEAPONKILLS02_SHOTGUNS         0x20000016
#define PROPERTY_LBC_WEAPONKILLS03_SMGS             0x20000017
#define PROPERTY_LBC_WEAPONKILLS04_ASSAULTS         0x20000018
#define PROPERTY_LBC_WEAPONKILLS05_SNIPERS          0x20000019
#define PROPERTY_LBC_WEAPONKILLS06_GRENADES         0x2000001A
#define PROPERTY_LBC_WEAPONKILLS07_C4S              0x2000001B
#define PROPERTY_LBC_EXPERIENCE                     0x2000001C

//
// Achievement ids
//
// These values are used in the dwAchievementId member of the
// XUSER_ACHIEVEMENT structure that is used with
// XUserWriteAchievements and XUserCreateAchievementEnumerator.
//

#define ACHIEVEMENT_ACHIEVEMENT_1                   1
#define ACHIEVEMENT_ACHEIVEMENT_2                   2
#define ACHIEVEMENT_ACHEIVEMENT_3                   3
#define ACHIEVEMENT_ACHEIVEMENT_4                   4
#define ACHIEVEMENT_ACHEIVEMENT_5                   5
#define ACHIEVEMENT_ACHIEVEMENT_6                   6
#define ACHIEVEMENT_ACHIEVEMENT_7                   7
#define ACHIEVEMENT_ACHIEVEMENT_8                   8
#define ACHIEVEMENT_ACHIEVEMENT_9                   9

//
// Stats view ids
//
// These are used in the dwViewId member of the XUSER_STATS_SPEC structure
// passed to the XUserReadStats* and XUserCreateStatsEnumerator* functions.
//

// Skill leaderboards for ranked game modes

#define STATS_VIEW_SKILL_RANKED_PARTY               0xFFFF0000
#define STATS_VIEW_SKILL_RANKED_CTF                 0xFFFF0001
#define STATS_VIEW_SKILL_RANKED_TDM                 0xFFFF0002
#define STATS_VIEW_SKILL_RANKED_OBJECTIVE           0xFFFF0003
#define STATS_VIEW_SKILL_RANKED_ZONE                0xFFFF0004

// Skill leaderboards for unranked (standard) game modes

#define STATS_VIEW_SKILL_STANDARD_PARTY             0xFFFE0000
#define STATS_VIEW_SKILL_STANDARD_CTF               0xFFFE0001
#define STATS_VIEW_SKILL_STANDARD_TDM               0xFFFE0002
#define STATS_VIEW_SKILL_STANDARD_OBJECTIVE         0xFFFE0003
#define STATS_VIEW_SKILL_STANDARD_ZONE              0xFFFE0004

// Title defined leaderboards

#define STATS_VIEW_EXPERIENCE                       1

//
// Stats view column ids
//
// These ids are used to read columns of stats views.  They are specified in
// the rgwColumnIds array of the XUSER_STATS_SPEC structure.  Rank, rating
// and gamertag are not retrieved as custom columns and so are not included
// in the following definitions.  They can be retrieved from each row's
// header (e.g., pStatsResults->pViews[x].pRows[y].dwRank, etc.).
//

// Column ids for EXPERIENCE

#define STATS_COLUMN_EXPERIENCE_EXPERIENCE          18
#define STATS_COLUMN_EXPERIENCE_POINTS              1
#define STATS_COLUMN_EXPERIENCE_PLAYER_RANK         2
#define STATS_COLUMN_EXPERIENCE_GAMES_PLAYED        3
#define STATS_COLUMN_EXPERIENCE_GAMES_WON           4
#define STATS_COLUMN_EXPERIENCE_OBJECTIVES          5
#define STATS_COLUMN_EXPERIENCE_ASSISTS             6
#define STATS_COLUMN_EXPERIENCE_HEADSHOTS           7
#define STATS_COLUMN_EXPERIENCE_DEATHS              8
#define STATS_COLUMN_EXPERIENCE_KILLS               9
#define STATS_COLUMN_EXPERIENCE_KNIFE_KILLS         10
#define STATS_COLUMN_EXPERIENCE_PISTOL_KILLS        11
#define STATS_COLUMN_EXPERIENCE_SHOTGUN_KILLS       12
#define STATS_COLUMN_EXPERIENCE_SNG_KILLS           13
#define STATS_COLUMN_EXPERIENCE_ASSAULT_RIFLE_KILLS 14
#define STATS_COLUMN_EXPERIENCE_SNIPER_RIFLE_KILLS  15
#define STATS_COLUMN_EXPERIENCE_GRENADE_KILLS       16
#define STATS_COLUMN_EXPERIENCE_C4_KILLS            17

//
// Matchmaking queries
//
// These values are passed as the dwProcedureIndex parameter to
// XSessionSearch to indicate which matchmaking query to run.
//

#define SESSION_MATCH_QUERY_DEFAULT_QUERY           0

//
// Gamer pictures
//
// These ids are passed as the dwPictureId parameter to XUserAwardGamerTile.
//



#ifdef __cplusplus
}
#endif

#endif // __MERCURY_SAMPLE_SPA_H__


