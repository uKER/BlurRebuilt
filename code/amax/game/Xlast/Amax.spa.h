////////////////////////////////////////////////////////////////////
//
// Amax.spa.h
//
// Auto-generated on Thursday, 22 April 2010 at 12:14:38
// XLAST project version 1.0.471.0
// SPA Compiler version 2.0.11164.2
//
////////////////////////////////////////////////////////////////////

#ifndef __BLUR_SPA_H__
#define __BLUR_SPA_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Title info
//

#define TITLEID_BLUR                                0x41560845

//
// Context ids
//
// These values are passed as the dwContextId to XUserSetContext.
//

#define CONTEXT_RICH_PRESENCE                       0
#define CONTEXT_RP_TOP                              1
#define CONTEXT_RP_BOTTOM                           2
#define CONTEXT_RP_LEGEND_ROMAN                     3

//
// Context values
//
// These values are passed as the dwContextValue to XUserSetContext.
//

// Values for CONTEXT_RICH_PRESENCE

#define CONTEXT_RICH_PRESENCE_DEFAULT               0
#define CONTEXT_RICH_PRESENCE_ONLINE                1
#define CONTEXT_RICH_PRESENCE_ONLINE_WORLD_TOUR     2
#define CONTEXT_RICH_PRESENCE_SYSTEMLINK            3
#define CONTEXT_RICH_PRESENCE_SPLITSCREEN           4
#define CONTEXT_RICH_PRESENCE_TIMEATTACK            5
#define CONTEXT_RICH_PRESENCE_CAREER                6

// Values for CONTEXT_RP_TOP

#define CONTEXT_RP_TOP_SIGNEDIN                     0
#define CONTEXT_RP_TOP_IDLE                         1
#define CONTEXT_RP_TOP_LEADERBOARDS                 2
#define CONTEXT_RP_TOP_HELP                         3
#define CONTEXT_RP_TOP_SINGLEPLAYER                 4
#define CONTEXT_RP_TOP_PHOTOS                       5
#define CONTEXT_RP_TOP_SHOWROOM                     6
#define CONTEXT_RP_TOP_FRIENDCHALLENGES             7
#define CONTEXT_RP_TOP_CAREER                       8
#define CONTEXT_RP_TOP_TIER0                        9
#define CONTEXT_RP_TOP_TIER1                        10
#define CONTEXT_RP_TOP_TIER2                        11
#define CONTEXT_RP_TOP_TIER3                        12
#define CONTEXT_RP_TOP_TIER4                        13
#define CONTEXT_RP_TOP_TIER5                        14
#define CONTEXT_RP_TOP_TIER6                        15
#define CONTEXT_RP_TOP_TIER7                        16
#define CONTEXT_RP_TOP_TIER8                        17
#define CONTEXT_RP_TOP_MULTIPLAYER                  18
#define CONTEXT_RP_TOP_SPLITSCREEN                  19
#define CONTEXT_RP_TOP_RACEHISTORY                  20
#define CONTEXT_RP_TOP_SYSTEMLINK                   21
#define CONTEXT_RP_TOP_STICKERBOOK                  22
#define CONTEXT_RP_TOP_ONLINEWITHRANK               23
#define CONTEXT_RP_TOP_ONLINEWITHLEGEND             24

// Values for CONTEXT_RP_BOTTOM

#define CONTEXT_RP_BOTTOM_BLANK                     0
#define CONTEXT_RP_BOTTOM_FRIENDCHALLENGE           1
#define CONTEXT_RP_BOTTOM_RECHALLENGE               2
#define CONTEXT_RP_BOTTOM_SPCHOOSINGEVENT           3
#define CONTEXT_RP_BOTTOM_SPRACING                  4
#define CONTEXT_RP_BOTTOM_SPDESTRUCTION             5
#define CONTEXT_RP_BOTTOM_SPCHECKPOINT              6
#define CONTEXT_RP_BOTTOM_SPBOSS0                   7
#define CONTEXT_RP_BOTTOM_SPBOSS1                   8
#define CONTEXT_RP_BOTTOM_SPBOSS2                   9
#define CONTEXT_RP_BOTTOM_SPBOSS3                   10
#define CONTEXT_RP_BOTTOM_SPBOSS4                   11
#define CONTEXT_RP_BOTTOM_SPBOSS5                   12
#define CONTEXT_RP_BOTTOM_SPBOSS6                   13
#define CONTEXT_RP_BOTTOM_SPBOSS7                   14
#define CONTEXT_RP_BOTTOM_SPBOSS8                   15
#define CONTEXT_RP_BOTTOM_ONLINEMAIN                16
#define CONTEXT_RP_BOTTOM_SHOWROOM                  17
#define CONTEXT_RP_BOTTOM_MODSHOP                   18
#define CONTEXT_RP_BOTTOM_CHALLENGES                19
#define CONTEXT_RP_BOTTOM_LEADERBOARDS              20
#define CONTEXT_RP_BOTTOM_PRIVATEGAME               21
#define CONTEXT_RP_BOTTOM_INLOBBY                   22
#define CONTEXT_RP_BOTTOM_ONLINERACING              23
#define CONTEXT_RP_BOTTOM_ONLINEMOTORMASH           24

// Values for CONTEXT_RP_LEGEND_ROMAN

#define CONTEXT_RP_LEGEND_ROMAN_I                   0
#define CONTEXT_RP_LEGEND_ROMAN_II                  1
#define CONTEXT_RP_LEGEND_ROMAN_III                 2
#define CONTEXT_RP_LEGEND_ROMAN_IV                  3
#define CONTEXT_RP_LEGEND_ROMAN_V                   4
#define CONTEXT_RP_LEGEND_ROMAN_VI                  5
#define CONTEXT_RP_LEGEND_ROMAN_VII                 6
#define CONTEXT_RP_LEGEND_ROMAN_VIII                7
#define CONTEXT_RP_LEGEND_ROMAN_IX                  8
#define CONTEXT_RP_LEGEND_ROMAN_X                   9

// Values for X_CONTEXT_PRESENCE

#define CONTEXT_PRESENCE_MAINPRESENCE               0
#define CONTEXT_PRESENCE_ONLINELEGEND               1
#define CONTEXT_PRESENCE_ONLINERANK                 2
#define CONTEXT_PRESENCE_MAINPRESENCE_ONELINE       3

// Values for X_CONTEXT_GAME_MODE

#define CONTEXT_GAME_MODE_PARTY                     0
#define CONTEXT_GAME_MODE_CUSTOMRACE                1
#define CONTEXT_GAME_MODE_SINGLEPLAYER              2

//
// Property ids
//
// These values are passed as the dwPropertyId value to XUserSetProperty
// and as the dwPropertyId value in the XUSER_PROPERTY structure.
//

