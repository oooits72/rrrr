#include "rrrr_types.h"
#include "router_result.h"
#include <stdio.h>
#include <string.h>

/* Reverse the times and stops in a leg. Used for creating arrive-by itineraries. */
static void leg_swap (leg_t *leg) {
    struct leg temp = *leg;
    leg->s0 = temp.s1;
    leg->s1 = temp.s0;
    leg->t0 = temp.t1;
    leg->t1 = temp.t0;
}

/* Checks charateristics that should be the same for all vj plans produced by this router:
   All stops should chain, all times should be increasing, all waits should be at the ends of walk legs, etc.
   Returns true if any of the checks fail, false if no problems are detected. */
static bool check_plan_invariants (plan_t *plan) {
    uint8_t i_itinerary;
    bool fail = false;
    itinerary_t *prev_itin = NULL;
    rtime_t prev_target_time = UNREACHED;
    /* Loop over all itineraries in this plan. */
    for (i_itinerary = 0; i_itinerary < plan->n_itineraries; ++i_itinerary) {
        itinerary_t *itin = plan->itineraries + i_itinerary;
        if (itin->n_legs < 1) {
            fprintf(stderr, "itinerary contains no legs.\n");
            fail = true;
        } else {
            /* Itinarary has at least one leg. Grab its first and last leg. */
            leg_t *leg0 = itin->legs;
            leg_t *legN = itin->legs + (itin->n_legs - 1);
            /* Itineraries should be Pareto-optimal. Increase in number of rides implies improving arrival time. */
            rtime_t target_time = plan->req.arrive_by ? leg0->t0 : legN->t1;
            if (prev_itin != NULL) {
                if (itin->n_legs <= prev_itin->n_legs) {
                    fprintf(stderr, "itineraries do not have strictly increasing numbers of legs: %d, %d.\n", prev_itin->n_legs, itin->n_legs);
                    fail = true;
                }
                if (plan->req.arrive_by ? target_time <= prev_target_time : target_time >= prev_target_time) {
                    fprintf(stderr, "itineraries do not have strictly improving target times: %d, %d.\n", prev_target_time, target_time);
                    fail = true;
                }
            }
            prev_target_time = target_time;
            prev_itin = itin;
            /* Check that itinerary does indeed connect the places in the request. */
            if (leg0->s0 != plan->req.from) {
                fprintf(stderr, "itinerary does not begin at from location.\n");
                fail = true;
            }
            if (legN->s1 != plan->req.to) {
                fprintf(stderr, "itinerary does not end at to location.\n");
                fail = true;
            }
            /* Check that the itinerary respects the depart after or arrive-by criterion */
            /* finish when rtimes are in requests
               if (plan->req.arrive_by) {
               if (itin->legs[itin->n_legs - 1].s1 > plan->req.time)...
               } else {
               if (itin->legs[0].s0 < plan->req.time)...
               }
               */
            /* All itineraries are composed of ride-walk pairs, prefixed by a single walk leg. */
            if (itin->n_legs % 2 != 1) {
                fprintf(stderr, "itinerary has an inexplicable (even) number of legs: %d\n", itin->n_legs);
                fail = true;
            }
        }
        {
            /* Check per-leg invariants within each itinerary. */
            leg_t *prev_leg = NULL;
            uint8_t i_leg;
            for (i_leg = 0; i_leg < itin->n_legs; ++i_leg) {
                leg_t *leg = itin->legs + i_leg;
                if (i_leg % 2 == 0) {
                    if (leg->journey_pattern != WALK) fprintf(stderr, "even numbered leg %d has journey_pattern %d not WALK.\n", i_leg, leg->journey_pattern);
                    fail = true;
                } else {
                    if (leg->journey_pattern == WALK) fprintf(stderr, "odd numbered leg %d has journey_pattern WALK.\n", i_leg);
                    fail = true;
                }
                if (leg->t1 < leg->t0) {
                    fprintf(stderr, "non-increasing times within leg %d: %d, %d\n", i_leg, leg->t0, leg->t1);
                    fail = true;
                }
                if (i_leg > 0) {
                    if (leg->s0 != prev_leg->s1) {
                        fprintf(stderr, "legs do not chain: leg %d begins with stop %d, previous leg ends with stop %d.\n", i_leg, leg->s0, prev_leg->s1);
                        fail = true;
                    }
                    if (leg->journey_pattern == WALK && leg->t0 != prev_leg->t1) {
                        /* This will fail unless reversal is being performed */
#if 0
                        fprintf(stderr, "walk leg does not immediately follow ride: leg %d begins at time %d, previous leg ends at time %d.\n", l, leg->t0, prev_leg->t1);
                        fail = true;
#endif
                    }
                    if (leg->t0 < prev_leg->t1) {
                        fprintf(stderr, "itin %d: non-increasing times between legs %d and %d: %d, %d\n",
                                i_itinerary, i_leg - 1, i_leg, prev_leg->t1, leg->t0);
                        fail = true;
                    }
                }
                prev_leg = leg;
            } /* End for (legs) */
        }
    } /* End for (itineraries) */
    return fail;
}

