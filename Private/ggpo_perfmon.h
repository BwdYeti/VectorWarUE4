#ifndef _GGPOUTIL_PERFMON_H
#define _GGPOUTIL_PERFMON_H

// UE4: allow Windows platform types to avoid naming collisions
//  this must be undone at the bottom of this file
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"

#include "include/ggponet.h"
#include <windows.h>
#include <timeapi.h>

void ggpoutil_perfmon_init(HWND hwnd);
void ggpoutil_perfmon_update(GGPOSession *ggpo, GGPOPlayerHandle players[], int num_players);
void ggpoutil_perfmon_toggle();

// UE4: disallow windows platform types
//  this was enabled at the top of the file
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

#endif 