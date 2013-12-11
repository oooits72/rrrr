/* worker.c : worker processes that handle routing requests */

#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <assert.h>
#include <string.h>
#include "config.h"
#include "rrrr.h"
#include "tdata.h"
#include "router.h"
#include "json.h"

#define OUTPUT_LEN 64000

int main(int argc, char **argv) {

    /* SETUP */
    
    // logging
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(PROGRAM_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "worker starting up");
    
    // load transit data from disk
    tdata_t tdata;
    tdata_load(RRRR_INPUT_FILE, &tdata);

    // initialize router
    router_t router;
    router_setup(&router, &tdata);
    //tdata_dump(&tdata); // debug timetable file format

    // establish nanomsg connection
    int sock = nn_socket(AF_SP, NN_REP);
    int rc = nn_bind(sock, WORKER_ENDPOINT);
    // int rc = nn_connect(sock, WORKER_ENDPOINT);
    printf("%s %d", WORKER_ENDPOINT, rc);
    if (rc < 1) exit(1);

    /* MAIN LOOP */
    uint32_t request_count = 0;
    char result_buf[OUTPUT_LEN];
    while (true) {
        router_request_t preq;

        struct nn_msghdr hdr;
        struct nn_iovec iov[2];
        uint32_t sd;

        iov[0].iov_base = &sd;
        iov[0].iov_len = sizeof(sd);
        iov[1].iov_base = &preq;
        iov[1].iov_len = sizeof(preq);

        bzero (&hdr, sizeof (hdr));

        hdr.msg_iov = iov;
        hdr.msg_iovlen = 2;

        nn_recvmsg (sock, &hdr, 0);

        if (++request_count % 100 == 0)
            syslog(LOG_INFO, "worker received %d requests\n", request_count);

        router_request_t req = preq; // protective copy, since we're going to reverse it
        D printf ("Searching with request: \n");
        I router_request_dump (&router, &req);
        router_route (&router, &req);
        // repeat search in reverse to compact transfers
        uint32_t n_reversals = req.arrive_by ? 1 : 2;
        //n_reversals = 0; // DEBUG turn off reversals
        for (uint32_t i = 0; i < n_reversals; ++i) {
            router_request_reverse (&router, &req); // handle case where route is not reversed
            D printf ("Repeating search with reversed request: \n");
            D router_request_dump (&router, &req);
            router_route (&router, &req);
        }
        // uint32_t result_length = router_result_dump(&router, &req, result_buf, OUTPUT_LEN);
        struct plan plan;
        router_result_to_plan (&plan, &router, &req);
        plan.req.time = preq.time; // restore the original request time
        uint32_t result_length = render_plan_json (&plan, router.tdata, result_buf, OUTPUT_LEN);

        iov[1].iov_base = &result_buf;
        iov[1].iov_len = result_length;

        nn_sendmsg(sock, &hdr, 0);
}
    
    /* TEAR DOWN */
    syslog(LOG_INFO, "worker terminating");
    router_teardown(&router);
    tdata_close(&tdata);
    nn_close(sock);
    exit(EXIT_SUCCESS);
}