#define PROPERTY_MM_GROUP_USER_CREATED              0x10000002
#define PROPERTY_MM_SLOTS                           0x10000003
#define PROPERTY_MM_SEARCHABLE                      0x10000004
#define PROPERTY_MM_RANK_RESTRICTION                0x10000006
#define PROPERTY_MM_MATCHING_MODE                   0x10000007
#define PROPERTY_MM_NAT                             0x1000000A
#define PROPERTY_MM_PACK_VERSION                    0x1000000B
#define PROPERTY_MM_MATCHING_REGION                 0x1000000C
#define PROPERTY_MM_MATCHING_ANY                    0x1000001D
#define PROPERTY_MM_INRACE                          0x1000001E
#define PROPERTY_RP_RANK                            0x1000001F
#define PROPERTY_MM_GROUP_ID                        0x20000001
#define PROPERTY_LB_RATING                          0x20000005
#define PROPERTY_LB_GAMERANK                        0x2000000D
#define PROPERTY_LB_TOTALFANS                       0x2000000E
#define PROPERTY_LB_TOTALTIMEPLAYED                 0x2000000F
#define PROPERTY_LB_RACEPOINTS                      0x20000010
#define PROPERTY_LB_TOPTHREE                        0x20000011
#define PROPERTY_LB_TOTALRACES                      0x20000012
#define PROPERTY_LB_RACEWINS                        0x20000013
#define PROPERTY_LB_POWERUPHITS                     0x20000014
#define PROPERTY_LB_POWERUPSFIRED                   0x20000015
#define PROPERTY_LB_CARSWRECKED                     0x20000016
#define PROPERTY_LB_LEGENDLEVEL                     0x20000017
#define PROPERTY_LB_TIMETOACHIEVELEGEND             0x20000018
#define PROPERTY_LB_LAPTIME                         0x20000019
#define PROPERTY_LB_COMPTIME                        0x2000001A
#define PROPERTY_LB_VEHICLE_ID                      0x2000001B
#define PROPERTY_LB_DIFFICULTY_LEVEL                0x2000001C

//
// Achievement ids
//
// These values are used in the dwAchievementId member of the
// XUSER_ACHIEVEMENT structure that is used with
// XUserWriteAchievements and XUserCreateAchievementEnumerator.
//

#define ACHIEVEMENT_ACHIEVEMENT_FIRSTFANPAR         61
#define ACHIEVEMENT_ACHIEVEMENT_FIRSTWRECK          62
#define ACHIEVEMENT_ACHIEVEMENT_SOCIALISER          63
#define ACHIEVEMENT_ACHIEVEMENT_LEADERBOARDCHALLENGE 64
#define ACHIEVEMENT_ACHIEVEMENT_FIRSTFANRUN         65
#define ACHIEVEMENT_ACHIEVEMENT_FIRSTEVADE          66
#define ACHIEVEMENT_ACHIEVEMENT_UNLOCKFIRSTBOSS     67
#define ACHIEVEMENT_ACHIEVEMENT_BEATSHANNON         68
#define ACHIEVEMENT_ACHIEVEMENT_WRECKBOSS           69
#define ACHIEVEMENT_ACHIEVEMENT_FRIENDDEMANDS       70
#define ACHIEVEMENT_ACHIEVEMENT_BEATKHAN            71
#define ACHIEVEMENT_ACHIEVEMENT_BOLTMASTER          72
#define ACHIEVEMENT_ACHIEVEMENT_BEATRHYMER          73
#define ACHIEVEMENT_ACHIEVEMENT_BARGEMASTER         74
#define ACHIEVEMENT_ACHIEVEMENT_NITROMASTER         75
#define ACHIEVEMENT_ACHIEVEMENT_TOURIST             76
#define ACHIEVEMENT_ACHIEVEMENT_BEATDRAKE           77
#define ACHIEVEMENT_ACHIEVEMENT_DESTRUCTION         78
#define ACHIEVEMENT_ACHIEVEMENT_CHECKPOINT          79
#define ACHIEVEMENT_ACHIEVEMENT_BEATNATALYA         80
#define ACHIEVEMENT_ACHIEVEMENT_SHOCKMASTER         81
#define ACHIEVEMENT_ACHIEVEMENT_BEATTALON           82
#define ACHIEVEMENT_ACHIEVEMENT_MINEMASTER          83
#define ACHIEVEMENT_ACHIEVEMENT_REPAIRMASTER        84
#define ACHIEVEMENT_ACHIEVEMENT_RACE                85
#define ACHIEVEMENT_ACHIEVEMENT_GLOBETROTTER        86
#define ACHIEVEMENT_ACHIEVEMENT_FANDEMANDMASTER     87
#define ACHIEVEMENT_ACHIEVEMENT_BEATTWINS           88
#define ACHIEVEMENT_ACHIEVEMENT_SHIELDMASTER        89
#define ACHIEVEMENT_ACHIEVEMENT_FANRUNS             90
#define ACHIEVEMENT_ACHIEVEMENT_BEATCARVER          91
#define ACHIEVEMENT_ACHIEVEMENT_HUMILIATEBOSSES     92
#define ACHIEVEMENT_ACHIEVEMENT_GOLDHARUNA          93
#define ACHIEVEMENT_ACHIEVEMENT_SHUNTMASTER         94
#define ACHIEVEMENT_ACHIEVEMENT_BOSSDEMANDS         95
#define ACHIEVEMENT_ACHIEVEMENT_BEATSHANNONFINAL    97
#define ACHIEVEMENT_ACHIEVEMENT_SPEEDDEMON          100
#define ACHIEVEMENT_ACHIEVEMENT_COMPLETIST          101
#define ACHIEVEMENT_ACHIEVEMENT_BEATGAMEHARD        102
#define ACHIEVEMENT_ACHIEVEMENT_GIANTKILLER         103
#define ACHIEVEMENT_ACHIEVEMENT_MPCHALLENGES        104
#define ACHIEVEMENT_ACHIEVEMENT_ALLROUNDER          105
#define ACHIEVEMENT_ACHIEVEMENT_PAINTJOB            106
#define ACHIEVEMENT_ACHIEVEMENT_RANK50              107
#define ACHIEVEMENT_ACHIEVEMENT_MPAWARDS            109
#define ACHIEVEMENT_ACHIEVEMENT_MODDER              110
#define ACHIEVEMENT_ACHIEVEMENT_PHOTOFINISH         111
#define ACHIEVEMENT_ACHIEVEMENT_BEATBIZARRE         113
#define ACHIEVEMENT_ACHIEVEMENT_MOTORMASH           114
#define ACHIEVEMENT_ACHIEVEMENT_SPLITSCREEN         115

//
// AvatarAssetAward ids
//

#define AVATARASSETAWARD_AVATAR_WRECKTEE                 5
#define AVATARASSETAWARD_AVATAR_ALLSTICKERSTEE           8
#define AVATARASSETAWARD_AVATAR_LEGENDTEE                9
#define AVATARASSETAWARD_AVATAR_NINTHBOSSTEE             10
#define AVATARASSETAWARD_AVATAR_RECHALLENGETEE           11

//
// Stats view ids
//
// These are used in the dwViewId member of the XUSER_STATS_SPEC structure
// passed to the XUserReadStats* and XUserCreateStatsEnumerator* functions.
//

// Skill leaderboards for ranked game modes

#define STATS_VIEW_SKILL_RANKED_PARTY               0xFFFF0000
#define STATS_VIEW_SKILL_RANKED_CUSTOMRACE          0xFFFF0001
#define STATS_VIEW_SKILL_RANKED_SINGLEPLAYER        0xFFFF0002

// Skill leaderboards for unranked (standard) game modes

#define STATS_VIEW_SKILL_STANDARD_PARTY             0xFFFE0000
#define STATS_VIEW_SKILL_STANDARD_CUSTOMRACE        0xFFFE0001
#define STATS_VIEW_SKILL_STANDARD_SINGLEPLAYER      0xFFFE0002

