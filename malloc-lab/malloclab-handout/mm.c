#include "mm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memlib.h"

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

#define ALIGNMENT 8

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define ALLOC_OVERHEAD (2 * WSIZE) // header + footer
#define FREE_OVERHEAD (3 * WSIZE)  // header + prev + next

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define GET(p) (*(size_t*)(p))
#define PUT(p, val) ((*(size_t*)(p)) = (val))

#define PACK(size, alloc) ((size) | (alloc))

#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_SIZE(p) (GET(p) & ~0x7)

#define HDRP(bp) ((char*)(bp)-WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define PREP(bp) ((char*)(bp))
#define SUCP(bp) ((char*)(bp) + WSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE((char*)(bp)-WSIZE))
#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE((char*)(bp)-DSIZE))

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

static char* heap_listp;
static char* free_headp;

static void* extend_heap(size_t words);
// static void* coalesce(void* bp);
static void* find_fit(size_t size);
static void place(void* bp, size_t asize);

static void delete_from_freelist(void* bp);
static void add_to_freelist(void* bp);

static void print_block(void* bp);
static void check_heap(int verbose);
static void check_block(void* bp);

int mm_init(void) {
  if ((heap_listp = (char*)mem_sbrk(4 * WSIZE)) == (void*)-1) {
    return 0;
  }

  PUT(heap_listp, 0);
  PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

  heap_listp += (2 * WSIZE);
  free_headp = NULL;

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
    return 0;
  }

  return 1;
}

void* mm_malloc(size_t size) {
  void* bp;
  size_t asize;
  size_t extend_size;

  if (size == 0) {
    return NULL;
  }

  // TODO
  asize = ALIGN(size + ALLOC_OVERHEAD);

  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  extend_size = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extend_size / WSIZE)) == NULL) {
    return NULL;
  }

  place(bp, asize);
  return bp;
}

void mm_free(void* ptr) {
  size_t size;

  if (ptr == NULL) {
    return;
  }

  size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0));
  // TODO: PREV

  add_to_freelist(ptr);
}

void* mm_realloc(void* ptr, size_t size) {
  void* new_ptr;

  if (ptr == NULL) {
    return mm_malloc(size);
  }

  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }

  if ((new_ptr = mm_malloc(size)) == NULL) {
    return NULL;
  }
  memcpy(new_ptr, ptr, size);
  mm_free(ptr);

  return new_ptr;
}

// The words must be at least CHUNKSIZE
static void* extend_heap(size_t words) {
  void* bp;
  size_t asize;

  asize = ALIGN(words * WSIZE);
  if ((bp = mem_sbrk(asize)) == (void*)-1) {
    return NULL;
  }

  PUT(HDRP(bp), PACK(asize, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // set new epilogue

  add_to_freelist(bp);

  return bp;
}

static void* find_fit(size_t size) {
  for (void* bp = free_headp; bp != NULL; bp = (void*)GET(SUCP(bp))) {
    if (GET_SIZE(HDRP(bp)) >= size) {
      return bp;
    }
  }
  return NULL;
}

static void place(void* bp, size_t asize) {
  size_t block_size = GET_SIZE(HDRP(bp));

  if (block_size - asize < FREE_OVERHEAD) {
    delete_from_freelist(bp);

    PUT(HDRP(bp), PACK(block_size, 1));
    PUT(FTRP(bp), PACK(block_size, 1));
  } else {
    delete_from_freelist(bp);

    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(block_size - asize, 0));

    add_to_freelist(bp);
  }
}

static void delete_from_freelist(void* bp) {
  size_t prep;
  size_t sucp;

  // bp is the head node and  the only node
  if ((GET(PREP(bp)) == (size_t)NULL) && (GET(SUCP(bp)) == (size_t)NULL)) {
    free_headp = NULL;
  }

  // bp is the head node, but not the only node
  else if (GET(PREP(bp)) == (size_t)NULL) {
    sucp = GET(SUCP(bp));
    PUT(PREP(sucp), (size_t)NULL);
    free_headp = (char*)sucp;
  }

  // bp is the last node
  else if (GET(SUCP(bp)) == (size_t)NULL) {
    prep = GET(PREP(bp));
    PUT(SUCP(prep), (size_t)NULL);
  }

  else {
    prep = GET(PREP(bp));
    sucp = GET(SUCP(bp));

    PUT(SUCP(prep), sucp);
    PUT(PREP(sucp), prep);
  }
}

static void add_to_freelist(void* bp) {
  if (free_headp == NULL) {
    free_headp = bp;
    PUT(PREP(bp), (size_t)NULL);
    PUT(SUCP(bp), (size_t)NULL);
  } else {
    PUT(PREP(free_headp), (size_t)bp);
    PUT(SUCP(bp), (size_t)free_headp);
    PUT(PREP(bp), (size_t)NULL);
    free_headp = bp;
  }
}

void check_freelist(void) {
  size_t hsize, halloc, prep, sucp;

  for (void* bp = free_headp; bp != NULL; bp = (void*)GET(SUCP(bp))) {
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    prep = GET(PREP(bp));
    sucp = GET(SUCP(bp));

    printf("%p: header: [%d:%c] prep: %p sucp: %p\n", bp,
           hsize, (halloc ? 'a' : 'f'), (void*)prep, (void*)sucp);
  }
}

static void print_block(void* bp) {
  size_t hsize, halloc, fsize, falloc;

  check_heap(0);
  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));

  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }

  printf("%p: header: [%d:%c]\n", bp, hsize, (halloc ? 'a' : 'f'));
  if (halloc) {
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));
    printf("            footer: [%d:%c]\n", fsize, (falloc ? 'a' : 'f'));
  }
}

static void check_block(void* bp) {
  if ((size_t)bp % 8) {
    printf("Error: %p is not doubleword aligned\n", bp);
  }
  if (GET_ALLOC(HDRP(bp)) && GET(HDRP(bp)) != GET(FTRP(bp))) {
    printf("Error: header does not match footer\n");
  }
}

/*
 * checkheap - Minimal check of the heap for consistency
 */
static void check_heap(int verbose) {
  char* bp = heap_listp;

  if (verbose) {
    printf("Heap (%p):\n", heap_listp);
  }

  if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))) {
    printf("Bad prologue header\n");
  }
  check_block(heap_listp);

  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (verbose) {
      print_block(bp);
    }
    check_block(bp);
  }

  if (verbose) {
    print_block(bp);
  }
  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
    printf("Bad epilogue header\n");
  }
}
