#ifndef _VECTORWAR_H
#define _VECTORWAR_H

#include "../Private/gamestate.h"
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

void VectorWar_InitSpectator(HWND hwnd, unsigned short localport, int num_players, char *host_ip, unsigned short host_port);
void VectorWar_DrawCurrentFrame();
void VectorWar_AdvanceFrame(int inputs[], int disconnect_flags);
void VectorWar_DisconnectPlayer(int player);
BOOL VectorWar_DestroyWindow(HWND hwnd);

#define ARRAY_SIZE(n)      (sizeof(n) / sizeof(n[0]))
#define FRAME_DELAY        2

GameState gs = { 0 };

class VECTORWAR_API VectorWarHost
{
public:
    static HWND CreateMainWindow(HINSTANCE hInstance);
    static BOOL DestroyWindow(HWND hwnd);

    static void VectorWar_Init(HWND hwnd, unsigned short localport, int num_players, GGPOPlayer* players, int num_spectators);
    static void RunMainLoop(HWND hwnd);
    static void VectorWar_RunFrame(HWND hwnd);
    static void VectorWar_Idle(int time);
    static void VectorWar_Exit();
};

#endif