// Title defined leaderboards

#define STATS_VIEW_MP_RACEWINS                      1
#define STATS_VIEW_MP_POWERUPS                      2
#define STATS_VIEW_MP_LOBBY                         3
#define STATS_VIEW_MP_LEGENDS                       4
#define STATS_VIEW_SP_57_A                          5
#define STATS_VIEW_SP_36_A                          7
#define STATS_VIEW_SP_25_A                          8
#define STATS_VIEW_SP_14_A                          9
#define STATS_VIEW_MP_TOTALFANS                     10
#define STATS_VIEW_SP_61_A                          11
#define STATS_VIEW_SP_72_A                          28
#define STATS_VIEW_SP_80_A                          30
#define STATS_VIEW_SP_67_A                          31
#define STATS_VIEW_SP_71_A                          32
#define STATS_VIEW_SP_69_A                          33
#define STATS_VIEW_SP_75_A                          34
#define STATS_VIEW_SP_76_A                          35
#define STATS_VIEW_SP_68_A                          36
#define STATS_VIEW_SP_66_A                          37
#define STATS_VIEW_SP_83_A                          38
#define STATS_VIEW_SP_74_A                          39
#define STATS_VIEW_SP_70_A                          40
#define STATS_VIEW_SP_63_A                          42
#define STATS_VIEW_SP_64_A                          43
#define STATS_VIEW_SP_65_A                          44
#define STATS_VIEW_SP_62_A                          45
#define STATS_VIEW_SP_60_A                          47
#define STATS_VIEW_SP_59_A                          48
#define STATS_VIEW_SP_58_A                          49
#define STATS_VIEW_SP_56_A                          50
#define STATS_VIEW_SP_55_A                          51
#define STATS_VIEW_SP_54_A                          52
#define STATS_VIEW_SP_53_A                          53
#define STATS_VIEW_SP_52_A                          54
#define STATS_VIEW_SP_51_A                          55
#define STATS_VIEW_SP_50_A                          56
#define STATS_VIEW_SP_49_A                          57
#define STATS_VIEW_SP_48_A                          58
#define STATS_VIEW_SP_47_A                          59
#define STATS_VIEW_SP_46_A                          60
#define STATS_VIEW_SP_45_A                          61
#define STATS_VIEW_SP_44_A                          62
#define STATS_VIEW_SP_43_A                          63
#define STATS_VIEW_SP_42_A                          64
#define STATS_VIEW_SP_41_A                          65
#define STATS_VIEW_SP_40_A                          66
#define STATS_VIEW_SP_39_A                          67
#define STATS_VIEW_SP_38_A                          68
#define STATS_VIEW_SP_37_A                          69
#define STATS_VIEW_SP_35_A                          70
#define STATS_VIEW_SP_34_A                          71
#define STATS_VIEW_SP_33_A                          72
#define STATS_VIEW_SP_32_A                          73
#define STATS_VIEW_SP_31_A                          74
#define STATS_VIEW_SP_30_A                          75
#define STATS_VIEW_SP_29_A                          76
#define STATS_VIEW_SP_28_A                          77
#define STATS_VIEW_SP_27_A                          78
#define STATS_VIEW_SP_26_A                          79
#define STATS_VIEW_SP_24_A                          80
#define STATS_VIEW_SP_23_A                          81
#define STATS_VIEW_SP_22_A                          82
#define STATS_VIEW_SP_21_A                          83
#define STATS_VIEW_SP_20_A                          84
#define STATS_VIEW_SP_19_A                          85
#define STATS_VIEW_SP_18_A                          86
#define STATS_VIEW_SP_17_A                          87
#define STATS_VIEW_SP_16_A                          88
#define STATS_VIEW_SP_15_A                          89
#define STATS_VIEW_SP_13_A                          90
#define STATS_VIEW_SP_12_A                          91
#define STATS_VIEW_SP_11_A                          92
#define STATS_VIEW_SP_10_A                          93
#define STATS_VIEW_SP_9_A                           94
#define STATS_VIEW_SP_8_A                           95
#define STATS_VIEW_SP_7_A                           96
#define STATS_VIEW_SP_6_A                           97
#define STATS_VIEW_SP_5_A                           98
#define STATS_VIEW_SP_4_A                           99
#define STATS_VIEW_SP_1_A                           100
#define STATS_VIEW_SP_2_A                           101
#define STATS_VIEW_SP_3_A                           102
#define STATS_VIEW_SP_90_A                          260
#define STATS_VIEW_SP_87_A                          261
#define STATS_VIEW_SP_88_A                          262
#define STATS_VIEW_SP_86_A                          263
#define STATS_VIEW_SP_89_A                          266
#define STATS_VIEW_SP_78_A                          268
#define STATS_VIEW_SP_85_A                          269
#define STATS_VIEW_SP_84_A                          270
#define STATS_VIEW_SP_81_A                          271
#define STATS_VIEW_SP_79_A                          273
#define STATS_VIEW_SP_77_A                          274
#define STATS_VIEW_SP_73_A                          275
#define STATS_VIEW_SP_82_A                          276

//
// Stats view column ids
//
// These ids are used to read columns of stats views.  They are specified in
// the rgwColumnIds array of the XUSER_STATS_SPEC structure.  Rank, rating
// and gamertag are not retrieved as custom columns and so are not included
// in the following definitions.  They can be retrieved from each row's
// header (e.g., pStatsResults->pViews[x].pRows[y].dwRank, etc.).
//

// Column ids for MP_RACEWINS

#define STATS_COLUMN_MP_RACEWINS_MP_GAMERANK        1
#define STATS_COLUMN_MP_RACEWINS_MP_TOTALFANS       2
#define STATS_COLUMN_MP_RACEWINS_MP_TOTALTIMEPLAYED 3
#define STATS_COLUMN_MP_RACEWINS_MP_RACEPOINTS      4
#define STATS_COLUMN_MP_RACEWINS_MP_TOPTHREE        5
#define STATS_COLUMN_MP_RACEWINS_MP_TOTALRACES      6
#define STATS_COLUMN_MP_RACEWINS_MP_RACEWINS        7
#define STATS_COLUMN_MP_RACEWINS_MP_POWERUPHITS     8
#define STATS_COLUMN_MP_RACEWINS_MP_POWERUPSFIRED   9
#define STATS_COLUMN_MP_RACEWINS_MP_CARSWRECKED     10
#define STATS_COLUMN_MP_RACEWINS_MP_LEGENDLEVEL     11
#define STATS_COLUMN_MP_RACEWINS_MP_TIMETOACHIEVELEGEND 12

// Column ids for MP_POWERUPS

#define STATS_COLUMN_MP_POWERUPS_MP_GAMERANK        1
#define STATS_COLUMN_MP_POWERUPS_MP_TOTALFANS       2
#define STATS_COLUMN_MP_POWERUPS_MP_TOTALTIMEPLAYED 3
#define STATS_COLUMN_MP_POWERUPS_MP_RACEPOINTS      4
#define STATS_COLUMN_MP_POWERUPS_MP_TOPTHREE        5
#define STATS_COLUMN_MP_POWERUPS_MP_TOTALRACES      6
#define STATS_COLUMN_MP_POWERUPS_MP_RACEWINS        7
#define STATS_COLUMN_MP_POWERUPS_MP_POWERUPHITS     8
#define STATS_COLUMN_MP_POWERUPS_MP_POWERUPSFIRED   9
#define STATS_COLUMN_MP_POWERUPS_MP_CARSWRECKED     10
#define STATS_COLUMN_MP_POWERUPS_MP_LEGENDLEVEL     11
#define STATS_COLUMN_MP_POWERUPS_MP_TIMETOACHIEVELEGEND 12

