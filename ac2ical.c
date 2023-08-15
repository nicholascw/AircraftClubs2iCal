#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fetch_aircraftclubs.h"
#include "gen_ical.h"

int main(int argc, char *argv[]) {
  int c;
  int errflg = 0;
  char *accname = NULL, *accpass = NULL, *output = NULL, *tzid = NULL;
  char *other_mid = NULL;
  while ((c = getopt(argc, argv, "lu:p:o:t:m:")) != -1) {
    switch (c) {
      case 'l':
        // list tzid
        print_tzids();
        exit(4);
        break;
      case 'u':
        accname = optarg;
        break;
      case 'p':
        accpass = optarg;
        break;
      case 'o':
        output = optarg;
        break;
      case 't':
        tzid = optarg;
        break;
      case 'm':
        other_mid = optarg;
        break;
      case ':':
        fprintf(stderr, "Option -%c requires an operand\n", optopt);
        errflg++;
        break;
      case '?':
        fprintf(stderr, "Unrecognized or deprecated option: '-%c'\n", optopt);
        errflg++;
    }
  }
  if (errflg || (!accname || !accpass || !output || !tzid)) {
    fprintf(stderr,
            "usage: %s -u <accname from cookie> -p <accpass from cookie> "
            "-o <output ical filename> -t <TZID> [-m <other member's id>]\n"
            "   or: %s -l to list all available TZIDs.\n",
            argv[0], argv[0]);
    exit(2);
  }
  icalcomponent *cal = gen_ical(tzid);
  if (!cal) exit(3);
  char *cookie;
  asprintf(&cookie, "accname=%s; accpass=%s;", accname, accpass);
  char *table = fetch_aircraftclubs(cookie, other_mid);
  free(cookie);
  parse_table(table, cal, tzid);
  save_ical(cal, output);
}
