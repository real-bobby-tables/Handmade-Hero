#if !defined(HANDMADE_H)
/*
TODO(brandon): Services that the platform layer provides to the game
*/

/*
NOTE(brandon): Services that the game provides to the platform layer
this may expand in the future
*/

//this needs four things --
//controller/keyboard input
//bitmap buffer
//sound buffer
//timing info
struct game_offscreen_buffer
{
    void* Memory;
    int Width;
    int Height;
    int Pitch;
};

internal void GameUpdateAndRender(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif