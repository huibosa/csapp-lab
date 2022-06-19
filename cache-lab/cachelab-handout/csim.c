#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
  unsigned long valid;
  unsigned long tag;
  unsigned long block;
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
      pline->block = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//
// Parse trace data
//
// Form: [space]operation address,size
//
// "I" denotes an instruction load;
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
    unsigned long s = SUBBIT(addr, cache->b + 1, cache->s);
    unsigned long t = SUBBIT(addr, cache->s + 1, cache->m);

    int hitFlag = 0;
    Line* victimLine = NULL;

    Set* pset = cache->set + s;           // Set Selection
    for (int i = 0; i < cache->E; i++) {  // Line Matching
      Line* pline = pset->line + i;
      if (pline->valid == 1 && pline->tag == t) {
        hitFlag = 1;  // Cache hit
        break;
      } else if (pline->valid == 0) {
        victimLine = pline;
      }
    }

    if (hitFlag) {
      cache->numHits++;
    } else if (victimLine != NULL) {  // Empty line exists
      victimLine->tag = t;
      victimLine->valid = 1;
      cache->numMisses++;
    } else {  // Randomly choose victimLine
      victimLine = pset->line + rand() % cache->E;
      victimLine->tag = t;
      cache->numMisses++;
    }
  }

  if (fclose(fp) != 0) {
    fprintf(stderr, "Error in closing \"%s\"\n", infile);
  }
}

void freeCache(Cache* cache) {
  for (int i = 0; i < cache->S; i++) {
    free(cache->set[i].line);
  }
  free(cache->set);
}

////////////////////////////////////////////////////////////////
////
//// acs.opt: iload, dload, store, modify
//// acs.addr: target address
//// acs.size: allocated size
////
////////////////////////////////////////////////////////////////
// void parseLine(char* line, MemAccess* acs) {
//   // NOTE: trailing spaces
//   char* const tokenSpace = strrchr(line, ' ');
//   char* const tokenComma = strrchr(line, ',');

//  char* const addrBegin = tokenSpace + 1;
//  char* const addrEnd = tokenComma;
//  const int addrSize = addrEnd - addrBegin;

//  char* const sizeBegin = tokenComma + 1;

//  /* Parse option */
//  if (line[0] == 'I') {
//    acs->opt = iload;
//  } else if (line[0] == ' ') {
//    switch (line[1]) {
//      case 'L':
//        acs->opt = dload;
//        break;
//      case 'S':
//        acs->opt = store;
//        break;
//      case 'M':
//        acs->opt = modify;
//        break;
//      default:
//        fprintf(stderr, "Error parsing file: No \"%c\" option\n", line[1]);
//    }
//  }

//  /* Parse target address */
//  if (addrSize > ADDRLEN) {
//    // Discard redundant byte
//    char* p = addrBegin;
//    p += addrSize - ADDRLEN;
//    strncpy(acs->addr, p, addrSize);
//  } else if (addrSize < ADDRLEN) {
//    // Pad empty space with '0'
//    int p;
//    for (p = 0; p < ADDRLEN - addrSize; p++) {
//      acs->addr[p] = '0';
//    }
//    strncpy(acs->addr + p, addrBegin, addrSize);
//  } else {
//    strncpy(acs->addr, addrBegin, addrSize);
//  }

//  acs->addr[ADDRLEN] = '\0';  // Pad address with '\0' to create string

//  /* Parse allocated size */
//  char* p;
//  for (p = addrBegin; *p != '\n'; p++) {
//    continue;
//  }
//  *p = '\0';
//  acs->size = atoi(sizeBegin);
//}

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
  for (int i = 1; i < argc && argv[i][0] == '-'; i++) {
    // TODO:
    switch (argv[i][1]) {
      case 'h':
        opts.help = 1;
        usage();
        exit(0);
        break;
      case 'v':
        opts.verbose = 1;
        break;
      case 's':
        i++;
        opts.s = atoi(argv[i]);
        break;
      case 'E':
        i++;
        opts.E = atoi(argv[i]);
        break;
      case 'b':
        i++;
        opts.b = atoi(argv[i]);
        break;
      case 't':
        i++;
        opts.infile = argv[i];
        break;
      default:
        puts("Undefined flag.");
    }
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