// Column ids for MP_LOBBY

#define STATS_COLUMN_MP_LOBBY_MP_GAMERANK           1
#define STATS_COLUMN_MP_LOBBY_MP_TOTALFANS          2
#define STATS_COLUMN_MP_LOBBY_MP_TOTALTIMEPLAYED    3
#define STATS_COLUMN_MP_LOBBY_MP_RACEPOINTS         4
#define STATS_COLUMN_MP_LOBBY_MP_TOPTHREE           5
#define STATS_COLUMN_MP_LOBBY_MP_TOTALRACES         6
#define STATS_COLUMN_MP_LOBBY_MP_RACEWINS           7
#define STATS_COLUMN_MP_LOBBY_MP_POWERUPHITS        8
#define STATS_COLUMN_MP_LOBBY_MP_POWERUPSFIRED      9
#define STATS_COLUMN_MP_LOBBY_MP_CARSWRECKED        10
#define STATS_COLUMN_MP_LOBBY_MP_LEGENDLEVEL        11
#define STATS_COLUMN_MP_LOBBY_MP_TIMETOACHIEVELEGEND 12

// Column ids for MP_LEGENDS

#define STATS_COLUMN_MP_LEGENDS_MP_GAMERANK         1
#define STATS_COLUMN_MP_LEGENDS_MP_TOTALFANS        2
#define STATS_COLUMN_MP_LEGENDS_MP_TOTALTIMEPLAYED  3
#define STATS_COLUMN_MP_LEGENDS_MP_RACEPOINTS       4
#define STATS_COLUMN_MP_LEGENDS_MP_TOPTHREE         5
#define STATS_COLUMN_MP_LEGENDS_MP_TOTALRACES       6
#define STATS_COLUMN_MP_LEGENDS_MP_RACEWINS         7
#define STATS_COLUMN_MP_LEGENDS_MP_POWERUPHITS      8
#define STATS_COLUMN_MP_LEGENDS_MP_POWERUPSFIRED    9
#define STATS_COLUMN_MP_LEGENDS_MP_CARSWRECKED      10
#define STATS_COLUMN_MP_LEGENDS_MP_LEGENDLEVEL      11
#define STATS_COLUMN_MP_LEGENDS_MP_TIMETOACHIEVELEGEND 12

// Column ids for SP_57_A

#define STATS_COLUMN_SP_57_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_57_A_LAP_TIME               2
#define STATS_COLUMN_SP_57_A_COMP_TIME              3
#define STATS_COLUMN_SP_57_A_WRECKS                 4
#define STATS_COLUMN_SP_57_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_57_A_DIFFICULTY_LEVEL       6

// Column ids for SP_36_A

#define STATS_COLUMN_SP_36_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_36_A_LAP_TIME               2
#define STATS_COLUMN_SP_36_A_COMP_TIME              3
#define STATS_COLUMN_SP_36_A_WRECKS                 4
#define STATS_COLUMN_SP_36_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_36_A_DIFFICULTY_LEVEL       6

// Column ids for SP_25_A

#define STATS_COLUMN_SP_25_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_25_A_LAP_TIME               2
#define STATS_COLUMN_SP_25_A_COMP_TIME              3
#define STATS_COLUMN_SP_25_A_WRECKS                 4
#define STATS_COLUMN_SP_25_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_25_A_DIFFICULTY_LEVEL       6

// Column ids for SP_14_A

#define STATS_COLUMN_SP_14_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_14_A_LAP_TIME               2
#define STATS_COLUMN_SP_14_A_COMP_TIME              3
#define STATS_COLUMN_SP_14_A_WRECKS                 4
#define STATS_COLUMN_SP_14_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_14_A_DIFFICULTY_LEVEL       6

// Column ids for MP_TOTALFANS

#define STATS_COLUMN_MP_TOTALFANS_MP_GAMERANK       1
#define STATS_COLUMN_MP_TOTALFANS_MP_TOTALFANS      2
#define STATS_COLUMN_MP_TOTALFANS_MP_TOTALTIMEPLAYED 3
#define STATS_COLUMN_MP_TOTALFANS_MP_RACEPOINTS     4
#define STATS_COLUMN_MP_TOTALFANS_MP_TOPTHREE       5
#define STATS_COLUMN_MP_TOTALFANS_MP_TOTALRACES     6
#define STATS_COLUMN_MP_TOTALFANS_MP_RACEWINS       7
#define STATS_COLUMN_MP_TOTALFANS_MP_POWERUPHITS    8
#define STATS_COLUMN_MP_TOTALFANS_MP_POWERUPSFIRED  9
#define STATS_COLUMN_MP_TOTALFANS_MP_CARSWRECKED    10
#define STATS_COLUMN_MP_TOTALFANS_MP_LEGENDLEVEL    11
#define STATS_COLUMN_MP_TOTALFANS_MP_TIMETOACHIEVELEGEND 12

// Column ids for SP_61_A

#define STATS_COLUMN_SP_61_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_61_A_LAP_TIME               2
#define STATS_COLUMN_SP_61_A_COMP_TIME              3
#define STATS_COLUMN_SP_61_A_WRECKS                 4
#define STATS_COLUMN_SP_61_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_61_A_DIFFICULTY_LEVEL       6

// Column ids for SP_72_A

#define STATS_COLUMN_SP_72_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_72_A_LAP_TIME               2
#define STATS_COLUMN_SP_72_A_COMP_TIME              3
#define STATS_COLUMN_SP_72_A_WRECKS                 4
#define STATS_COLUMN_SP_72_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_72_A_DIFFICULTY_LEVEL       6

// Column ids for SP_80_A

#define STATS_COLUMN_SP_80_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_80_A_LAP_TIME               2
#define STATS_COLUMN_SP_80_A_COMP_TIME              3
#define STATS_COLUMN_SP_80_A_WRECKS                 4
#define STATS_COLUMN_SP_80_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_80_A_DIFFICULTY_LEVEL       6

// Column ids for SP_67_A

#define STATS_COLUMN_SP_67_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_67_A_LAP_TIME               2
#define STATS_COLUMN_SP_67_A_COMP_TIME              3
#define STATS_COLUMN_SP_67_A_WRECKS                 4
#define STATS_COLUMN_SP_67_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_67_A_DIFFICULTY_LEVEL       6

// Column ids for SP_71_A

#define STATS_COLUMN_SP_71_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_71_A_LAP_TIME               2
#define STATS_COLUMN_SP_71_A_COMP_TIME              3
#define STATS_COLUMN_SP_71_A_WRECKS                 4
#define STATS_COLUMN_SP_71_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_71_A_DIFFICULTY_LEVEL       6

// Column ids for SP_69_A

#define STATS_COLUMN_SP_69_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_69_A_LAP_TIME               2
#define STATS_COLUMN_SP_69_A_COMP_TIME              3
#define STATS_COLUMN_SP_69_A_WRECKS                 4
#define STATS_COLUMN_SP_69_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_69_A_DIFFICULTY_LEVEL       6

