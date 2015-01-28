#ifndef _ROUTER_RESULT_H
#define _ROUTER_RESULT_H

#include "config.h"
#include "util.h"
#include "rrrr_types.h"
#include "router.h"

/* A leg represents one ride or walking transfer. */
typedef struct leg leg_t;
struct leg {
    /* journey_pattern index */
    uint32_t journey_pattern;

    /* vj index */
    uint32_t vj;

    /* from stop index */
    spidx_t s0;

    /* to stop index */
    spidx_t s1;

    /* start time */
    rtime_t  t0;

    /* end time */
    rtime_t  t1;

    #ifdef RRRR_FEATURE_REALTIME
    /* start delay */
    int16_t d0;

    /* end delay */
    int16_t d1;
    #endif
};

STRUCT_INITILIZER_DEFINITION(leg)

/* An itinerary is a chain of legs leading from one place to another. */
typedef struct itinerary itinerary_t;
struct itinerary {
    uint32_t n_rides;
    uint32_t n_legs;
    leg_t legs[RRRR_DEFAULT_MAX_ROUNDS * 2 + 1];
};

STRUCT_INITILIZER_DEFINITION(itinerary)

STRUCT_ARRAY_GETTER_DEFINITION(itinerary, leg)

STRUCT_ARRAY_SETTER_DEFINITION(itinerary, leg)


/* A plan is several pareto-optimal itineraries connecting the same two stops. */
typedef struct plan plan_t;
struct plan {
    uint32_t n_itineraries;
    itinerary_t itineraries[RRRR_DEFAULT_MAX_ROUNDS];
    router_request_t req;
};

STRUCT_INITILIZER_DEFINITION(plan)

STRUCT_ARRAY_GETTER_DEFINITION(plan, itinerary)


bool router_result_to_plan (struct plan *plan, router_t *router, router_request_t *req);

/* return num of chars written */
uint32_t router_result_dump(router_t*, router_request_t*, char *buf, uint32_t buflen);

#endif