bool router_result_to_plan (struct plan *plan, router_t *router, router_request_t *req) {
    itinerary_t *itin;
    uint8_t i_transfer;
    /* Router states are a 2D array of stride n_stops */
    /* router_state_t (*states)[n_stops] = (router_state_t(*)[]) router->states; */
    plan->n_itineraries = 0;
    plan->req = *req; /* copy the request into the plan for use in rendering */
    itin = plan->itineraries;
    /* Loop over the rounds to get ending states of itineraries using different numbers of vehicles */
    for (i_transfer = 0; i_transfer < RRRR_DEFAULT_MAX_ROUNDS; ++i_transfer) {
        /* Work backward from the target to the origin */
        uint64_t i_state;
        leg_t *l = itin->legs; /* the slot in which record a leg, reversing them for forward vehicle_journey's */
        uint32_t stop = router->target; /* Work backward from the target to the origin */
        int16_t j_transfer; /* signed int because we will be decreasing */

        i_state = (i_transfer * router->tdata->n_stops) + stop;

        /* skip rounds that were not reached */
        if (router->states_walk_time[i_state] == UNREACHED) continue;
        itin->n_rides = i_transfer + 1;
        itin->n_legs = itin->n_rides * 2 + 1; /* always same number of legs for same number of transfers */
        if ( ! req->arrive_by) l += itin->n_legs - 1;
        /* Follow the chain of states backward */
        for (j_transfer = i_transfer; j_transfer >= 0; --j_transfer) {
            uint64_t i_walk, i_ride;
            uint32_t walk_stop, ride_stop;

            i_state = (((uint8_t) j_transfer) * router->tdata->n_stops);

            if (stop > router->tdata->n_stops) {
                fprintf (stderr, "ERROR: stopid %d out of range.\n", stop);
                return false;
            }

            /* Walk phase */
            i_walk = i_state + stop;
            if (router->states_walk_time[i_walk] == UNREACHED) {
                fprintf (stderr, "ERROR: stop %d was unreached by walking.\n", stop);
                return false;
            }
            walk_stop = stop;
            stop = router->states_walk_from[i_walk];  /* follow the chain of states backward */

            /* Ride phase */
            i_ride = i_state + stop;
            if (router->states_time[i_ride] == UNREACHED) {
                fprintf (stderr, "ERROR: stop %d was unreached by riding.\n", stop);
                return false;
            }
            ride_stop = stop;
            stop = router->states_ride_from[i_ride];  /* follow the chain of states backward */

            /* Walk phase */
            l->s0 = router->states_walk_from[i_walk];
            l->s1 = walk_stop;
            l->t0 = router->states_time[i_ride]; /* Rendering the walk requires already having the ride arrival time */
            l->t1 = router->states_walk_time[i_walk];
            l->journey_pattern = WALK;
            l->vj = WALK;

            if (req->arrive_by) leg_swap (l);
            l += (req->arrive_by ? 1 : -1); /* next leg */

            /* Ride phase */
            l->s0 = router->states_ride_from[i_ride];
            l->s1 = ride_stop;
            l->t0 = router->states_board_time[i_ride];
            l->t1 = router->states_time[i_ride];
            l->journey_pattern = router->states_back_journey_pattern[i_ride];
            l->vj = router->states_back_vehicle_journey[i_ride];

            #ifdef RRRR_FEATURE_REALTIME_EXPANDED
            {
                journey_pattern_t *jp;
                vehicle_journey_t *vj;
                uint32_t vj_index;

                jp = router->tdata->journey_patterns + router->states_back_journey_pattern[i_ride];
                vj_index = jp->vj_ids_offset + router->states_back_vehicle_journey[i_ride];
                vj = router->tdata->vjs + vj_index;

                if (router->tdata->vj_stoptimes[vj_index] &&
                    router->tdata->stop_times[vj->stop_times_offset + router->states_journey_pattern_point[i_ride]].arrival != UNREACHED) {

                    l->d0 = RTIME_TO_SEC_SIGNED(router->tdata->vj_stoptimes[vj_index][router->states_back_journey_pattern_point[i_ride]].departure) - RTIME_TO_SEC_SIGNED(router->tdata->stop_times[vj->stop_times_offset + router->states_back_journey_pattern_point[i_ride]].departure + vj->begin_time);
                    l->d1 = RTIME_TO_SEC_SIGNED(router->tdata->vj_stoptimes[vj_index][router->states_journey_pattern_point[i_ride]].arrival) - RTIME_TO_SEC_SIGNED(router->tdata->stop_times[vj->stop_times_offset + router->states_journey_pattern_point[i_ride]].arrival + vj->begin_time);
                } else {
                    l->d0 = 0;
                    l->d1 = 0;
                }
            }
            #endif

            if (req->arrive_by) leg_swap (l);
            l += (req->arrive_by ? 1 : -1);   /* next leg */

        }
        if (req->onboard_journey_pattern_offset != NONE) {
            if (!req->arrive_by) {
                /* Results starting on board do not have an initial walk leg. */
                l->s0 = l->s1 = ONBOARD;
                l->t0 = l->t1 = req->time;
                l->journey_pattern = l->vj = WALK;
                l += 1; /* move back to first transit leg */
                l->s0 = ONBOARD;
                l->t0 = req->time;
            } else {
                #ifdef RRRR_DEBUG
                fprintf(stderr, "We observed an onboard departure with an arrive by.\n");
                #endif
                return false;
            }
        } else {
            /* The initial walk leg leading out of the search origin. This is inferred, not stored explicitly. */
            uint32_t origin_stop = (req->arrive_by ? req->to : req->from);
            rtime_t duration;

            l->s0 = origin_stop;
            l->s1 = stop;
            /* It would also be possible to work from s1 to s0 and compress out the wait time. */
            l->t0 = router->states_time[origin_stop];
            duration = transfer_duration (router->tdata, req, l->s0, l->s1);
            l->t1 = l->t0 + (req->arrive_by ? -duration : +duration);
            l->journey_pattern = WALK;
            l->vj = WALK;
            if (req->arrive_by) leg_swap (l);
        }
        /* Move to the next itinerary in the plan. */
        plan->n_itineraries += 1;
        itin += 1;
    }
    return check_plan_invariants (plan);
}

