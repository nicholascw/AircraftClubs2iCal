#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <libical/ical.h>
#include <libical/icaltimezone.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

icalcomponent *gen_event(char *id, char *date, char *aircraft, char *instructor,
                         char *equipment, char *tach, char *hobbs, char *tzid) {
  if (!id || !date || !aircraft || !instructor || !equipment || !tach ||
      !hobbs || !tzid)
    return NULL;
  icaltimezone *tz = icaltimezone_get_builtin_timezone_from_tzid(tzid);
  if (!tz) return NULL;
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
           (has_aircraft && has_instructor ? ", with " : ""),
           (has_instructor ? instructor : ""),
           (has_equipment && (has_aircraft || has_instructor) ? ", " : ""),
           (has_equipment ? equipment : ""));

  // char *epoch_n_id;
  // asprintf(&epoch_n_id, "%lu-%s", (unsigned long)time(NULL), id);
  icalcomponent *event = icalcomponent_vanew(
      ICAL_VEVENT_COMPONENT, icalproperty_new_summary(summary),
      icalproperty_new_dtstamp(dtstart), icalproperty_new_uid(id),
      icalproperty_vanew_dtstart(dtstart, icalparameter_new_tzid(tzid), 0),
      icalproperty_vanew_dtend(dtend, icalparameter_new_tzid(tzid), 0),
      icalproperty_new_lastmodified(icaltime_current_time_with_zone(tz)), 0);
  char *desc;

  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  asprintf(&desc,
           "ID: %s\nDate: %s\nAircraft: %s\nInstructor: %s\nEquipment: "
           "%s\nTach Hours: %s\nHobbs Hours: %s\nFetched at: %s\n",
           id, date, aircraft, instructor, equipment, tach, hobbs,
           asctime(timeinfo));
  icalcomponent_add_property(event, icalproperty_new_description(desc));
  return event;
}

icalcomponent *gen_ical(char *tzid) {
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
  icaltimezone *tz = icaltimezone_get_builtin_timezone_from_tzid(tzid);
  if (!tz) {
    fprintf(stderr, "TZID not found!\n");
    return NULL;
  }
  icalcomponent *vtimezone = icaltimezone_get_component(tz);
  icalcomponent_add_component(cal, vtimezone);
  return cal;
}

void save_ical(icalcomponent *cal, char *path) {
  FILE *f = fopen(path, "w+");
  char *ical_str = icalcomponent_as_ical_string(cal);
  if (f && ical_str) {
    fprintf(f, "%s", ical_str);
    fclose(f);
  } else {
    fprintf(stderr,
            "Failed to open path %s or error occurred in "
            "icalcomponent_as_ical_string().\n",
            path);
  }
}

void print_tzids() {
  icalarray *arr = icaltimezone_get_builtin_timezones();
  if (arr && arr->num_elements > 0) {
    for (size_t i = 0; i < arr->num_elements; i++) {
      icaltimezone *this_tz = icalarray_element_at(arr, i);
      printf("%s\n", icaltimezone_get_tzid(this_tz));
    }
  }
}
