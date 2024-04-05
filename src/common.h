#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;

#define PI 3.14159265358979

struct Profiler
{
  u64 StartTSC;
  u64 EndTSC;
};
typedef struct Profiler Profiler;

extern Profiler         profiler;
u64                     ReadCPUTimer(void);
u64                     EstimateCPUTimerFreq(void);

void                    initProfiler();
void                    displayProfilingResult();

#define PROFILER 1
#if PROFILER

struct ProfileAnchor
{
  u64         elapsedExclusive;
  u64         elapsedInclusive;
  u64         hitCount;
  u64         processedByteCount;
  char const* label;
};
typedef struct ProfileAnchor ProfileAnchor;

extern ProfileAnchor         globalProfileAnchors[4096];
extern u32                   globalProfilerParentIndex;

struct ProfileBlock
{
  char const* label;
  u64         oldElapsedInclusive;
  u64         startTime;
  u32         parentIndex;
  u32         index;
};
typedef struct ProfileBlock ProfileBlock;
void                        initProfileBlock(ProfileBlock* block, char const* label_, u32 index_, u64 byteCount);
void                        exitProfileBlock(ProfileBlock* block);

#define NameConcat2(A, B) A##B
#define NameConcat(A, B)  NameConcat2(A, B)
#define TimeBandwidth(Name, ByteCount)                                                                                                                                                                 \
  ProfileBlock Name;                                                                                                                                                                                   \
  initProfileBlock(&Name, "Name", __COUNTER__ + 1, ByteCount);
#define ExitBlock(Name)              exitProfileBlock(&Name)
#define TimeBlock(Name)              TimeBandwidth(Name, 0)
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(GlobalProfilerAnchors), "Number of profile points exceeds size of profiler::Anchors array")
#define TimeFunction                 TimeBlock(__func__)

#else

#define TimeBlock(blockName)
#define TimeFunction
#endif

void parseFloatFromString(float* dest, char* source, u8* length);
void parseIntFromString(int* dest, char* source, u8* length);

#endif
