#ifndef _VECTORWAR_H
#define _VECTORWAR_H

#include "gamestate.h"
#include "nongamestate.h"
#include "include/ggponet.h"

/*
 * vectorwar.h --
 *
 * Interface to the vector war application.
 *
 */

enum VectorWarInputs {
   INPUT_THRUST            = (1 << 0),
   INPUT_BREAK             = (1 << 1),
   INPUT_ROTATE_LEFT       = (1 << 2),
   INPUT_ROTATE_RIGHT      = (1 << 3),
   INPUT_FIRE              = (1 << 4),
   INPUT_BOMB              = (1 << 5),
};

void VectorWar_AdvanceFrame(int inputs[], int disconnect_flags);
void VectorWar_DisconnectPlayer(int player);

#define ARRAY_SIZE(n)      (sizeof(n) / sizeof(n[0]))
#define FRAME_DELAY        2

GGPOSession* ggpo = nullptr;

GameState gs = { 0 };
NonGameState ngs = { 0 };

int fletcher32_checksum(short* data, size_t len);
uint32 get_time();

class VECTORWARUE4_API VectorWarHost
{
public:
    static void VectorWar_RunFrame(int local_input);
    static void VectorWar_Idle(int time);
    static void VectorWar_Exit();

    static TArray<FGGPONetworkStats> VectorWar_GetNetworkStats();
};

#endif
