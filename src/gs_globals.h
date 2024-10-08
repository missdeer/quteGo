/*
 * gs_globals.h
 */

#ifndef IGS_GLOBALS_H
#define IGS_GLOBALS_H

#define DEFAULT_HOST "igs.joyjoy.net"
#define DEFAULT_PORT 6969

// enum RankType {kyu, dan, pro, NR};
enum class Status
{
    guest,
    registered,
    offline
};
enum InfoType
{
    PLAYER,
    GAME,
    MESSAGE,
    YOUHAVEMSG,
    SERVERNAME,
    ACCOUNT,
    STATUS,
    IT_OTHER,
    CMD,
    READY,
    NOCLIENTMODE,
    TELL,
    KIBITZ,
    MOVE,
    BEEP,
    WHO,
    STATS,
    GAMES,
    NONE,
    HELP,
    CHANNELS,
    SHOUT,
    PLAYER27,
    PLAYER27_START,
    PLAYER27_END,
    GAME7,
    GAME7_START,
    PLAYER42,
    PLAYER42_START,
    PLAYER42_END,
    WS
};
enum GSName
{
    IGS,
    NNGS,
    LGS,
    WING,
    CTN,
    CWS,
    DEFAULT,
    GS_UNKNOWN
};

#endif
