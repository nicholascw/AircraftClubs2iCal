#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libical/ical.h>
#include <stdio.h>
#include <string.h>

icalcomponent *gen_event(char *id, char *date, char *aircraft, char *instructor,
                         char *equipment, char *tz) {
  if (!id || !date || !aircraft || !instructor || !equipment || !tz)
    return NULL;
  struct icaltimetype dtstart, dtend;
  dtstart = icaltime_null_time();
  dtend = icaltime_null_time();
  char start_ampm, end_ampm;
  int ret = sscanf(date, "%d/%d/%d, %d:%d%cm to<br/>%d/%d/%d, %d:%d%cm",
                   &dtstart.month, &dtstart.day, &dtstart.year, &dtstart.hour,
                   &dtstart.minute, &start_ampm, &dtend.month, &dtend.day,
                   &dtend.year, &dtend.hour, &dtend.minute, &end_ampm);
  if (ret != 12) return NULL;
  if (start_ampm == 'p' && dtstart.hour < 12)
    dtstart.hour += 12;
  else if (start_ampm == 'a' && dtstart.hour == 12)
    dtstart.hour = 0;
  if (end_ampm == 'p' && dtend.hour < 12)
    dtend.hour += 12;
  else if (end_ampm == 'a' && dtend.hour == 12)
    dtend.hour = 0;

  int has_aircraft, has_instructor, has_equipment;
  has_aircraft = strcmp(aircraft, "<i>None</i>");
  has_instructor = strcmp(instructor, "<i>None</i>");
  has_equipment = strcmp(equipment, "<i>None</i>");

  char *summary;
  asprintf(&summary, "%s%s%s%s%s", (has_aircraft ? aircraft : ""),
           (has_aircraft && (has_instructor || has_equipment) ? ", with " : ""),
           (has_instructor ? instructor : ""),
           (has_equipment && (has_aircraft || has_instructor) ? ", " : ""),
           (has_equipment ? equipment : ""));

  icalcomponent *event = icalcomponent_vanew(
      ICAL_VEVENT_COMPONENT, icalproperty_new_summary(summary),
      icalproperty_new_uid(id),
      icalproperty_vanew_dtstart(dtstart, icalparameter_new_tzid(tz), 0),
      icalproperty_vanew_dtend(dtend, icalparameter_new_tzid(tz), 0), 0);
  char *desc;
  asprintf(&desc,
           "ID: %s\nDate: %s\nAircraft: %s\nInstructor: %s\nEquipment: %s\n",
           id, date, aircraft, instructor, equipment);
  icalcomponent_add_property(event, icalproperty_new_description(desc));
  return event;
}

icalcomponent *gen_ical() {
  icalcomponent *cal;
  icalproperty *prop;
  const char *my_product;

  /* Create VCALENDAR component */
  cal = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

  /* Create VERSION property and add to calendar.
     In this case, we create the new property and assign it to a variable
     (perhaps we need to access it later).
     We then pass this variable to the 'add' function.
  */
  prop = icalproperty_new_version("2.0");
  icalcomponent_add_property(cal, prop);

  /* Create PRODID property and add it to calendar.
     In this case, we don't plan on needing the property later
     so we can nest the 'new' function within the 'add' function.
   */
  my_product = "-//nicholascw/aircraftclubs2ical//EN";
  icalcomponent_add_property(cal, icalproperty_new_prodid(my_product));
  return cal;
}

void save_ical(icalcomponent *cal, char *path) {
  FILE *f = fopen(path, "w+");
  fprintf(f, "%s", icalcomponent_as_ical_string(cal));
  fclose(f);
}
