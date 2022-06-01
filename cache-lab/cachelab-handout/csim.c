#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cachelab.h"

#define LINELEN 100

typedef char byte;

typedef struct {
  int valid;
  int tag;
  byte* block;
} Line;

typedef struct {
  Line* line;
  int sz;
  int cap;
} Set;

typedef struct {
  int C, S, E, B, m, t, s, b;
  int setSize;
  Set* set;
} Cache;

typedef enum {
  iload,
  dload,
  store,
  modify,
} option;

typedef struct {
  option opt;
  char addr[9];
  int size;
} MemAccess;

void usage(void);
void printMessage(void);
char* parseFlag(int argc, char* argv[], Cache* cache);
void parseFile(char* infile, MemAccess** acss);
void parseLine(char* line, MemAccess* acs);

int verbose;

int main(int argc, char* argv[]) {
  Cache cache;
  MemAccess** acss = NULL;

  char* infile = parseFlag(argc, argv, &cache);
  parseFile(infile, acss);

  return 0;
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
void parseFile(char* infile, MemAccess** acss) {
  FILE* fp;
  char buf[LINELEN];
  MemAccess acs;

  if ((fp = (fopen(infile, "r"))) == NULL) {
    fprintf(stderr, "Can't open file \"%s\"", infile);
    exit(EXIT_FAILURE);
  }

  while (fgets(buf, LINELEN, fp) != NULL) {
    parseLine(buf, &acs);
    printf("%d, %s, %d\n", acs.opt, acs.addr, acs.size);
  }

  // free(buffer);
  if (fclose(fp) != 0) {
    fprintf(stderr, "Error in closing \"%s\"\n", infile);
  }
}

//////////////////////////////////////////////////////////////
//
// acs.opt: iload, dload, store, modify
// acs.addr: target address
// acs.size: allocated size
//
//////////////////////////////////////////////////////////////
void parseLine(char* line, MemAccess* acs) {
  // NOTE: trailing spaces
  char* const tokenSpace = strrchr(line, ' ');
  char* const tokenComma = strrchr(line, ',');

  char* const addrBegin = tokenSpace + 1;
  char* const addrEnd = tokenComma;
  const int addrSize = addrEnd - addrBegin;

  char* const sizeBegin = tokenComma + 1;

  /* Parse option */
  if (line[0] == 'I') {
    acs->opt = iload;
  } else if (line[0] == ' ') {
    switch (line[1]) {
      case 'L':
        acs->opt = dload;
        break;
      case 'S':
        acs->opt = store;
        break;
      case 'M':
        acs->opt = modify;
        break;
      default:
        fprintf(stderr, "Error parsing file: No \"%c\" option\n", line[1]);
    }
  }

  /* Parse target address */
  if (addrSize > 8) {
    // Discard redundant byte
    char* p = addrBegin;
    p += addrSize - 8;
    strncpy(acs->addr, p, addrSize);
  } else if (addrSize < 8) {
    // Pad empty space with '0'
    int p;
    for (p = 0; p < 8 - addrSize; p++) {
      acs->addr[p] = '0';
    }
    strncpy(acs->addr + p, addrBegin, addrSize);
  } else {
    strncpy(acs->addr, addrBegin, addrSize);
  }

  acs->addr[8] = '\0';  // Pad address with '\0' to create string

  /* Parse allocated size */
  char* p;
  for (p = addrBegin; *p != '\n'; p++) {
    continue;
  }
  *p = '\0';
  acs->size = atoi(sizeBegin);
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
char* parseFlag(int argc, char* argv[], Cache* cache) {
  char* infile;

  for (int i = 1; i < argc && argv[i][0] == '-'; i++) {
    switch (argv[i][1]) {
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
      case 'v':
        verbose = 1;
      case 's':
        i++;
        cache->s = atoi(argv[i]);
        cache->S = pow(2, cache->s);  // S = 2 ^ s
        break;
      case 'E':
        i++;
        cache->E = atoi(argv[i]);
        break;
      case 'b':
        i++;
        cache->b = atoi(argv[i]);
        cache->B = pow(2, cache->b);  // B = 2 ^ b
        break;
      case 't':
        i++;
        infile = argv[i];
        break;
      default:
        puts("Undefined flag.");
        exit(EXIT_FAILURE);
    }
  }

  cache->C = cache->B * cache->E * cache->S;  // C = B * E * S
  cache->m = (1 << 6);                        // 64 bit address
  cache->t = cache->m - cache->s - cache->b;  // t = m - s - b

  return infile;
}

/* void printMessage(void) { */
/*   printf("%-20s %10d\n", "Cache size:", C); */
/*   printf("%-20s %10d\n", "Set number:", S); */
/*   printf("%-20s %10d\n", "Set line Number:", E); */
/*   printf("%-20s %10d\n", "Block size:", B); */
/*   printf("%-20s %10d\n", "Tag bit number:", t); */
/*   printf("%-20s %10s\n", "File name:", infile); */
/* } */

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
