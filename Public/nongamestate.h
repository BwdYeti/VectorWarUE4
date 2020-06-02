#pragma once

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

UENUM(BlueprintType)
enum class EPlayerConnectState : uint8 {
   Connecting = 0   UMETA(DisplayName = "Connecting"),
   Synchronizing    UMETA(DisplayName = "Synchronizing"),
   Running          UMETA(DisplayName = "Running"),
   Disconnected     UMETA(DisplayName = "Disconnected"),
   Disconnecting    UMETA(DisplayName = "Disconnecting"),
};

struct PlayerConnectionInfo {
   EGGPOPlayerType      type;
   GGPOPlayerHandle     handle;
   EPlayerConnectState  state;
   int                  connect_progress;
   int                  disconnect_timeout;
   int                  disconnect_start;
};

struct NonGameState {
   struct ChecksumInfo {
      int framenumber;
      int checksum;
   };

   void SetConnectState(GGPOPlayerHandle handle, EPlayerConnectState state);

   void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout);

   void SetConnectState(EPlayerConnectState state);

   void UpdateConnectProgress(GGPOPlayerHandle handle, int progress);

   GGPOPlayerHandle     local_player_handle;
   PlayerConnectionInfo players[MAX_PLAYERS];
   int                  num_players;

   ChecksumInfo         now;
   ChecksumInfo         periodic;
};
