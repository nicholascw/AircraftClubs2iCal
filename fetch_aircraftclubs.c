#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gen_ical.h"

struct memory {
  char *response;
  size_t size;
};

struct ll {
  struct ll *next;
  char *str;
};

void curl_err_exit(CURLcode res);
size_t cb(void *data, size_t size, size_t nmemb, void *userp);
char *get_element_content(char *html, char *element_name, char **at);
struct ll *get_element_arr(char *html, char *element_name);
void destroy_ll(struct ll *gg);

char curl_errbuf[CURL_ERROR_SIZE];
struct memory chunk = {0};

char *fetch_aircraftclubs(char *cookies, char *other_mid) {
  curl_global_init(CURL_GLOBAL_SSL);
  CURL *curl = curl_easy_init();
  if (curl) {
    curl_errbuf[0] = '\0';
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    // curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
    res = curl_easy_setopt(
        curl, CURLOPT_URL,
        "https://www.aircraftclubs.com/pages/view/profile.php");
    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    if (res != CURLE_OK) curl_err_exit(res);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) curl_err_exit(res);
    long rcode;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);
    if (res != CURLE_OK) curl_err_exit(res);
    printf("responded: %ld\n", rcode);
    if (rcode != 200) exit(1);

    // get member id
    char *member_id = NULL;
    if (other_mid) {
      member_id = strdup(other_mid);
    } else {
      char *mid = strstr(chunk.response, "/pages/edit/member.php?id=");
      if (!mid) {
        fprintf(stderr, "Failed to locate member id\n");
        exit(1);
      }
      mid += strlen("/pages/edit/member.php?id=");
      char *mid_end = strstr(mid, "';");
      if (!mid_end || mid_end - mid <= 0) {
        fprintf(stderr, "Failed to locate member id\n");
        exit(1);
      }
      member_id = strndup(mid, mid_end - mid);
    }
    printf("member id: %s\n", member_id);

    // get PHPSESSID
    char *sessid = strstr(chunk.response, "PHPSESSID=");
    if (!sessid) {
      fprintf(stderr, "Failed to locate PHPSESSID\n");
      exit(1);
    }
    sessid += strlen("PHPSESSID=");
    char *sessid_end = strstr(sessid, ";");
    if (!sessid_end || sessid_end - sessid <= 0) {
      fprintf(stderr, "Failed to locate PHPSESSID\n");
      exit(1);
    }
    char *PHPSESSID = strndup(sessid, sessid_end - sessid);
    printf("PHPSESSID: %s\n", PHPSESSID);

    free(chunk.response);
    chunk.response = 0;
    chunk.size = 0;

    char *new_cookies;
    asprintf(&new_cookies, "filterType=a; reserveView=c; PHPSESSID=%s; %s",
             PHPSESSID, cookies);
    curl_easy_setopt(curl, CURLOPT_COOKIE, new_cookies);

    char *formUpcoming;
    asprintf(&formUpcoming,
             "p=%s&fromDate=01%%2F01%%2F2000&toDate=12%%2F31%%2F2030",
             member_id);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://www.aircraftclubs.com/functions/booking/"
                     "getBookingHistory.php");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, formUpcoming);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(formUpcoming));
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) curl_err_exit(res);
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rcode);
    if (res != CURLE_OK) curl_err_exit(res);
    printf("responded: %ld\n", rcode);
    if (rcode != 200) exit(1);
    return get_element_content(chunk.response, "table", NULL);
  }
  return NULL;
}

void destroy_ll(struct ll *gg) {
  while (gg) {
    struct ll *gg_next = gg->next;
    if (gg->str) free(gg->str);
    free(gg);
    gg = gg_next;
  }
}