// Column ids for SP_75_A

#define STATS_COLUMN_SP_75_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_75_A_LAP_TIME               2
#define STATS_COLUMN_SP_75_A_COMP_TIME              3
#define STATS_COLUMN_SP_75_A_WRECKS                 4
#define STATS_COLUMN_SP_75_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_75_A_DIFFICULTY_LEVEL       6

// Column ids for SP_76_A

#define STATS_COLUMN_SP_76_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_76_A_LAP_TIME               2
#define STATS_COLUMN_SP_76_A_COMP_TIME              3
#define STATS_COLUMN_SP_76_A_WRECKS                 4
#define STATS_COLUMN_SP_76_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_76_A_DIFFICULTY_LEVEL       6

// Column ids for SP_68_A

#define STATS_COLUMN_SP_68_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_68_A_LAP_TIME               2
#define STATS_COLUMN_SP_68_A_COMP_TIME              3
#define STATS_COLUMN_SP_68_A_WRECKS                 4
#define STATS_COLUMN_SP_68_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_68_A_DIFFICULTY_LEVEL       6

// Column ids for SP_66_A

#define STATS_COLUMN_SP_66_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_66_A_LAP_TIME               2
#define STATS_COLUMN_SP_66_A_COMP_TIME              3
#define STATS_COLUMN_SP_66_A_WRECKS                 4
#define STATS_COLUMN_SP_66_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_66_A_DIFFICULTY_LEVEL       6

// Column ids for SP_83_A

#define STATS_COLUMN_SP_83_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_83_A_LAP_TIME               2
#define STATS_COLUMN_SP_83_A_COMP_TIME              3
#define STATS_COLUMN_SP_83_A_WRECKS                 4
#define STATS_COLUMN_SP_83_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_83_A_DIFFICULTY_LEVEL       6

// Column ids for SP_74_A

#define STATS_COLUMN_SP_74_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_74_A_LAP_TIME               2
#define STATS_COLUMN_SP_74_A_COMP_TIME              3
#define STATS_COLUMN_SP_74_A_WRECKS                 4
#define STATS_COLUMN_SP_74_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_74_A_DIFFICULTY_LEVEL       6

// Column ids for SP_70_A

#define STATS_COLUMN_SP_70_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_70_A_LAP_TIME               2
#define STATS_COLUMN_SP_70_A_COMP_TIME              3
#define STATS_COLUMN_SP_70_A_WRECKS                 4
#define STATS_COLUMN_SP_70_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_70_A_DIFFICULTY_LEVEL       6

// Column ids for SP_63_A

#define STATS_COLUMN_SP_63_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_63_A_LAP_TIME               2
#define STATS_COLUMN_SP_63_A_COMP_TIME              3
#define STATS_COLUMN_SP_63_A_WRECKS                 4
#define STATS_COLUMN_SP_63_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_63_A_DIFFICULTY_LEVEL       6

// Column ids for SP_64_A

#define STATS_COLUMN_SP_64_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_64_A_LAP_TIME               2
#define STATS_COLUMN_SP_64_A_COMP_TIME              3
#define STATS_COLUMN_SP_64_A_WRECKS                 4
#define STATS_COLUMN_SP_64_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_64_A_DIFFICULTY_LEVEL       6

// Column ids for SP_65_A

#define STATS_COLUMN_SP_65_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_65_A_LAP_TIME               2
#define STATS_COLUMN_SP_65_A_COMP_TIME              3
#define STATS_COLUMN_SP_65_A_WRECKS                 4
#define STATS_COLUMN_SP_65_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_65_A_DIFFICULTY_LEVEL       6

// Column ids for SP_62_A

#define STATS_COLUMN_SP_62_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_62_A_LAP_TIME               2
#define STATS_COLUMN_SP_62_A_COMP_TIME              3
#define STATS_COLUMN_SP_62_A_WRECKS                 4
#define STATS_COLUMN_SP_62_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_62_A_DIFFICULTY_LEVEL       6

// Column ids for SP_60_A

#define STATS_COLUMN_SP_60_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_60_A_LAP_TIME               2
#define STATS_COLUMN_SP_60_A_COMP_TIME              3
#define STATS_COLUMN_SP_60_A_WRECKS                 4
#define STATS_COLUMN_SP_60_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_60_A_DIFFICULTY_LEVEL       6

// Column ids for SP_59_A

#define STATS_COLUMN_SP_59_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_59_A_LAP_TIME               2
#define STATS_COLUMN_SP_59_A_COMP_TIME              3
#define STATS_COLUMN_SP_59_A_WRECKS                 4
#define STATS_COLUMN_SP_59_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_59_A_DIFFICULTY_LEVEL       6

// Column ids for SP_58_A

#define STATS_COLUMN_SP_58_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_58_A_LAP_TIME               2
#define STATS_COLUMN_SP_58_A_COMP_TIME              3
#define STATS_COLUMN_SP_58_A_WRECKS                 4
#define STATS_COLUMN_SP_58_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_58_A_DIFFICULTY_LEVEL       6

// Column ids for SP_56_A

#define STATS_COLUMN_SP_56_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_56_A_LAP_TIME               2
#define STATS_COLUMN_SP_56_A_COMP_TIME              3
#define STATS_COLUMN_SP_56_A_WRECKS                 4
#define STATS_COLUMN_SP_56_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_56_A_DIFFICULTY_LEVEL       6

// Column ids for SP_55_A

#define STATS_COLUMN_SP_55_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_55_A_LAP_TIME               2
#define STATS_COLUMN_SP_55_A_COMP_TIME              3
#define STATS_COLUMN_SP_55_A_WRECKS                 4
#define STATS_COLUMN_SP_55_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_55_A_DIFFICULTY_LEVEL       6

// Column ids for SP_54_A

#define STATS_COLUMN_SP_54_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_54_A_LAP_TIME               2
#define STATS_COLUMN_SP_54_A_COMP_TIME              3
#define STATS_COLUMN_SP_54_A_WRECKS                 4
#define STATS_COLUMN_SP_54_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_54_A_DIFFICULTY_LEVEL       6

// Column ids for SP_53_A

#define STATS_COLUMN_SP_53_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_53_A_LAP_TIME               2
#define STATS_COLUMN_SP_53_A_COMP_TIME              3
#define STATS_COLUMN_SP_53_A_WRECKS                 4
#define STATS_COLUMN_SP_53_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_53_A_DIFFICULTY_LEVEL       6

// Column ids for SP_52_A

#define STATS_COLUMN_SP_52_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_52_A_LAP_TIME               2
#define STATS_COLUMN_SP_52_A_COMP_TIME              3
#define STATS_COLUMN_SP_52_A_WRECKS                 4
#define STATS_COLUMN_SP_52_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_52_A_DIFFICULTY_LEVEL       6

// Column ids for SP_51_A

#define STATS_COLUMN_SP_51_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_51_A_LAP_TIME               2
#define STATS_COLUMN_SP_51_A_COMP_TIME              3
#define STATS_COLUMN_SP_51_A_WRECKS                 4
#define STATS_COLUMN_SP_51_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_51_A_DIFFICULTY_LEVEL       6

// Column ids for SP_50_A

