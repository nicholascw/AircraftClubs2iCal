#include <libical/ical.h>
icalcomponent *gen_ical();
icalcomponent *gen_event(char *id, char *date, char *aircraft, char *instructor,
                         char *equipment, char *tz);
void save_ical(icalcomponent *cal, char *path);
