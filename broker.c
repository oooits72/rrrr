/* Load-balancing broker using nanomsg. */

#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include "config.h"

int main(int argc, char *argv[]) {

    printf("Broker starting up.\n");

    int frontend = nn_socket (AF_SP_RAW, NN_REP);
    int backend  = nn_socket (AF_SP_RAW, NN_REQ);

    nn_bind (frontend, CLIENT_ENDPOINT);
    nn_bind (backend,  WORKER_ENDPOINT);

    nn_device (frontend, backend);

    printf("Broker exiting.\n");
    return 0;

}
