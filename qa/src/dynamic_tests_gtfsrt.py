from gtfs_realtime_pb2 import FeedMessage, FeedHeader, TripUpdate, TripDescriptor
import time

feedmessage = FeedMessage()
feedmessage.header.gtfs_realtime_version = "1.0"
feedmessage.header.incrementality = FeedHeader.FULL_DATASET

feedmessage.header.timestamp = 1388530800 # 2014-01-01

feedentity = feedmessage.entity.add()
feedentity.id = '3b'
feedentity.trip_update.trip.start_date = '20140101'
feedentity.trip_update.trip.trip_id = '3b|1'
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 0
update.stop_id = '3b1'
update.departure.delay = 600

feedentity = feedmessage.entity.add()
feedentity.id = '3b|2'
feedentity.trip_update.trip.start_date = '20140101'
feedentity.trip_update.trip.trip_id = '3b|2'
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 1
update.stop_id = '3b1'
update.departure.delay = 60

f = open("/tmp/3b.pb", "wb")
f.write(feedmessage.SerializeToString())
f.close()

feedmessage = FeedMessage()
feedmessage.header.gtfs_realtime_version = "1.0"
feedmessage.header.incrementality = FeedHeader.FULL_DATASET

feedmessage.header.timestamp = 1388531040 # 2014-01-01 00:04

feedentity = feedmessage.entity.add()
feedentity.id = '4a'
feedentity.trip_update.trip.start_date = '20140101'
feedentity.trip_update.trip.trip_id = '4a|1'
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 1
update.stop_id = '4a1'
update.arrival.time = int(time.mktime((2014, 01, 01, 0, 6, 0, 0, 0, 0)))
update.departure.time = int(time.mktime((2014, 01, 01, 0, 6, 0, 0, 0, 0)))
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 2
update.stop_id = '4a2'
update.arrival.time = int(time.mktime((2014, 01, 01, 0, 7, 0, 0, 0, 0)))
update.departure.time = int(time.mktime((2014, 01, 01, 0, 7, 0, 0, 0, 0)))
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 3
update.stop_id = '4a3'
update.schedule_relationship = TripUpdate.StopTimeUpdate.SKIPPED;

f = open("/tmp/4a-skipped.pb", "wb")
f.write(feedmessage.SerializeToString())
f.close()

feedmessage = FeedMessage()
feedmessage.header.gtfs_realtime_version = "1.0"
feedmessage.header.incrementality = FeedHeader.FULL_DATASET

feedmessage.header.timestamp = 1388531040 # 2014-01-01 00:04

feedentity = feedmessage.entity.add()
feedentity.id = '4a'
feedentity.trip_update.trip.start_date = '20140101'
feedentity.trip_update.trip.trip_id = '4a|1'
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 1
update.stop_id = '4a1'
update.arrival.time = int(time.mktime((2014, 01, 01, 0, 6, 0, 0, 0, 0)))
update.departure.time = int(time.mktime((2014, 01, 01, 0, 6, 0, 0, 0, 0)))
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 2
update.stop_id = '4a2'
update.arrival.time = int(time.mktime((2014, 01, 01, 0, 7, 0, 0, 0, 0)))
update.departure.time = int(time.mktime((2014, 01, 01, 0, 7, 0, 0, 0, 0)))
update = feedentity.trip_update.stop_time_update.add()
update.stop_sequence = 3
update.stop_id = '4a3'
update.schedule_relationship = TripUpdate.StopTimeUpdate.SKIPPED;
update = feedentity.trip_update.stop_time_update.add()
update.stop_id = '4a4'
update.arrival.time = int(time.mktime((2014, 01, 01, 0, 8, 0, 0, 0, 0)))
update.departure.time = int(time.mktime((2014, 01, 01, 0, 8, 0, 0, 0, 0)))
update.schedule_relationship = TripUpdate.StopTimeUpdate.SCHEDULED;

f = open("/tmp/4a-added.pb", "wb")
f.write(feedmessage.SerializeToString())
f.close()

feedmessage = FeedMessage()
feedmessage.header.gtfs_realtime_version = "1.0"
feedmessage.header.incrementality = FeedHeader.FULL_DATASET

feedmessage.header.timestamp = 1388531040 # 2014-01-01 00:04

feedentity = feedmessage.entity.add()
feedentity.id = '3e'
feedentity.trip_update.trip.start_date = '20140101'
feedentity.trip_update.trip.trip_id = '3e|1'
feedentity.trip_update.trip.schedule_relationship = TripDescriptor.CANCELED

f = open("/tmp/3e.pb", "wb")
f.write(feedmessage.SerializeToString())
f.close()

