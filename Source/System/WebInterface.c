/****************************/
/*   WEB INTERFACE          */
/*   Emscripten/WASM only   */
/****************************/
//
// Exposes C functions to JavaScript for the level editor
// and developer cheat interface.
//
// JavaScript usage:
//   Module.ccall('SetFenceCollisionEnabled', null, ['number'], [0]); // disable
//   Module.ccall('SetFenceCollisionEnabled', null, ['number'], [1]); // enable
//   Module.ccall('GetFenceCollisionEnabled', 'number', [], []);       // query
//   Module.ccall('SetStartLevel', null, ['number'], [3]);             // queue level jump
//
// Level file override (replace a Data/ file before the level loads):
//   Module.FS.writeFile('Data/Terrain/Level1_Garden.ter', byteArray);
//

#ifdef __EMSCRIPTEN__

#include "game.h"
#include <emscripten.h>

extern Boolean gDisableFenceCollision;
extern int gStartLevel;


/************** SET FENCE COLLISION ENABLED **************/
//
// enabled = 0 to disable fence collision, 1 to enable
//
EMSCRIPTEN_KEEPALIVE void SetFenceCollisionEnabled(int enabled)
{
	gDisableFenceCollision = !enabled;
	SDL_Log("Fence collision %s via web interface", enabled ? "enabled" : "disabled");
}


/************** GET FENCE COLLISION ENABLED **************/
//
// Returns 1 if fence collision is active, 0 if disabled
//
EMSCRIPTEN_KEEPALIVE int GetFenceCollisionEnabled(void)
{
	return gDisableFenceCollision ? 0 : 1;
}


/************** SET START LEVEL **************/
//
// Queues a direct level start (skipping menus).
// Takes effect on the next call to GameMain() – call before the game loop starts.
// level: 0-9 (see LEVEL_NUM_* constants in main.h)
//
EMSCRIPTEN_KEEPALIVE void SetStartLevel(int level)
{
	if (level >= 0 && level < NUM_LEVELS)
	{
		gStartLevel = level;
		SDL_Log("Start level set to %d via web interface", level);
	}
}

#endif	// __EMSCRIPTEN__