static char *
plan_render_itinerary (struct itinerary *itin, tdata_t *tdata, char *b, char *b_end) {
    leg_t *leg;

    b += sprintf (b, "\nITIN %d rides \n", itin->n_rides);

    /* Render the legs of this itinerary, which are in chronological order */
    for (leg = itin->legs; leg < itin->legs + itin->n_legs; ++leg) {
        char ct0[16];
        char ct1[16];
        const char *agency_name, *short_name, *headsign, *productcategory, *leg_mode = NULL;
        char *alert_msg = NULL;
        const char *s0_id = tdata_stop_name_for_index(tdata, leg->s0);
        const char *s1_id = tdata_stop_name_for_index(tdata, leg->s1);
        float d0 = 0.0, d1 = 0.0;

        btimetext(leg->t0, ct0);
        btimetext(leg->t1, ct1);

        /* d1 = 0.0; */

        if (leg->journey_pattern == WALK) {
            agency_name = "";
            short_name = "walk";
            headsign = "walk";
            productcategory = "";

            /* Skip uninformative legs that just tell you to stay in the same place. if (leg->s0 == leg->s1) continue; */
            if (leg->s0 == ONBOARD) continue;
            if (leg->s0 == leg->s1) leg_mode = "WAIT";
            else leg_mode = "WALK";
        } else {
            agency_name = tdata_agency_name_for_journey_pattern(tdata, leg->journey_pattern);
            short_name = tdata_line_code_for_journey_pattern(tdata, leg->journey_pattern);
            headsign = tdata_headsign_for_journey_pattern(tdata, leg->journey_pattern);
            productcategory = tdata_productcategory_for_journey_pattern(tdata, leg->journey_pattern);
            #ifdef RRRR_FEATURE_REALTIME_EXPANDED
            d0 = leg->d0 / 60.0f;
            d1 = leg->d1 / 60.0f;
            #endif

            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_tram)      == m_tram)      leg_mode = "TRAM";      else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_subway)    == m_subway)    leg_mode = "SUBWAY";    else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_rail)      == m_rail)      leg_mode = "RAIL";      else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_bus)       == m_bus)       leg_mode = "BUS";       else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_ferry)     == m_ferry)     leg_mode = "FERRY";     else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_cablecar)  == m_cablecar)  leg_mode = "CABLE_CAR"; else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_gondola)   == m_gondola)   leg_mode = "GONDOLA";   else
            if ((tdata->journey_patterns[leg->journey_pattern].attributes & m_funicular) == m_funicular) leg_mode = "FUNICULAR"; else
            leg_mode = "INVALID";

            #ifdef RRRR_FEATURE_REALTIME_ALERTS
            if (leg->journey_pattern != WALK && tdata->alerts) {
                size_t i_entity;
                for (i_entity = 0; i_entity < tdata->alerts->n_entity; ++i_entity) {
                    if (tdata->alerts->entity[i_entity] &&
                        tdata->alerts->entity[i_entity]->alert) {

                        TransitRealtime__Alert *alert = tdata->alerts->entity[i_entity]->alert;
                        size_t i_informed_entity;

                        for (i_informed_entity = 0; i_informed_entity < alert->n_informed_entity; ++i_informed_entity) {
                            TransitRealtime__EntitySelector *informed_entity = alert->informed_entity[i_informed_entity];

                            if ( ( (!informed_entity->route_id) || ((uint32_t) *(informed_entity->route_id) == leg->journey_pattern) ) &&
                                ( (!informed_entity->stop_id)  || ((uint32_t) *(informed_entity->stop_id) == leg->s0) ) &&
                                ( (!informed_entity->trip)     || (!informed_entity->trip->trip_id) || ((uint32_t) *(informed_entity->trip->trip_id) == leg->vj) )
                                /* TODO: need to have rtime_to_date  for informed_entity->vj->start_date */
                                /* TODO: need to have rtime_to_epoch for informed_entity->active_period */
                            ) {
                                alert_msg = alert->header_text->translation[0]->text;
                            }

                            if (alert_msg) break;
                        }

                        /* TODO: theoretically we could have multiple alert messages */
                        if (alert_msg) break;
                    }
                }
            }
            #endif
        }

        /* TODO: we are able to calculate the maximum length required for each line
         * therefore we could prevent a buffer overflow from happening. */
        b += sprintf (b, "%s %5d %3d %5d %5d %s %+3.1f %s %+3.1f ;%s;%s;%s;%s;%s;%s;%s\n",
            leg_mode, leg->journey_pattern, leg->vj, leg->s0, leg->s1, ct0, d0, ct1, d1, agency_name, short_name, headsign, productcategory, s0_id, s1_id,
                        (alert_msg ? alert_msg : ""));

        /* EXAMPLE
        polyline_for_leg (tdata, leg);
        b += sprintf (b, "%s\n", polyline_result());
        */

        if (b > b_end) {
            fprintf (stderr, "buffer overflow\n");
            return b;
            /* exit(2); */
        }
    }

    return b;
}

