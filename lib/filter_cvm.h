/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 2, December 2004

    Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

    Everyone is permitted to copy and distribute verbatim or modified
    copies of this license document, and changing it is allowed as long
    as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

    0. You just DO WHAT THE FUCK YOU WANT TO.

    Illustrate filter.h, in this example we first check "cvm" by MAGIC
    then check "Suppleme"ntarInfoBox by MAGIC too.

    Then filter with a expensive regex (which can be bypased in fact)
    and then extract room / asin from SupplementarInfoBox
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <inttypes.h>  // pro PRIx64 stuff

#ifndef FILTER_H
#include "filter.h"
#endif

#include "./htmlrequest.h"

// in string : 'cvm['
#define MAGIC_CVM  0x5b6d7663

// in string : 'Suppleme'
#define MAGIC_SupplementarInfoBox 0x656d656c70707553

// in string : 'BookletM'
#define MAGIC_BookletManager 0x4d74656c6b6f6f42

static bool checkMAGIC_CVM(const char** line);
static bool check_MAGIC_SupplementarInfoBox(const char** line);
static bool check_regexp_SupplementarInfoBox(const char** line);
static void extractdata_SupplementarInfoBox(const char** line, char** asin,
        char** word);
void process_SupplementarInfoBox(const char** line);

static bool check_MAGIC_BookletManager(const char** line);
static bool check_regexp_BookletManager(const char** line);
static void extractdata_BookletManager(const char** line, char** asin,
                                       char** word);
void process_BookletManager(const char** line);


static bool checkMAGIC_CVM(const char** line) {
    //~ (void)printf("\OKx %"PRIx32" vs %"PRIx32" ", (uint32_t) *log_prefix, (uint32_t)MAGIC_CVM );
    uint32_t* log_prefix = (uint32_t*) *line;
    return *log_prefix == MAGIC_CVM;
}

/* *****************************   SupplementarInfoBox    ******************************** */

static bool check_MAGIC_SupplementarInfoBox(const char** line) {
    bool result = false;          // line is supposed to first not be MAGIC
    if  (checkMAGIC_CVM(line)) {  // Ensure line starts with 'cvm['
        char* colon;              // future pointer to the first colon
        uint64_t* linemagic;             // future pointer to the MAGIC
        colon = strchr(*line, ':');
        // No need to check colon is NULL cause if we have a cvm[ we have at least one colon
        // It's possible syslog fails at sending log line but is it our problem ?
        colon += 4;                      // MAGIC is 4 chars after ':' cvm[1234]: I S'
        linemagic = (uint64_t*) colon;   // pointing to colon
        //~ (void)printf("\OKx %"PRIx64" vs %"PRIx64" ", (uint64_t) *linemagic,
        //~ (uint64_t)MAGIC_SupplementarInfoBox );
        result = (MAGIC_SupplementarInfoBox == *linemagic); // updating result
    }
    return result;
}



static bool check_regexp_SupplementarInfoBox(const char** line) {
    // test is string contains SupplementarInfoBox one asin and one word with both
    // have at least one character length
    const char* regexp_SupplementarInfoBox =
        "SupplementarInfoBox:QUICK_LOOKUP:asin=.+,word=.+:$";
    return check_regexp(line, &regexp_SupplementarInfoBox);
}

static void extractdata_SupplementarInfoBox(const char** line,
        char** asin, char** word) {
    char* lastcursor = (char*) *line;  // lastcursor point to first char of line
    // According to doc, and relying on RegExp, we first extract asin starting from
    // the beginnig ont line
    lastcursor = extract_substring((const char**)&lastcursor, asin, '=', ',');
    // and then extract room from the last comma.
    (void)extract_substring((const char**) &lastcursor, word, '=', ':');
    // Now asin & word contains their value.
}