#define STATS_COLUMN_SP_50_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_50_A_LAP_TIME               2
#define STATS_COLUMN_SP_50_A_COMP_TIME              3
#define STATS_COLUMN_SP_50_A_WRECKS                 4
#define STATS_COLUMN_SP_50_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_50_A_DIFFICULTY_LEVEL       6

// Column ids for SP_49_A

#define STATS_COLUMN_SP_49_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_49_A_LAP_TIME               2
#define STATS_COLUMN_SP_49_A_COMP_TIME              3
#define STATS_COLUMN_SP_49_A_WRECKS                 4
#define STATS_COLUMN_SP_49_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_49_A_DIFFICULTY_LEVEL       6

// Column ids for SP_48_A

#define STATS_COLUMN_SP_48_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_48_A_LAP_TIME               2
#define STATS_COLUMN_SP_48_A_COMP_TIME              3
#define STATS_COLUMN_SP_48_A_WRECKS                 4
#define STATS_COLUMN_SP_48_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_48_A_DIFFICULTY_LEVEL       6

// Column ids for SP_47_A

#define STATS_COLUMN_SP_47_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_47_A_LAP_TIME               2
#define STATS_COLUMN_SP_47_A_COMP_TIME              3
#define STATS_COLUMN_SP_47_A_WRECKS                 4
#define STATS_COLUMN_SP_47_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_47_A_DIFFICULTY_LEVEL       6

// Column ids for SP_46_A

#define STATS_COLUMN_SP_46_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_46_A_LAP_TIME               2
#define STATS_COLUMN_SP_46_A_COMP_TIME              3
#define STATS_COLUMN_SP_46_A_WRECKS                 4
#define STATS_COLUMN_SP_46_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_46_A_DIFFICULTY_LEVEL       6

// Column ids for SP_45_A

#define STATS_COLUMN_SP_45_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_45_A_LAP_TIME               2
#define STATS_COLUMN_SP_45_A_COMP_TIME              3
#define STATS_COLUMN_SP_45_A_WRECKS                 4
#define STATS_COLUMN_SP_45_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_45_A_DIFFICULTY_LEVEL       6

// Column ids for SP_44_A

#define STATS_COLUMN_SP_44_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_44_A_LAP_TIME               2
#define STATS_COLUMN_SP_44_A_COMP_TIME              3
#define STATS_COLUMN_SP_44_A_WRECKS                 4
#define STATS_COLUMN_SP_44_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_44_A_DIFFICULTY_LEVEL       6

// Column ids for SP_43_A

#define STATS_COLUMN_SP_43_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_43_A_LAP_TIME               2
#define STATS_COLUMN_SP_43_A_COMP_TIME              3
#define STATS_COLUMN_SP_43_A_WRECKS                 4
#define STATS_COLUMN_SP_43_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_43_A_DIFFICULTY_LEVEL       6

// Column ids for SP_42_A

#define STATS_COLUMN_SP_42_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_42_A_LAP_TIME               2
#define STATS_COLUMN_SP_42_A_COMP_TIME              3
#define STATS_COLUMN_SP_42_A_WRECKS                 4
#define STATS_COLUMN_SP_42_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_42_A_DIFFICULTY_LEVEL       6

// Column ids for SP_41_A

#define STATS_COLUMN_SP_41_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_41_A_LAP_TIME               2
#define STATS_COLUMN_SP_41_A_COMP_TIME              3
#define STATS_COLUMN_SP_41_A_WRECKS                 4
#define STATS_COLUMN_SP_41_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_41_A_DIFFICULTY_LEVEL       6

// Column ids for SP_40_A

#define STATS_COLUMN_SP_40_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_40_A_LAP_TIME               2
#define STATS_COLUMN_SP_40_A_COMP_TIME              3
#define STATS_COLUMN_SP_40_A_WRECKS                 4
#define STATS_COLUMN_SP_40_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_40_A_DIFFICULTY_LEVEL       6

// Column ids for SP_39_A

#define STATS_COLUMN_SP_39_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_39_A_LAP_TIME               2
#define STATS_COLUMN_SP_39_A_COMP_TIME              3
#define STATS_COLUMN_SP_39_A_WRECKS                 4
#define STATS_COLUMN_SP_39_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_39_A_DIFFICULTY_LEVEL       6

// Column ids for SP_38_A

#define STATS_COLUMN_SP_38_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_38_A_LAP_TIME               2
#define STATS_COLUMN_SP_38_A_COMP_TIME              3
#define STATS_COLUMN_SP_38_A_WRECKS                 4
#define STATS_COLUMN_SP_38_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_38_A_DIFFICULTY_LEVEL       6

// Column ids for SP_37_A

#define STATS_COLUMN_SP_37_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_37_A_LAP_TIME               2
#define STATS_COLUMN_SP_37_A_COMP_TIME              3
#define STATS_COLUMN_SP_37_A_WRECKS                 4
#define STATS_COLUMN_SP_37_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_37_A_DIFFICULTY_LEVEL       6

// Column ids for SP_35_A

#define STATS_COLUMN_SP_35_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_35_A_LAP_TIME               2
#define STATS_COLUMN_SP_35_A_COMP_TIME              3
#define STATS_COLUMN_SP_35_A_WRECKS                 4
#define STATS_COLUMN_SP_35_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_35_A_DIFFICULTY_LEVEL       6

// Column ids for SP_34_A

#define STATS_COLUMN_SP_34_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_34_A_LAP_TIME               2
#define STATS_COLUMN_SP_34_A_COMP_TIME              3
#define STATS_COLUMN_SP_34_A_WRECKS                 4
#define STATS_COLUMN_SP_34_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_34_A_DIFFICULTY_LEVEL       6

// Column ids for SP_33_A

#define STATS_COLUMN_SP_33_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_33_A_LAP_TIME               2
#define STATS_COLUMN_SP_33_A_COMP_TIME              3
#define STATS_COLUMN_SP_33_A_WRECKS                 4
#define STATS_COLUMN_SP_33_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_33_A_DIFFICULTY_LEVEL       6

// Column ids for SP_32_A

#define STATS_COLUMN_SP_32_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_32_A_LAP_TIME               2
#define STATS_COLUMN_SP_32_A_COMP_TIME              3
#define STATS_COLUMN_SP_32_A_WRECKS                 4
#define STATS_COLUMN_SP_32_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_32_A_DIFFICULTY_LEVEL       6

// Column ids for SP_31_A

#define STATS_COLUMN_SP_31_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_31_A_LAP_TIME               2
#define STATS_COLUMN_SP_31_A_COMP_TIME              3
#define STATS_COLUMN_SP_31_A_WRECKS                 4
#define STATS_COLUMN_SP_31_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_31_A_DIFFICULTY_LEVEL       6

// Column ids for SP_30_A

#define STATS_COLUMN_SP_30_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_30_A_LAP_TIME               2
#define STATS_COLUMN_SP_30_A_COMP_TIME              3
#define STATS_COLUMN_SP_30_A_WRECKS                 4
#define STATS_COLUMN_SP_30_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_30_A_DIFFICULTY_LEVEL       6

// Column ids for SP_29_A

#define STATS_COLUMN_SP_29_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_29_A_LAP_TIME               2
#define STATS_COLUMN_SP_29_A_COMP_TIME              3
#define STATS_COLUMN_SP_29_A_WRECKS                 4
#define STATS_COLUMN_SP_29_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_29_A_DIFFICULTY_LEVEL       6

// Column ids for SP_28_A

