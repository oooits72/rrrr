/* Load-balancing broker using nanomsg. Borrows heavily from poll.c in nanomsg tests. */

#include <stdio.h>
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <sys/select.h>
#include "config.h"

int main(int argc, char *argv[]) {

    fd_set pollset;
    size_t fdsz;
    int rcvfd_frontend;
    int rcvfd_backend;
    int rc;

    int frontend = nn_socket (AF_SP, NN_REP);
    int backend = nn_socket (AF_SP, NN_REQ);

    nn_bind(frontend, CLIENT_ENDPOINT);
    nn_bind(backend, WORKER_ENDPOINT);

    fdsz = sizeof (rcvfd_frontend);
    rc = nn_getsockopt (frontend, NN_SOL_SOCKET, NN_RCVFD, (char*) &rcvfd_frontend, &fdsz);

    fdsz = sizeof (rcvfd_backend);
    rc = nn_getsockopt (backend, NN_SOL_SOCKET, NN_RCVFD, (char*) &rcvfd_backend, &fdsz);

    int maxfd = rcvfd_backend + 1;
    if (rcvfd_frontend > rcvfd_backend)
        maxfd = rcvfd_frontend + 1;

    while (1) {
        FD_ZERO (&pollset);
        FD_SET (rcvfd_frontend, &pollset);
        FD_SET (rcvfd_backend, &pollset);

        rc = select(maxfd, &pollset, NULL, NULL, NULL);
        if (FD_ISSET (rcvfd_frontend, &pollset)) {
            void *buf = NULL;
            size_t nbytes = nn_recv (frontend, &buf, NN_MSG, 0);
            if (nbytes > 0) {
                nn_send(backend, buf, nbytes, 0);
                nn_freemsg(buf);
            }
        }
        if (FD_ISSET (rcvfd_backend, &pollset)) {
            void *buf = NULL;
            size_t nbytes = nn_recv (backend, &buf, NN_MSG, 0);
            if (nbytes > 0) {
                nn_send(frontend, buf, nbytes, 0);
                nn_freemsg(buf);
            }
        }
    }

    return 0;
}
