#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <x86intrin.h>

Profiler      profiler;
u32           globalProfilerParentIndex = 0;
ProfileAnchor globalProfileAnchors[4096];

void          initProfileBlock(ProfileBlock* block, char const* label_, u32 index_, u64 byteCount)
{
  block->parentIndex         = globalProfilerParentIndex;

  block->index               = index_;
  block->label               = label_;

  ProfileAnchor* profile     = globalProfileAnchors + block->index;
  block->oldElapsedInclusive = profile->elapsedInclusive;
  profile->processedByteCount += byteCount;

  globalProfilerParentIndex = block->index;
  block->startTime          = ReadCPUTimer();
}
void exitProfileBlock(ProfileBlock* block)
{
  u64 elapsed               = ReadCPUTimer() - block->startTime;
  globalProfilerParentIndex = block->parentIndex;

  ProfileAnchor* parent     = globalProfileAnchors + block->parentIndex;
  ProfileAnchor* profile    = globalProfileAnchors + block->index;

  parent->elapsedExclusive -= elapsed;
  profile->elapsedExclusive += elapsed;
  profile->elapsedInclusive = block->oldElapsedInclusive + elapsed;
  ++profile->hitCount;

  profile->label = block->label;
}

static void PrintTimeElapsed(ProfileAnchor* Anchor, u64 timerFreq, u64 TotalTSCElapsed)
{

  f64 Percent = 100.0 * ((f64)Anchor->elapsedExclusive / (f64)TotalTSCElapsed);
  printf("  %s[%lu]: %lu (%.2f%%", Anchor->label, Anchor->hitCount, Anchor->elapsedExclusive, Percent);
  if (Anchor->elapsedInclusive != Anchor->elapsedExclusive)
  {
    f64 PercentWithChildren = 100.0 * ((f64)Anchor->elapsedInclusive / (f64)TotalTSCElapsed);
    printf(", %.2f%% w/children", PercentWithChildren);
  }
  if (Anchor->processedByteCount)
  {
    f64 mb             = 1024.0f * 1024.0f;
    f64 gb             = mb * 1024.0f;

    f64 seconds        = Anchor->elapsedInclusive / (f64)timerFreq;
    f64 bytesPerSecond = Anchor->processedByteCount / seconds;
    f64 mbProcessed    = Anchor->processedByteCount / mb;
    f64 gbProcessed    = bytesPerSecond / gb;

    printf(" %.3fmb at %.2fgb/s", mbProcessed, gbProcessed);
  }
  printf(")\n");
}
static u64 GetOSTimerFreq(void)
{
  return 1000000;
}

static u64 ReadOSTimer(void)
{
  struct timeval Value;
  gettimeofday(&Value, 0);

  u64 Result = GetOSTimerFreq() * (u64)Value.tv_sec + (u64)Value.tv_usec;
  return Result;
}

u64 ReadCPUTimer(void)
{

  return __rdtsc();
}

#define TIME_TO_WAIT 100

u64 EstimateCPUTimerFreq(void)
{
  u64 OSFreq     = GetOSTimerFreq();

  u64 CPUStart   = ReadCPUTimer();
  u64 OSStart    = ReadOSTimer();
  u64 OSElapsed  = 0;
  u64 OSEnd      = 0;
  u64 OSWaitTime = OSFreq * TIME_TO_WAIT / 1000;
  while (OSElapsed < OSWaitTime)
  {
    OSEnd     = ReadOSTimer();
    OSElapsed = OSEnd - OSStart;
  }

  u64 CPUEnd     = ReadCPUTimer();
  u64 CPUElapsed = CPUEnd - CPUStart;

  return OSFreq * CPUElapsed / OSElapsed;
}

void initProfiler()
{
  profiler.StartTSC = ReadCPUTimer();
}

void displayProfilingResult()
{
  u64 endTime      = ReadCPUTimer();
  u64 totalElapsed = endTime - profiler.StartTSC;
  u64 cpuFreq      = EstimateCPUTimerFreq();

  printf("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)totalElapsed / (f64)cpuFreq, cpuFreq);
  for (u32 i = 0; i < ArrayCount(globalProfileAnchors); i++)
  {
    ProfileAnchor* profile = globalProfileAnchors + i;

    if (profile->elapsedInclusive)
    {
      PrintTimeElapsed(profile, cpuFreq, totalElapsed);
    }
  }
}

void parseIntFromString(int* dest, char* source, u8* length)
{
  char number[32];
  memset(number, 0, 32);

  for (int i = 0; i < 32; i++)
  {
    number[i] = 0;
  }
  u8 pos = 0;
  while (isdigit(source[pos]))
  {
    pos++;
  }
  memcpy(number, source, pos);
  *dest   = atoi(number);
  *length = pos;
}

void parseFloatFromString(float* dest, char* source, u8* length)
{
  char number[32];
  u8   pos = 0;
  while (source[pos] != ' ')
  {
    pos++;
  }
  memcpy(number, source, pos);
  *dest   = atof(number);
  *length = pos;
}
