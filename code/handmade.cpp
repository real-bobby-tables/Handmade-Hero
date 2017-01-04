#include "handmade.h"


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


internal void GameUpdateAndRender(game_offscreen_buffer* Buffer, int BlueOffset, int GreenOffset) 
{
    RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}