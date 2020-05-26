#ifndef _RENDERER_H_
#define _RENDERER_H_

// Forward declarations
struct GameState;
struct NonGameState;

/*
 * renderer.h --
 *
 * Abstract class used to render the game state.
 *
 */

class Renderer {
public:
   virtual ~Renderer() { }

   virtual void Draw(GameState &gs, NonGameState &ngs) = 0;
   virtual void SetStatusText(const char *text) = 0;
};

#endif
