#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <bits/getopt_core.h>
#include "cachelab.h"

#define LINELEN 100
#define ADDRLEN 16
#define ADDRBITS (1 << 6)
#define LASTBIT(k, n) ((k) & ((1 << (n)) - 1))
#define SUBBIT(k, m, n) LASTBIT((k) >> (m), ((n) - (m)))

typedef struct {
  int help;
  int verbose;
  int s;
  int E;
  int b;
  char* infile;
} CmdOpts;

typedef struct {
  unsigned long tag;
  char* block;
  unsigned short valid : 1;
} Line;

typedef struct {
  Line* line;
} Set;

typedef struct {
  int C, S, E, B, m, t, s, b;
  int numHits;
  int numMisses;
  int numEvictions;
  Set* set;
} Cache;

void usage(void);
void parseFlag(int argc, char* argv[]);
void buildCache(Cache* cache);
void freeCache(Cache* cache);
void parseFile(char* infile, Cache* cache);
void printCache(Cache* cache);

CmdOpts opts;  // Commmand line options

int main(int argc, char* argv[]) {
  Cache cache;

  parseFlag(argc, argv);
  buildCache(&cache);
  parseFile(opts.infile, &cache);
  printCache(&cache);
  freeCache(&cache);

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
void buildCache(Cache* cache) {
  cache->numHits = 0;
  cache->numMisses = 0;
  cache->numEvictions = 0;

  cache->s = opts.s;
  cache->E = opts.E;
  cache->b = opts.b;

  cache->B = pow(2, cache->b);
  cache->S = pow(2, cache->s);
  cache->C = cache->B * cache->E * cache->S;  // C = B * E * S

  cache->m = ADDRBITS;                        // 64 bit address
  cache->t = cache->m - cache->s - cache->b;  // t = m - s - b

  cache->set = (Set*)malloc(cache->S * sizeof(Set));
  for (Set* p = cache->set; p < cache->set + cache->S; p++) {
    p->line = (Line*)malloc(cache->E * sizeof(Line));
  }

  // Init lines
  for (int i = 0; i < cache->S; i++) {
    Set* pset = cache->set + i;
    for (int j = 0; j < cache->E; j++) {
      Line* pline = pset->line + j;
      pline->tag = 0;
      pline->valid = 0;
      pline->block = (char*)malloc(cache->B * sizeof(char));
    }
  }
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
void parseFile(char* infile, Cache* cache) {
  FILE* fp;
  char buf[LINELEN];

  if ((fp = (fopen(infile, "r"))) == NULL) {
    fprintf(stderr, "Can't open file \"%s\"", infile);
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  // Parse line
  while (fgets(buf, LINELEN, fp) != NULL) {
    // Ignore all instruction cache accesses
    if (buf[0] == 'I') {
      continue;
    }

    // Address starts at 4th pos
    unsigned long addr = strtoul(buf + 3, NULL, 16);
    unsigned long setid = SUBBIT(addr, cache->b + 1, cache->s);
    unsigned long tagid = SUBBIT(addr, cache->s + 1, cache->m);

    int hitFlag = 0;
    Line* victimLine = NULL;

    Set* pset = cache->set + setid;       // Set Selection
    for (int i = 0; i < cache->E; i++) {  // Line Matching
      Line* pline = pset->line + i;
      if (pline->valid == 1 && pline->tag == tagid) {
        hitFlag = 1;  // Cache hit
        break;
      } else if (pline->valid == 0) {
        victimLine = pline;
      }
    }

    if (hitFlag) {
      cache->numHits++;
    } else if (victimLine != NULL) {  // Empty line exists
      victimLine->tag = tagid;
      victimLine->valid = 1;
      cache->numMisses++;
    } else {  // Randomly choose victimLine
      victimLine = pset->line + rand() % cache->E;
      victimLine->tag = tagid;
      cache->numMisses++;
    }
  }

  if (fclose(fp) != 0) {
    fprintf(stderr, "Error in closing \"%s\"\n", infile);
  }
}

void freeCache(Cache* cache) {
  for (int i = 0; i < cache->S; i++) {
    free(cache->set[i].line->block);
    free(cache->set[i].line);
  }
  free(cache->set);
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
void parseFlag(int argc, char* argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
      case 'h':
        opts.help = 1;
        usage();
        exit(EXIT_SUCCESS);
      case 'v':
        opts.verbose = 1;
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
        opts.infile = optarg;
        break;
      default:
        usage();
        exit(EXIT_FAILURE);
    }
  }
  // Handle case with no arguments or wrong arguments
  if (optind < argc || optind == 1) {
    fprintf(stderr, "%s: Missing required command line argument\n",
            argv[0]);
    usage();
    exit(EXIT_FAILURE);
  }
}

void printCache(Cache* cache) {
  printf("(S, E, B, m) = (%d, %d, %d, %d)\n", cache->S, cache->E, cache->B,
         cache->m);
  printf("Cache hits = %d\n", cache->numHits);
  printf("Cache misses = %d\n", cache->numMisses);
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