void process_SupplementarInfoBox(const char** line) {
    if (strlen(*line) > 4 && check_MAGIC_SupplementarInfoBox(line)) {
        if (check_regexp_SupplementarInfoBox(line)) {
            char* room;
            char* asin;
            extractdata_SupplementarInfoBox(line, (char**) &asin, (char**) &room);
            char* url_request;
            if (-1 != asprintf(&url_request, "?toggle=%s", room)) {
                (void)printf("url_request=%s",url_request);
                do_http_request(url_request);
                free(url_request);
            } else {
                perror("process_SupplementarInfoBox() can't build url_request");
            }
            free(room);
            free(asin);
        }
    }
}

/* *****************************   BookletManager    ******************************** */

static bool check_MAGIC_BookletManager(const char** line) {
    bool result = false;          // line is supposed to first not be MAGIC
    if  (checkMAGIC_CVM(line)) {  // Ensure line starts with 'cvm['
        char* colon;              // future pointer to the first colon
        uint64_t* linemagic;             // future pointer to the MAGIC
        colon = strchr(*line, ':');
        // No need to check colon is NULL cause if we have a cvm[ we have at least one colon
        // It's possible syslog fails at sending log line but is it our problem ?
        colon += 4;                      // MAGIC is 4 chars after ':' cvm[1234]: I S'
        linemagic = (uint64_t*) colon;   // pointing to colon
        //~ (void)printf("\nOKx %"PRIx64" vs %"PRIx64" for %s", (uint64_t) *linemagic, (uint64_t)MAGIC_BookletManager,colon );
        result = (MAGIC_BookletManager == *linemagic); // updating result
    }
    return result;
}

static bool check_regexp_BookletManager(const char** line) {
    // test is string contains SupplementarInfoBox one asin and one word with both
    // have at least one character length
    const char* regexp_BookletManager =
        "BookletManager:SwitchingBooklets:from=.+,to=.+:$";
    return check_regexp(line, &regexp_BookletManager);
}

static void extractdata_BookletManager(const char** line,
                                       char** first, char** second) {
    char* lastcursor = (char*) *line;  // lastcursor point to first char of line
    // According to doc, and relying on RegExp, we first extract asin starting from
    // the beginnig ont line
    lastcursor = extract_substring((const char**)&lastcursor, first, '=', ',');
    // and then extract room from the last comma.
    (void)extract_substring((const char**) &lastcursor, second, '=', ':');
    // Now asin & word contains their value.
}

void process_BookletManager(const char** line) {
    if (strlen(*line) > 47 && check_MAGIC_BookletManager(line)) {
        if (check_regexp_BookletManager(line)) {
            char* from;
            char* to;
            extractdata_BookletManager(line, (char**) &from, (char**) &to);
            //do_http_request(room);
            (void)printf("\nBookletManager user went to %s from %s\n", from, to);
            free(from);
            free(to);
        }
    }
}


/* *****************************   Garbage  ¯\_(ツ)_/¯   ******************************** */

/*
    //~ Old version with array instead of pointer.
    void process_SupplementarInfoBox(const char* line[STATIC_MAXGETLINE]) {
    if (strlen(*line) > 4 && check_MAGIC_SupplementarInfoBox(line)) {
        if (check_regexp_SupplementarInfoBox(line)) {
            char room[MAX_GETLINE + 1];
            char asin[MAX_GETLINE + 1];
            // I did'nt manage to pass asin & room as parameters to extractdata_
            // SupplementarInfoBox(). If you know how to do that, ping me please !
            char** proom = (char**) &room;
            char** pasin = (char**) &asin;
            //~ (void)printf("process_SupplementarInfoBox() with %s\n", *line);
            // Better use  size_t nmatch / regmatch_t pmatch…
            //~ (void)printf("[%zu] : **>%s<**\n", strlen(*line), *line);
            extractdata_SupplementarInfoBox(line, (char**) &pasin, (char**) &proom);
            //~ (void)printf("  \\_____ match : asin=%s /", asin);
            //~ (void)printf(" word=%s", room);
            //~ (void)printf("\n");
            //~ do_http_request(room);
        }
    }
    }
*/
