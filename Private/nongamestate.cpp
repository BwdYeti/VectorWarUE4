#include "nongamestate.h"

void NonGameState::SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].handle == handle) {
            players[i].connect_progress = 0;
            players[i].state = state;
            break;
        }
    }
}

void NonGameState::SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].handle == handle) {
            players[i].disconnect_start = when;
            players[i].disconnect_timeout = timeout;
            players[i].state = Disconnecting;
            break;
        }
    }
}

void NonGameState::SetConnectState(PlayerConnectState state) {
    for (int i = 0; i < num_players; i++) {
        players[i].state = state;
    }
}

void NonGameState::UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
    for (int i = 0; i < num_players; i++) {
        if (players[i].handle == handle) {
            players[i].connect_progress = progress;
            break;
        }
    }
}