void parse_table(char *table, icalcomponent *cal, char *tzid) {
  char *thead = get_element_content(table, "thead", NULL);
  if (!thead) return;
  char *thead_tr = get_element_content(thead, "tr", NULL);
  free(thead);
  if (!thead_tr) return;
  struct ll *head = get_element_arr(thead_tr, "t");
  free(thead_tr);
  char *tbody = get_element_content(table, "tbody", NULL);
  if (!tbody) {
    destroy_ll(head);
    return;
  }
  struct ll *tbody_rows = get_element_arr(tbody, "tr");
  free(tbody);
  int row_cnt = 0;
  struct ll *tbody_rows_ptr = tbody_rows;
  while (tbody_rows_ptr) {
    if (tbody_rows_ptr->str) {
      row_cnt++;
      struct ll *row_head = get_element_arr(tbody_rows_ptr->str, "t");
      char *id = NULL, *date = NULL, *aircraft = NULL, *instructor = NULL,
           *equipment = NULL, *tach = NULL, *hobbs = NULL;
      struct ll *head_it = head;
      struct ll *col_it = row_head;
      while (head_it && col_it) {
        if (!head_it->str || !col_it->str) {
          destroy_ll(row_head);
          break;
        }
        if (!strcmp(head_it->str, "id") && !id)
          id = col_it->str;
        else if (!strcmp(head_it->str, "Date") && !date)
          date = col_it->str;
        else if (!strcmp(head_it->str, "Aircraft") && !aircraft)
          aircraft = col_it->str;
        else if (!strcmp(head_it->str, "Instructor") && !instructor)
          instructor = col_it->str;
        else if (!strcmp(head_it->str, "Equipment") && !equipment)
          equipment = col_it->str;
        else if (!strcmp(head_it->str, "Tach Hours") && !tach)
          tach = col_it->str;
        else if (!strcmp(head_it->str, "Hobbs Hours") && !hobbs)
          hobbs = col_it->str;
        head_it = head_it->next;
        col_it = col_it->next;
      }
      icalcomponent *event_this_row = gen_event(id, date, aircraft, instructor,
                                                equipment, tach, hobbs, tzid);
      icalcomponent_add_component(cal, event_this_row);
      destroy_ll(row_head);
    }
    tbody_rows_ptr = tbody_rows_ptr->next;
  }
  fprintf(stderr, "%d rows processed.\n", row_cnt);
  destroy_ll(head);
  return;
}

struct ll *get_element_arr(char *html, char *element_name) {
  struct ll *head_ll = NULL, *this_ll;
  char *end_of_html = html + strlen(html);
  char *ptr = html;
  char *next, *next_ptr;
  while ((next = get_element_content(ptr, element_name, &next_ptr)) &&
         next_ptr < end_of_html) {
    if (!head_ll) {
      head_ll = calloc(1, sizeof(struct ll));
      if (!head_ll) return NULL;
      this_ll = head_ll;
    } else {
      this_ll->next = calloc(1, sizeof(struct ll));
      if (!this_ll->next) {
        destroy_ll(head_ll);
        return NULL;
      }
      this_ll = this_ll->next;
    }
    this_ll->str = next;
    ptr = next_ptr;
  }
  return head_ll;
}

void curl_err_exit(CURLcode res) {
  size_t len = strlen(curl_errbuf);
  fprintf(stderr, "\nlibcurl: (%d) ", res);
  if (len)
    fprintf(stderr, "%s%s", curl_errbuf,
            ((curl_errbuf[len - 1] != '\n') ? "\n" : ""));
  else
    fprintf(stderr, "%s\n", curl_easy_strerror(res));
  exit(res);
}

size_t cb(void *data, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)userp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (ptr == NULL) return 0; /* out of memory! */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
  return realsize;
}

char *get_element_content(char *html, char *element_name, char **at) {
  char *searching_buf;
  asprintf(&searching_buf, "<%s", element_name);
  char *starting = strcasestr(html, searching_buf);
  free(searching_buf);
  if (!starting) return NULL;
  char *starting_end = strchr(starting, '>');
  if (!starting_end) return NULL;
  starting_end++;
  asprintf(&searching_buf, "</%s", element_name);
  char *ending_start = strcasestr(starting_end, searching_buf);
  if (at) *at = ending_start + strlen(searching_buf);
  free(searching_buf);
  if (!ending_start || ending_start < starting_end) return NULL;
  return strndup(starting_end, ending_start - starting_end);
}
