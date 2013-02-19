RRRR Rapid Round-based Routing
==============================

Introduction
------------

RRRR (code name: bliksemrouter) is a C-language implementation of the RAPTOR public transit routing algorithm. The goal of this project is to generate sets of Pareto-optimal itineraries over large geographic areas (e.g. BeNeLux or all of Europe), improving on the resource consumption and complexity of existing more flexible alternatives. The system should eventually support real-time vehicle/trip updates reflected in trip plans and be capable of running directly on a mobile device with no Internet connection.

Multiple RRRR processes running on the same machine map the same read-only data file into their private address space. This file contains a structured and indexed representation of transit timetables and other information from a GTFS feed. Additional handler processes should only increase physical memory consumption by the amount needed for search state (roughly 16 * num_stops * max_transfers bytes, on the order of a few megabytes). Eventually the real-time updater process will probably also use memory-mapped files for interprocess communication.

Each worker is a separate process and keeps a permanent scratch buffer that is reused from one request to the next, so no dynamic memory allocation is performed in inner loops. Transit stops are the only locations considered. On-street searches will not be handled in the first phase of development. Eventually we will probably use protocol buffers over a 0MQ fan-out pattern to distribute real-time updates. This is basically standard GTFS-RT over a message passing system instead of HTTP pull.

It looks like in may be possible to keep memory consumption for a Portland, Oregon-sized system under 10MiB. Full Netherlands coverage should be possible in about 50MiB.

Dependencies
------------

1. Graphserver (for its SQLite GTFS tools)
2. zeromq and libczmq
3. gcc


Building transit data
---------------------

Download a GTFS feed for your favorite transit agency (stick with small ones for now).
Run `gs_gtfsdb_compile input.gtfs.zip output.gtfsdb` to load your GTFS feed into an SQLite database.
Next, run `python transfers.py output.gtfsdb` to add distance-based transfers to the database.
Finally, run `python timetable.py output.gtfsdb` to create the timetable file `timetable.dat` based on that GTFS database.


Building and starting up RRRR
-----------------------------

```bash
~/git/rrrr$ make clean && make && ./rrrr.sh
```

you should see make output followed by the broker and workers restarting:

```
killing old processes
rrrr[31023]: broker terminating
rrrr[31024]: worker terminating
rrrr[31025]: worker terminating
rrrr[31026]: worker terminating
rrrr[31027]: worker terminating

starting new processes
rrrr[31109]: broker starting up
rrrr[31111]: worker starting up
rrrr[31110]: worker starting up
rrrr[31112]: worker starting up
rrrr[31113]: worker starting up
rrrr[31111]: worker sent ready message to load balancer
rrrr[31110]: worker sent ready message to load balancer
rrrr[31112]: worker sent ready message to load balancer
rrrr[31113]: worker sent ready message to load balancer
rrrr[31114]: test client starting
rrrr[31114]: test client number of requests: 1
rrrr[31114]: test client concurrency: 1
rrrr[31114]: test client thread will send 1 requests
rrrr[31114]: test client received response: OK
rrrr[31114]: test client thread terminating
rrrr[31114]: 1 threads, 1 total requests, 0.036565 sec total time (27.348557 req/sec)
````

Then you should be able to run the test client:

```
~/git/rrrr$ ./client 1000 4

rrrr[31144]: test client starting
rrrr[31144]: test client number of requests: 1000
rrrr[31144]: test client concurrency: 4
rrrr[31144]: test client thread will send 250 requests
rrrr[31144]: test client thread will send 250 requests
rrrr[31144]: test client thread will send 250 requests
rrrr[31144]: test client thread will send 250 requests
rrrr[31111]: worker received 100 requests
rrrr[31112]: worker received 100 requests
rrrr[31110]: worker received 100 requests
rrrr[31113]: worker received 100 requests
rrrr[31109]: broker: frx 0502 ftx 0499 brx 0499 btx 0502 / 4 workers
rrrr[31112]: worker received 200 requests
rrrr[31111]: worker received 200 requests
rrrr[31113]: worker received 200 requests
rrrr[31110]: worker received 200 requests
rrrr[31144]: test client thread terminating
rrrr[31144]: test client thread terminating
rrrr[31144]: test client thread terminating
rrrr[31144]: test client thread terminating
rrrr[31144]: 4 threads, 1000 total requests, 3.893521 sec total time (256.836935 req/sec)
```
