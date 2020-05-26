#ifndef _GDI_RENDERER_H_
#define _GDI_RENDERER_H_

// UE4: allow Windows platform types to avoid naming collisions
//  this must be undone at the bottom of this file
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"

#include <windef.h>
#include "nongamestate.h"
#include "renderer.h"

// Forward declarations
struct Ship;

/*
 * renderer.h --
 *
 * A simple C++ renderer that uses GDI to render the game state.
 *
 */

class GDIRenderer : public Renderer {
public:
   GDIRenderer(HWND hwnd);
   ~GDIRenderer();

   virtual void Draw(GameState &gs, NonGameState &ngs);
   virtual void SetStatusText(const char *text);

protected:
   void RenderChecksum(HDC hdc, int y, NonGameState::ChecksumInfo &info);
   void DrawShip(HDC hdc, int which, GameState &gamestate);
   void DrawConnectState(HDC hdc, Ship &ship, PlayerConnectionInfo &info);
   void CreateGDIFont(HDC hdc);

   HFONT          _font;
   HWND           _hwnd;
   RECT           _rc;
   HGLRC          _hrc;
   char           _status[1024];
   COLORREF       _shipColors[4];
   HPEN           _shipPens[4];
   HBRUSH         _bulletBrush;
   HBRUSH         _redBrush;
};

// UE4: disallow windows platform types
//  this was enabled at the top of the file
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

#endif