#define STATS_COLUMN_SP_28_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_28_A_LAP_TIME               2
#define STATS_COLUMN_SP_28_A_COMP_TIME              3
#define STATS_COLUMN_SP_28_A_WRECKS                 4
#define STATS_COLUMN_SP_28_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_28_A_DIFFICULTY_LEVEL       6

// Column ids for SP_27_A

#define STATS_COLUMN_SP_27_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_27_A_LAP_TIME               2
#define STATS_COLUMN_SP_27_A_COMP_TIME              3
#define STATS_COLUMN_SP_27_A_WRECKS                 4
#define STATS_COLUMN_SP_27_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_27_A_DIFFICULTY_LEVEL       6

// Column ids for SP_26_A

#define STATS_COLUMN_SP_26_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_26_A_LAP_TIME               2
#define STATS_COLUMN_SP_26_A_COMP_TIME              3
#define STATS_COLUMN_SP_26_A_WRECKS                 4
#define STATS_COLUMN_SP_26_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_26_A_DIFFICULTY_LEVEL       6

// Column ids for SP_24_A

#define STATS_COLUMN_SP_24_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_24_A_LAP_TIME               2
#define STATS_COLUMN_SP_24_A_COMP_TIME              3
#define STATS_COLUMN_SP_24_A_WRECKS                 4
#define STATS_COLUMN_SP_24_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_24_A_DIFFICULTY_LEVEL       6

// Column ids for SP_23_A

#define STATS_COLUMN_SP_23_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_23_A_LAP_TIME               2
#define STATS_COLUMN_SP_23_A_COMP_TIME              3
#define STATS_COLUMN_SP_23_A_WRECKS                 4
#define STATS_COLUMN_SP_23_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_23_A_DIFFICULTY_LEVEL       6

// Column ids for SP_22_A

#define STATS_COLUMN_SP_22_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_22_A_LAP_TIME               2
#define STATS_COLUMN_SP_22_A_COMP_TIME              3
#define STATS_COLUMN_SP_22_A_WRECKS                 4
#define STATS_COLUMN_SP_22_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_22_A_DIFFICULTY_LEVEL       6

// Column ids for SP_21_A

#define STATS_COLUMN_SP_21_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_21_A_LAP_TIME               2
#define STATS_COLUMN_SP_21_A_COMP_TIME              3
#define STATS_COLUMN_SP_21_A_WRECKS                 4
#define STATS_COLUMN_SP_21_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_21_A_DIFFICULTY_LEVEL       6

// Column ids for SP_20_A

#define STATS_COLUMN_SP_20_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_20_A_LAP_TIME               2
#define STATS_COLUMN_SP_20_A_COMP_TIME              3
#define STATS_COLUMN_SP_20_A_WRECKS                 4
#define STATS_COLUMN_SP_20_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_20_A_DIFFICULTY_LEVEL       6

// Column ids for SP_19_A

#define STATS_COLUMN_SP_19_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_19_A_LAP_TIME               2
#define STATS_COLUMN_SP_19_A_COMP_TIME              3
#define STATS_COLUMN_SP_19_A_WRECKS                 4
#define STATS_COLUMN_SP_19_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_19_A_DIFFICULTY_LEVEL       6

// Column ids for SP_18_A

#define STATS_COLUMN_SP_18_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_18_A_LAP_TIME               2
#define STATS_COLUMN_SP_18_A_COMP_TIME              3
#define STATS_COLUMN_SP_18_A_WRECKS                 4
#define STATS_COLUMN_SP_18_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_18_A_DIFFICULTY_LEVEL       6

// Column ids for SP_17_A

#define STATS_COLUMN_SP_17_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_17_A_LAP_TIME               2
#define STATS_COLUMN_SP_17_A_COMP_TIME              3
#define STATS_COLUMN_SP_17_A_WRECKS                 4
#define STATS_COLUMN_SP_17_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_17_A_DIFFICULTY_LEVEL       6

// Column ids for SP_16_A

#define STATS_COLUMN_SP_16_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_16_A_LAP_TIME               2
#define STATS_COLUMN_SP_16_A_COMP_TIME              3
#define STATS_COLUMN_SP_16_A_WRECKS                 4
#define STATS_COLUMN_SP_16_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_16_A_DIFFICULTY_LEVEL       6

// Column ids for SP_15_A

#define STATS_COLUMN_SP_15_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_15_A_LAP_TIME               2
#define STATS_COLUMN_SP_15_A_COMP_TIME              3
#define STATS_COLUMN_SP_15_A_WRECKS                 4
#define STATS_COLUMN_SP_15_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_15_A_DIFFICULTY_LEVEL       6

// Column ids for SP_13_A

#define STATS_COLUMN_SP_13_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_13_A_LAP_TIME               2
#define STATS_COLUMN_SP_13_A_COMP_TIME              3
#define STATS_COLUMN_SP_13_A_WRECKS                 4
#define STATS_COLUMN_SP_13_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_13_A_DIFFICULTY_LEVEL       6

// Column ids for SP_12_A

#define STATS_COLUMN_SP_12_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_12_A_LAP_TIME               2
#define STATS_COLUMN_SP_12_A_COMP_TIME              3
#define STATS_COLUMN_SP_12_A_WRECKS                 4
#define STATS_COLUMN_SP_12_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_12_A_DIFFICULTY_LEVEL       6

// Column ids for SP_11_A

#define STATS_COLUMN_SP_11_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_11_A_LAP_TIME               2
#define STATS_COLUMN_SP_11_A_COMP_TIME              3
#define STATS_COLUMN_SP_11_A_WRECKS                 4
#define STATS_COLUMN_SP_11_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_11_A_DIFFICULTY_LEVEL       6

// Column ids for SP_10_A

#define STATS_COLUMN_SP_10_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_10_A_LAP_TIME               2
#define STATS_COLUMN_SP_10_A_COMP_TIME              3
#define STATS_COLUMN_SP_10_A_WRECKS                 4
#define STATS_COLUMN_SP_10_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_10_A_DIFFICULTY_LEVEL       6

// Column ids for SP_9_A

#define STATS_COLUMN_SP_9_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_9_A_LAP_TIME                2
#define STATS_COLUMN_SP_9_A_COMP_TIME               3
#define STATS_COLUMN_SP_9_A_WRECKS                  4
#define STATS_COLUMN_SP_9_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_9_A_DIFFICULTY_LEVEL        6

// Column ids for SP_8_A

#define STATS_COLUMN_SP_8_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_8_A_LAP_TIME                2
#define STATS_COLUMN_SP_8_A_COMP_TIME               3
#define STATS_COLUMN_SP_8_A_WRECKS                  4
#define STATS_COLUMN_SP_8_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_8_A_DIFFICULTY_LEVEL        6

// Column ids for SP_7_A

#define STATS_COLUMN_SP_7_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_7_A_LAP_TIME                2
#define STATS_COLUMN_SP_7_A_COMP_TIME               3
#define STATS_COLUMN_SP_7_A_WRECKS                  4
#define STATS_COLUMN_SP_7_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_7_A_DIFFICULTY_LEVEL        6

// Column ids for SP_6_A

#define STATS_COLUMN_SP_6_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_6_A_LAP_TIME                2
#define STATS_COLUMN_SP_6_A_COMP_TIME               3
#define STATS_COLUMN_SP_6_A_WRECKS                  4
#define STATS_COLUMN_SP_6_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_6_A_DIFFICULTY_LEVEL        6

