#include "vectorwar.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>
#include "gamestate.h"

// UE4: allow Windows platform types to avoid naming collisions
//  this must be undone at the bottom of this file
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"

#include <timeapi.h>

//#define SYNC_TEST    // test: turn on synctest
#define MAX_PLAYERS     64

/* 
 * Simple checksum function stolen from wikipedia:
 *
 *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
 */

int
fletcher32_checksum(short *data, size_t len)
{
   int sum1 = 0xffff, sum2 = 0xffff;

   while (len) {
      size_t tlen = len > 360 ? 360 : len;
      len -= tlen;
      do {
         sum1 += *data++;
         sum2 += sum1;
      } while (--tlen);
      sum1 = (sum1 & 0xffff) + (sum1 >> 16);
      sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   }

   /* Second reduction step to reduce sums to 16 bits */
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   return sum2 << 16 | sum1;
}

uint32 get_time()
{
    return timeGetTime();
}

/*
 * VectorWar_DisconnectPlayer --
 *
 * Disconnects a player from this session.
 */

void
VectorWar_DisconnectPlayer(int player)
{
   if (player < ngs.num_players) {
      char logbuf[128];
      GGPOErrorCode result = GGPONet::ggpo_disconnect_player(ggpo, ngs.players[player].handle);
      if (GGPO_SUCCEEDED(result)) {
         sprintf_s(logbuf, ARRAYSIZE(logbuf), "Disconnected player %d.\n", player);
      } else {
         sprintf_s(logbuf, ARRAYSIZE(logbuf), "Error while disconnecting player (err:%d).\n", result);
      }
   }
}

/*
 * VectorWar_AdvanceFrame --
 *
 * Advances the game state by exactly 1 frame using the inputs specified
 * for player 1 and player 2.
 */
void VectorWar_AdvanceFrame(int inputs[], int disconnect_flags)
{
   gs.Update(inputs, disconnect_flags);

   // update the checksums to display in the top of the window.  this
   // helps to detect desyncs.
   ngs.now.framenumber = gs._framenumber;
   ngs.now.checksum = fletcher32_checksum((short *)&gs, sizeof(gs) / 2);
   if ((gs._framenumber % 90) == 0) {
      ngs.periodic = ngs.now;
   }

   // Notify ggpo that we've moved forward exactly 1 frame.
   GGPONet::ggpo_advance_frame(ggpo);

   // Update the performance monitor display.
   GGPOPlayerHandle handles[MAX_PLAYERS];
   int count = 0;
   for (int i = 0; i < ngs.num_players; i++) {
      if (ngs.players[i].type == EGGPOPlayerType::REMOTE) {
         handles[count++] = ngs.players[i].handle;
      }
   }
   //ggpoutil_perfmon_update(ggpo, handles, count);
}

/*
 * VectorWar_RunFrame --
 *
 * Run a single frame of the game.
 */
void
VectorWarHost::VectorWar_RunFrame(int local_input)
{
  GGPOErrorCode result = GGPO_OK;
  int disconnect_flags;
  int inputs[MAX_SHIPS] = { 0 };

  if (ngs.local_player_handle != GGPO_INVALID_HANDLE) {
#if defined(SYNC_TEST)
     local_input = rand(); // test: use random inputs to demonstrate sync testing
#endif
     result = GGPONet::ggpo_add_local_input(ggpo, ngs.local_player_handle, &local_input, sizeof(local_input));
  }

   // synchronize these inputs with ggpo.  If we have enough input to proceed
   // ggpo will modify the input list with the correct inputs to use and
   // return 1.
  if (GGPO_SUCCEEDED(result)) {
     result = GGPONet::ggpo_synchronize_input(ggpo, (void *)inputs, sizeof(int) * MAX_SHIPS, &disconnect_flags);
     if (GGPO_SUCCEEDED(result)) {
         // inputs[0] and inputs[1] contain the inputs for p1 and p2.  Advance
         // the game by 1 frame using those inputs.
         VectorWar_AdvanceFrame(inputs, disconnect_flags);
     }
  }
}

/*
 * VectorWar_Idle --
 *
 * Spend our idle time in ggpo so it can use whatever time we have left over
 * for its internal bookkeeping.
 */
void
VectorWarHost::VectorWar_Idle(int time)
{
    GGPONet::ggpo_idle(ggpo, time);
}

void
VectorWarHost::VectorWar_Exit()
{
   memset(&gs, 0, sizeof(gs));
   memset(&ngs, 0, sizeof(ngs));

   if (ggpo) {
       GGPONet::ggpo_close_session(ggpo);
      ggpo = NULL;
   }
}

TArray<FGGPONetworkStats>
VectorWarHost::VectorWar_GetNetworkStats()
{
    GGPOPlayerHandle RemoteHandles[MAX_PLAYERS];
    int Count = 0;
    for (int i = 0; i < ngs.num_players; i++) {
        if (ngs.players[i].type == EGGPOPlayerType::REMOTE) {
            RemoteHandles[Count++] = ngs.players[i].handle;
        }
    }

    TArray<FGGPONetworkStats> Result;
    for (int i = 0; i < Count; i++)
    {
        FGGPONetworkStats Stats = { 0 };
        GGPONet::ggpo_get_network_stats(ggpo, RemoteHandles[i], &Stats);
        Result.Add(Stats);
    }

    return Result;
}

// UE4: disallow windows platform types
//  this was enabled at the top of the file
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

