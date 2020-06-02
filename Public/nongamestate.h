#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "include/ggponet.h"

#define MAX_PLAYERS     64

/*
 * nongamestate.h --
 *
 * These are other pieces of information not related to the state
 * of the game which are useful to carry around.  They are not
 * included in the GameState class because they specifically
 * should not be rolled back.
 */

enum PlayerConnectState {
   Connecting = 0,
   Synchronizing,
   Running,
   Disconnected,
   Disconnecting,
};

struct PlayerConnectionInfo {
   GGPOPlayerType       type;
   GGPOPlayerHandle     handle;
   PlayerConnectState   state;
   int                  connect_progress;
   int                  disconnect_timeout;
   int                  disconnect_start;
};

struct NonGameState {
   struct ChecksumInfo {
      int framenumber;
      int checksum;
   };

   void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state);

   void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout);

   void SetConnectState(PlayerConnectState state);

   void UpdateConnectProgress(GGPOPlayerHandle handle, int progress);

   GGPOPlayerHandle     local_player_handle;
   PlayerConnectionInfo players[MAX_PLAYERS];
   int                  num_players;

   ChecksumInfo         now;
   ChecksumInfo         periodic;
};

#endif