// Column ids for SP_5_A

#define STATS_COLUMN_SP_5_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_5_A_LAP_TIME                2
#define STATS_COLUMN_SP_5_A_COMP_TIME               3
#define STATS_COLUMN_SP_5_A_WRECKS                  4
#define STATS_COLUMN_SP_5_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_5_A_DIFFICULTY_LEVEL        6

// Column ids for SP_4_A

#define STATS_COLUMN_SP_4_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_4_A_LAP_TIME                2
#define STATS_COLUMN_SP_4_A_COMP_TIME               3
#define STATS_COLUMN_SP_4_A_WRECKS                  4
#define STATS_COLUMN_SP_4_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_4_A_DIFFICULTY_LEVEL        6

// Column ids for SP_1_A

#define STATS_COLUMN_SP_1_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_1_A_LAP_TIME                2
#define STATS_COLUMN_SP_1_A_COMP_TIME               3
#define STATS_COLUMN_SP_1_A_WRECKS                  4
#define STATS_COLUMN_SP_1_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_1_A_DIFFICULTY_LEVEL        6

// Column ids for SP_2_A

#define STATS_COLUMN_SP_2_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_2_A_LAP_TIME                2
#define STATS_COLUMN_SP_2_A_COMP_TIME               3
#define STATS_COLUMN_SP_2_A_WRECKS                  4
#define STATS_COLUMN_SP_2_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_2_A_DIFFICULTY_LEVEL        6

// Column ids for SP_3_A

#define STATS_COLUMN_SP_3_A_TOTAL_FANS              1
#define STATS_COLUMN_SP_3_A_LAP_TIME                2
#define STATS_COLUMN_SP_3_A_COMP_TIME               3
#define STATS_COLUMN_SP_3_A_WRECKS                  4
#define STATS_COLUMN_SP_3_A_VEHICLE_ID              5
#define STATS_COLUMN_SP_3_A_DIFFICULTY_LEVEL        6

// Column ids for SP_90_A

#define STATS_COLUMN_SP_90_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_90_A_LAP_TIME               2
#define STATS_COLUMN_SP_90_A_COMP_TIME              3
#define STATS_COLUMN_SP_90_A_WRECKS                 4
#define STATS_COLUMN_SP_90_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_90_A_DIFFICULTY_LEVEL       6

// Column ids for SP_87_A

#define STATS_COLUMN_SP_87_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_87_A_LAP_TIME               2
#define STATS_COLUMN_SP_87_A_COMP_TIME              3
#define STATS_COLUMN_SP_87_A_WRECKS                 4
#define STATS_COLUMN_SP_87_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_87_A_DIFFICULTY_LEVEL       6

// Column ids for SP_88_A

#define STATS_COLUMN_SP_88_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_88_A_LAP_TIME               2
#define STATS_COLUMN_SP_88_A_COMP_TIME              3
#define STATS_COLUMN_SP_88_A_WRECKS                 4
#define STATS_COLUMN_SP_88_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_88_A_DIFFICULTY_LEVEL       6

// Column ids for SP_86_A

#define STATS_COLUMN_SP_86_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_86_A_LAP_TIME               2
#define STATS_COLUMN_SP_86_A_COMP_TIME              3
#define STATS_COLUMN_SP_86_A_WRECKS                 4
#define STATS_COLUMN_SP_86_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_86_A_DIFFICULTY_LEVEL       6

// Column ids for SP_89_A

#define STATS_COLUMN_SP_89_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_89_A_LAP_TIME               2
#define STATS_COLUMN_SP_89_A_COMP_TIME              3
#define STATS_COLUMN_SP_89_A_WRECKS                 4
#define STATS_COLUMN_SP_89_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_89_A_DIFFICULTY_LEVEL       6

// Column ids for SP_78_A

#define STATS_COLUMN_SP_78_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_78_A_LAP_TIME               2
#define STATS_COLUMN_SP_78_A_COMP_TIME              3
#define STATS_COLUMN_SP_78_A_WRECKS                 4
#define STATS_COLUMN_SP_78_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_78_A_DIFFICULTY_LEVEL       6

// Column ids for SP_85_A

#define STATS_COLUMN_SP_85_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_85_A_LAP_TIME               2
#define STATS_COLUMN_SP_85_A_COMP_TIME              3
#define STATS_COLUMN_SP_85_A_WRECKS                 4
#define STATS_COLUMN_SP_85_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_85_A_DIFFICULTY_LEVEL       6

// Column ids for SP_84_A

#define STATS_COLUMN_SP_84_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_84_A_LAP_TIME               2
#define STATS_COLUMN_SP_84_A_COMP_TIME              3
#define STATS_COLUMN_SP_84_A_WRECKS                 4
#define STATS_COLUMN_SP_84_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_84_A_DIFFICULTY_LEVEL       6

// Column ids for SP_81_A

#define STATS_COLUMN_SP_81_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_81_A_LAP_TIME               2
#define STATS_COLUMN_SP_81_A_COMP_TIME              3
#define STATS_COLUMN_SP_81_A_WRECKS                 4
#define STATS_COLUMN_SP_81_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_81_A_DIFFICULTY_LEVEL       6

// Column ids for SP_79_A

#define STATS_COLUMN_SP_79_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_79_A_LAP_TIME               2
#define STATS_COLUMN_SP_79_A_COMP_TIME              3
#define STATS_COLUMN_SP_79_A_WRECKS                 4
#define STATS_COLUMN_SP_79_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_79_A_DIFFICULTY_LEVEL       6

// Column ids for SP_77_A

#define STATS_COLUMN_SP_77_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_77_A_LAP_TIME               2
#define STATS_COLUMN_SP_77_A_COMP_TIME              3
#define STATS_COLUMN_SP_77_A_WRECKS                 4
#define STATS_COLUMN_SP_77_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_77_A_DIFFICULTY_LEVEL       6

// Column ids for SP_73_A

#define STATS_COLUMN_SP_73_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_73_A_LAP_TIME               2
#define STATS_COLUMN_SP_73_A_COMP_TIME              3
#define STATS_COLUMN_SP_73_A_WRECKS                 4
#define STATS_COLUMN_SP_73_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_73_A_DIFFICULTY_LEVEL       6

// Column ids for SP_82_A

#define STATS_COLUMN_SP_82_A_TOTAL_FANS             1
#define STATS_COLUMN_SP_82_A_LAP_TIME               2
#define STATS_COLUMN_SP_82_A_COMP_TIME              3
#define STATS_COLUMN_SP_82_A_WRECKS                 4
#define STATS_COLUMN_SP_82_A_VEHICLE_ID             5
#define STATS_COLUMN_SP_82_A_DIFFICULTY_LEVEL       6

//
// Matchmaking queries
//
// These values are passed as the dwProcedureIndex parameter to
// XSessionSearch to indicate which matchmaking query to run.
//

#define SESSION_MATCH_QUERY_DEFAULT                 0

//
// Gamer pictures
//
// These ids are passed as the dwPictureId parameter to XUserAwardGamerTile.
//

#define GAMER_PICTURE_GOLDMEDAL2                    44
#define GAMER_PICTURE_GOLDMEDAL                     51


#ifdef __cplusplus
}
#endif

#endif // __BLUR_SPA_H__