/* Write a plan structure out to a text buffer in tabular format. */
static uint32_t
plan_render(plan_t *plan, tdata_t *tdata, router_request_t *req, char *buf, uint32_t buflen) {
    char *b = buf;
    char *b_end = buf + buflen;

    if ((req->optimise & o_all) == o_all) {
        /* Iterate over itineraries in this plan, which are in increasing order of number of rides */
        itinerary_t *itin;
        for (itin = plan->itineraries; itin < plan->itineraries + plan->n_itineraries; ++itin) {
            b = plan_render_itinerary (itin, tdata, b, b_end);
        }
    } else if (plan->n_itineraries > 0) {
        if ((req->optimise & o_transfers) == o_transfers) {
            /* only render the first itinerary, which has the least transfers */
            b = plan_render_itinerary (plan->itineraries, tdata, b, b_end);
        }
        if ((req->optimise & o_shortest) == o_shortest) {
            /* only render the last itinerary, which has the most rides and is the shortest in time */
            b = plan_render_itinerary (&plan->itineraries[plan->n_itineraries - 1], tdata, b, b_end);
        }
    }
    *b = '\0';
    return b - buf;
}

/*
  After routing, call to convert the router state into a readable list of itinerary legs.
  Returns the number of bytes written to the buffer.
*/
uint32_t router_result_dump(router_t *router, router_request_t *req, char *buf, uint32_t buflen) {
    plan_t plan;
    if (!router_result_to_plan (&plan, router, req)) {
        return 0;
    }

    /* plan_render_json (&plan, router->tdata, req); */
    return plan_render (&plan, router->tdata, req, buf, buflen);
}

STRUCT_INITILIZER(plan)
STRUCT_INITILIZER(itinerary)
STRUCT_INITILIZER(leg)
STRUCT_ARRAY_GETTER(itinerary, legs, leg)
STRUCT_ARRAY_SETTER(itinerary, legs, leg)
STRUCT_ARRAY_GETTER(plan, itineraries, itinerary)

