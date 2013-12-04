/* client.c : test client */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/time.h>
#include "rrrr.h"
#include "router.h"
#include "config.h"

static bool verbose = false;
static bool randomize = false;
static uint32_t from_s = 0;
static uint32_t to_s = 1;

static void *client_task (void *args) {
    uint32_t n_requests = *((uint32_t *) args);
    syslog (LOG_INFO, "test client thread will send %d requests", n_requests);
    // connect client thread to load balancer
    int sock = nn_socket (AF_SP, NN_REQ);
    int rc = nn_connect(sock, CLIENT_ENDPOINT);
    assert (rc > 0);
    uint32_t request_count = 0;

    // load transit data from disk
    tdata_t tdata;
    tdata_load(RRRR_INPUT_FILE, &tdata);

    while (true) {
        router_request_t req;
        router_request_initialize (&req);
        // unfortunately tdata is not available here or we could initialize from current epoch time
        if (randomize) 
            router_request_randomize (&req, &tdata);
        else {
            req.from=from_s;
            req.to=to_s;
        }
        // if (verbose) router_request_dump(&router, &req); // router is not available in client
        nn_send(sock, &req, sizeof(req), 0);
        // syslog (LOG_INFO, "test client thread has sent %d requests\n", request_count);
        void *msg;
        nn_recv(sock, &msg, NN_MSG, 0);
        char *reply = (char *) msg;
        if (!reply) 
            break;
        if (verbose) 
            printf ("%s", reply);
        nn_freemsg(msg);
        if (++request_count >= n_requests)
            break;
    }
    syslog (LOG_INFO, "test client thread terminating");

    return NULL;
}

void usage() {
    printf("usage: 'client rand [nreqs] [nthreads]' or 'client id [from_stop_id] [to_stop_id]'\n" );
    exit (1);
}

int main (int argc, char **argv) {
    
    // initialize logging
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog (PROGRAM_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    syslog (LOG_INFO, "test client starting");

    // ensure different random requests on different runs
    srand(time(NULL)); 
    
    // read and range-check parameters
    uint32_t n_requests = 1;
    uint32_t concurrency = RRRR_TEST_CONCURRENCY;
    if (argc != 4)
        usage();
    
    if (strcmp(argv[1], "rand") == 0) {
        randomize = true;
    } else if (strcmp(argv[1], "id") == 0) {
        randomize = false;
    } else {
        usage();
    }

    if (randomize && argc > 2) {
        n_requests = atoi(argv[2]);
        if (argc > 3)
            concurrency = atoi(argv[3]);
    } else {
        from_s = atoi(argv[2]);
        to_s = atoi(argv[3]);
        n_requests = 1;
        concurrency = 1;
    }
    if (concurrency < 1 || concurrency > 32)
        concurrency = RRRR_TEST_CONCURRENCY;
    if (concurrency > n_requests)
        concurrency = n_requests;

    syslog (LOG_INFO, "test client number of requests: %d", n_requests);
    syslog (LOG_INFO, "test client concurrency: %d", concurrency);
    verbose = (n_requests == 1);
    
    // divide up work between threads
    uint32_t n_reqs[concurrency];
    for (uint32_t i = 0; i < concurrency; i++)
        n_reqs[i] = n_requests / concurrency;
    for (uint32_t i = 0; i < n_requests % concurrency; i++)
        n_reqs[i] += 1;

    // track runtime
    struct timeval t0, t1;
    
    // create threads
    pthread_t pipes[concurrency];
    gettimeofday(&t0, NULL);
    for (uint32_t n = 0; n < concurrency; n++) {
        pthread_create (&pipes[n], NULL, client_task, n_reqs + n);
    }

    // wait for all threads to complete
    for (uint32_t n = 0; n < concurrency; n++) {
        pthread_join (pipes[n], NULL);
    }

    // output summary information
    gettimeofday(&t1, NULL);
    double dt = ((t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec)) / 1000000.0;
    syslog (LOG_INFO, "%d threads, %d total requests, %f sec total time (%f req/sec)",
        concurrency, n_requests, dt, n_requests / dt);
    
    //printf("%c", '\0x1A');
    
    // teardown
    closelog();
    
}

