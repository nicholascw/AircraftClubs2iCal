#include <libical/ical.h>
icalcomponent *gen_ical(char *tzid);
icalcomponent *gen_event(char *id, char *date, char *aircraft, char *instructor,
                         char *equipment, char *tach, char *hobbs, char *tzid);
void save_ical(icalcomponent *cal, char *path);
