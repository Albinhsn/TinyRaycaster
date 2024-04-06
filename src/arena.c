#include "arena.h"
#include <stdio.h>
#include <stdlib.h>

u64 ArenaPush(Arena* arena, u64 size)
{
  if (arena->ptr + size > arena->maxSize)
  {
    printf("SEVERE: Assigned %ld out of %ld\n", arena->ptr, arena->maxSize);
    return 0;
  }
  u64 out = arena->memory + arena->ptr;
  arena->ptr += size;
  printf("INFO: Assigned %ld out of %ld\n", arena->ptr, arena->maxSize);
  return out;
}
void ArenaPop(Arena* arena, u64 size)
{
  arena->ptr -= size;
}
