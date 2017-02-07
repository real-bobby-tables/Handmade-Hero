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

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16* Samples;
};

internal void GameUpdateAndRender(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset, game_sound_output_buffer* SoundBuffer, int ToneHz);

#define HANDMADE_H
#endif