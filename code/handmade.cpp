#include "handmade.h"


internal void GameOutputSound(game_sound_output_buffer* SoundBuffer, int ToneHz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int16* SampleOut = SoundBuffer->Samples;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;
    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume) ;
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
        tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
    
}

internal void RenderWeirdGradient(game_offscreen_buffer* Buffer, int XOffset, int YOffset)
{
    //lets check what the optimizer does kids
    
    uint8* Row = (uint8*)Buffer->Memory;
    
    for(int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32* Pixel = (uint32*)Row;
        for(int X = 0; X < Buffer->Width; ++X)
        {
            //remeber, this is little endian, so the pixel order is reversed!
            //BB GG RR
            uint8 Blue = (X + XOffset);
            uint8 Green = (Y + YOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }
        Row += Buffer->Pitch;
    }
    
}


internal void GameUpdateAndRender(game_offscreen_buffer* Buffer, 
                                  int BlueOffset, int GreenOffset, 
                                  game_sound_output_buffer* SoundBuffer,
                                  int ToneHz) 
{
    //TODO(brandon): allow sample offsets here for more robust platform optins
    GameOutputSound(SoundBuffer, ToneHz);
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}