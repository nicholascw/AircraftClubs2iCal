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
  char *accname = NULL, *accpass = NULL, *output = NULL, *tzurl = NULL,
       *tzid = NULL;
  while ((c = getopt(argc, argv, "u:p:o:t:z:")) != -1) {
    switch (c) {
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
      case 'z':
        tzurl = optarg;
        break;
      case ':':
        fprintf(stderr, "Option -%c requires an operand\n", optopt);
        errflg++;
        break;
      case '?':
        fprintf(stderr, "Unrecognized option: '-%c'\n", optopt);
        errflg++;
    }
  }
  if (errflg || (!accname || !accpass || !output || !tzid || !tzurl)) {
    fprintf(stderr,
            "usage: %s -u <accname from cookie> -p <accpass from cookie> "
            "-o <output ical filename> -t <TZID> -z <TZURL for VTIMEZONE>\n",
            argv[0]);
    exit(2);
  }
  char *cookie;
  asprintf(&cookie, "accname=%s; accpass=%s;", accname, accpass);
  char *table = fetch_aircraftclubs(cookie);
  free(cookie);
  icalcomponent *cal = gen_ical(tzid, tzurl);
  parse_table(table, cal, tzid);
  save_ical(cal, output);
}
