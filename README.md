# AircraftClubs2iCal
Generate iCal 2.0 for your reservations @ AircraftClubs.com

### Dependencies:
 - libical
 - libcurl
 - some `_GNU_SOURCE` functions

### Build:
 Just run `make` in the root directory of this repository.

### Usage:

Login your aircraftclubs.com account and tick "Remember Me". Copy values of `accname` and `accpass` from your Cookies.

Then,

```bash
$ ./ac2ical -u <accname from cookie> -p <accpass from cookie> -o <output ical filename> -t <tzid of timezone>
```

For example,
```cron
0 0 * * * /path/to/ac2ical -u johndoe -p deadbeef...deadbeef -o /var/www/johndoe.reservations.ics -t /freeassociation.sourceforge.net/America/New_York
```
and run any HTTP server you desire with root points to `/var/www` and listen on `example.com` should help you easily subscribe this calendar in Google Calendar or other services with link `https://example.com/johndoe.reservations.ics`.
