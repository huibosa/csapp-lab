#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <bits/getopt_core.h>
#include "cachelab.h"

#define LINELEN 100
#define ADDRLEN 16
#define ADDRBITS (1 << 6)
// Move right (n - 1) bit and then get last (m - n) bit
#define SUBBIT(k, m, n) ((k) >> ((n)-1)) & ((1 << ((m) - (n) + 1)) - 1)

typedef struct {
  unsigned long s;
  unsigned long E;
  unsigned long b;
  char* traceFile;
  bool verbose;
} CmdOpts;

typedef struct {
  unsigned long tag;
  unsigned long lruCounter;
  bool valid;
  char block[];
} Line;

typedef struct {
  unsigned long E;
  Line* line[];
} Set;

typedef struct {
  unsigned long numHits;
  unsigned long numMisses;
  unsigned long numEvictions;
  unsigned long C, S, E, B, m, t, s, b;
  Set* set[];
} Cache;

void usage(void);
void parseCmdOpts(int argc, char* argv[]);
Cache* initCache(void);
Set* initSet(unsigned long E, unsigned long B);
Line* initLine(unsigned long B);
void freeCache(Cache* cache);
void readFile(char* infile, Cache* cache);
void load(Cache* cache, unsigned long addr);

CmdOpts opts;  // Commmand line options
unsigned long counter = 0;

int main(int argc, char* argv[]) {
  Cache* pc;

  parseCmdOpts(argc, argv);
  pc = initCache();
  readFile(opts.traceFile, pc);
  printSummary(pc->numHits, pc->numMisses, pc->numEvictions);
  freeCache(pc);

  return 0;
}

///////////////////////////////////////////////////////////////////////
//
// * cache.s <s>: Number of set index bits (S = 2s is the number of sets)
// * cache.E <E>: Associativity (number of lines per set)
// * cache.b <b>: Number of block bits (B = 2b is the block size)
// * cache.t <t>: Number of tab bits (t = m - s - b)
//
///////////////////////////////////////////////////////////////////////
Cache* initCache(void) {
  unsigned long S = 1 << opts.s;
  unsigned long B = 1 << opts.b;

  Cache* pc = (Cache*)malloc(sizeof(Cache) + sizeof(Set*) * S);

  pc->numHits = 0;
  pc->numMisses = 0;
  pc->numEvictions = 0;

  pc->s = opts.s;
  pc->E = opts.E;
  pc->b = opts.b;

  pc->B = B;
  pc->S = S;
  pc->C = pc->B * pc->E * pc->S;  // C = B * E * S

  pc->m = ADDRBITS;               // 64 bit address
  pc->t = pc->m - pc->s - pc->b;  // t = m - s - b

  for (int i = 0; i < pc->S; i++) {
    pc->set[i] = initSet(pc->E, pc->B);
  }

  return pc;
}

Set* initSet(unsigned long E, unsigned long B) {
  Set* ps;

  ps = (Set*)malloc(sizeof(Set) + sizeof(Line*) * E);
  ps->E = E;

  for (int i = 0; i < E; i++) {
    ps->line[i] = initLine(B);
  }

  return ps;
}

Line* initLine(unsigned long B) {
  Line* pl;

  pl = (Line*)malloc(sizeof(Line) + sizeof(char) * B);

  pl->valid = false;
  pl->tag = 0;
  pl->lruCounter = 0;

  return pl;
}

void freeCache(Cache* pc) {
  for (size_t i = 0; i < pc->S; i++) {
    for (size_t j = 0; j < pc->E; j++) {
      free(pc->set[i]->line[j]);
    }
    free(pc->set[i]);
  }
  free(pc);
}

