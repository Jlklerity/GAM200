#include "Platform/Time.hpp"
#include "Application/Platform.hpp"

double Time::GetTime()
{
    return static_cast<double>(SDL_GetPerformanceCounter()) / static_cast<double>(SDL_GetPerformanceFrequency());
}