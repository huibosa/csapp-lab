/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "MAKIMA",
    /* First member's full name */
    "huibosa",
    /* First member's email address */
    "huibosa@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

#define NEXT_FIT

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define GET(p) (*(size_t*)(p))
#define PUT(p, val) ((*(size_t*)(p)) = (val))

#define PACK(size, alloc) ((size) | (alloc))

#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_SIZE(p) (GET(p) & ~0x7)

#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE((char*)(bp)-WSIZE))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE((char*)(bp)-DSIZE))

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

static char* heap_listp;

#ifdef NEXT_FIT
static char* rover;
#endif

static void* coalesce(void* bp);
static void* extend_heap(size_t size);
static void* find_fit(size_t asize);
static void place(void* bp, size_t asize);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) {
    return -1;
  }

  PUT(heap_listp, 0);                            // Padding
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // Epilogue
  heap_listp += (2 * WSIZE);                     // Point to first block now

#ifdef NEXT_FIT
  rover = heap_listp;
#endif

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
    return -1;
  }

  return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void* mm_malloc(size_t size) {
  void* bp;
  size_t asize;
  size_t extend_size;

  if (size == 0) {
    return NULL;
  }

  asize = ALIGN(size + 2 * WSIZE);

  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  extend_size = MAX(asize, CHUNKSIZE); // TODO
  if ((bp = extend_heap(extend_size / WSIZE)) == NULL) {
    return NULL;
  }
  place(bp, asize);

  return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void* ptr) {
  size_t size;

  if (ptr == NULL) {
    return;
  }

  size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));

  coalesce(ptr);

  return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void* mm_realloc(void* ptr, size_t size) {
  void* new_ptr;

  if (ptr == NULL) {
    return mm_malloc(size);
  }

  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }

  if (ALIGN(size) + DSIZE < GET_SIZE(HDRP(ptr))) {
    place(ptr, ALIGN(size));
    return ptr;
  }

  if ((new_ptr = mm_malloc(size)) == NULL) {
    return NULL;
  }
  memcpy(new_ptr, ptr, size);
  mm_free(ptr);

  return new_ptr;
}

static void* extend_heap(size_t words) {
  void* bp;
  size_t asize;

  asize = ALIGN(words * WSIZE);
  if ((bp = mem_sbrk(asize)) == (void*)-1) {
    return NULL;
  }

  PUT(HDRP(bp), PACK(asize, 0));
  PUT(FTRP(bp), PACK(asize, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // Set new epilogue

  return coalesce(bp);
}

static void* coalesce(void* bp) {
  int prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc) {
    return bp;
  }

  else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(FTRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

  else {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(FTRP(PREV_BLKP(bp)));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
  }

#ifdef NEXT_FIT
  if ((rover > (char*)bp) && (rover < NEXT_BLKP(bp))) {
    rover = bp;
  }
#endif

  return bp;
}

static void* find_fit(size_t asize) {
  char* bp;

#ifdef NEXT_FIT
  for (bp = rover; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
      rover = bp;
      return bp;
    }
  }

  for (bp = heap_listp; bp < rover; bp = NEXT_BLKP(bp)) {
    if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
      rover = bp;
      return bp;
    }
  }
#else
  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
      return bp;
    }
  }
#endif

  return NULL;
}

static void place(void* bp, size_t asize) {
  size_t block_size = GET_SIZE(HDRP(bp));

  // Remaining space not meet the alignment demand
  if (block_size - asize < 2 * WSIZE) {
    PUT(HDRP(bp), PACK(block_size, 1));
    PUT(FTRP(bp), PACK(block_size, 1));
  }
  // Split
  else {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    PUT(HDRP(NEXT_BLKP(bp)), PACK(block_size - asize, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(block_size - asize, 0));
  }
}