////////////////////////////////////////////////////////////////////
//
// Parse trace data
//
// Form: [space]operation address,size
//
// "I" (Ignored) denotes an instruction load;
// "L" a data load;
// "S" a data store;
// "M" a data modify (i.e., a data load followed by a data store).
//
////////////////////////////////////////////////////////////////////
void readFile(char* infile, Cache* pc) {
  FILE* fp;
  char buf[LINELEN];

  if ((fp = (fopen(infile, "r"))) == NULL) {
    fprintf(stderr, "Can't open file \"%s\"", infile);
    exit(EXIT_FAILURE);
  }

  while (fgets(buf, LINELEN, fp) != NULL) {
    char* p = strchr(buf, '\n');
    *p = '\0';

    // Ignore all instruction cache accesses
    if (buf[0] == 'I') {
      continue;
    }

    counter++;

    if (opts.verbose) {
      printf("%s ", buf + 1);
    }

    // Get address
    unsigned long addr = strtoul(buf + 3, NULL, 16);

    switch (buf[1]) {
      case 'L':
      case 'S':
        load(pc, addr);
        break;
      case 'M':
        load(pc, addr);
        load(pc, addr);
        break;
      default:
        fprintf(stderr, "Invalid instruction: %c\n", buf[1]);
    }

    if (opts.verbose) {
      printf("\n");
    }
  }

  if (fclose(fp) != 0) {
    fprintf(stderr, "Error in closing \"%s\"\n", infile);
  }
}

void load(Cache* pc, unsigned long addr) {
  unsigned long setIdx = SUBBIT(addr, pc->s + pc->b, pc->b + 1);
  unsigned long tag = SUBBIT(addr, pc->m, pc->s + pc->b + 1);
  // unsigned long blkOffset = SUBBIT(addr, pc->b, 1);

  // Set Selection
  Set* ps = pc->set[setIdx];

  long hitLineIdx = -1;
  unsigned long victimLineIdx = 0;
  unsigned long evictLruCounter = ps->line[victimLineIdx]->lruCounter;

  // Line Matching
  for (int i = 0; i < pc->E; i++) {
    Line* pl = ps->line[i];
    // Cache hit, set line lru counter
    if (pl->valid && pl->tag == tag) {
      hitLineIdx = i;
      break;  // Don't need to find LRU line
    }
    // Find LRU line
    else if (pl->lruCounter < evictLruCounter) {
      evictLruCounter = pl->lruCounter;
      victimLineIdx = i;
    }
  }

  // Cache hit
  if (hitLineIdx >= 0) {
    pc->numHits++;
    ps->line[hitLineIdx]->lruCounter = counter;  // Update lruCounter
    if (opts.verbose) {
      printf("hit ");
    }
  }
  // Cache miss
  else {
    pc->numMisses++;
    if (opts.verbose) {
      printf("miss ");
    }

    // LRU line as victim
    if (ps->line[victimLineIdx]->valid) {
      pc->numEvictions++;
      if (opts.verbose) {
        printf("miss eviction ");
      }
    }

    ps->line[victimLineIdx]->valid = true;
    ps->line[victimLineIdx]->tag = tag;
    ps->line[victimLineIdx]->lruCounter = counter;  // Update lruCounter
  }
}
///////////////////////////////////////////////////////////////////////
//
// * -h: Optional help flag that prints usage info
// * -v: Optional verbose flag that displays trace info
// * -s <s>: Number of set index bits (S = 2s is the number of sets)
// * -E <E>: Associativity (number of lines per set)
// * -b <b>: Number of block bits (B = 2b is the block size)
// * -t <tracefile>: Name of the valgrind trace to replay
//
///////////////////////////////////////////////////////////////////////
void parseCmdOpts(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
      case 'v':
        opts.verbose = true;
        break;
      case 's':
        opts.s = atoi(optarg);
        break;
      case 'E':
        opts.E = atoi(optarg);
        break;
      case 'b':
        opts.b = atoi(optarg);
        break;
      case 't':
        opts.traceFile = optarg;
        break;
      default:
        fprintf(stderr, "%s: Missing required command line argument\n",
                argv[0]);
        usage();
        exit(EXIT_FAILURE);
    }
  }
  // Handle case with no arguments or wrong arguments
  if (optind < argc || optind == 1) {
    fprintf(stderr, "%s: Missing required command line argument\n", argv[0]);
    usage();
    exit(EXIT_FAILURE);
  }
}

void usage(void) {
  printf(
      "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
      "Options:\n"
      "  -h         Print this help message.\n"
      "  -v         Optional verbose flag.\n"
      "  -s <num>   Number of set index bits.\n"
      "  -E <num>   Number of lines per set.\n"
      "  -b <num>   Number of block offset bits.\n"
      "  -t <file>  Trace file.\n\n"
      "Examples:\n"
      "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
      "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}
