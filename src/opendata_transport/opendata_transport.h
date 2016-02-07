#pragma once

#include <pebble.h>

#define OPENDATA_TRANSPORT_BUFFER_SIZE 512

//! Possible statuses of the opendata-transport library
typedef enum {
  //! OpendataTransport library has not yet initiated a fetch
  OpendataTransportStatusNotYetFetched = 0,
  //! Bluetooth is disconnected
  OpendataTransportStatusBluetoothDisconnected,
  //! OpendataTransport data fetch is in progress
  OpendataTransportStatusPending,
  //! OpendataTransport fetch failed
  OpendataTransportStatusFailed,
  //! OpendataTransport fetched and available
  OpendataTransportStatusAvailable,
  //! Locations Url was bad
  OpendataTransportStatusBadLocationsUrl,
  //! Stationboard Url was bad
  OpendataTransportStatusBadStationboardUrl,
  //! Location not available
  OpendataTransportStatusLocationUnavailable
} OpendataTransportStatus;

//! Struct containing opendata-transport data
typedef struct {
  //! list of stops e.g: "Bern, Elfenau"
  char stops[OPENDATA_TRANSPORT_BUFFER_SIZE];
  //! list of departure times in minutes
  char times[OPENDATA_TRANSPORT_BUFFER_SIZE];

} OpendataTransportInfo;

//! Callback for a opendata-transport fetch
//! @param info The struct containing the opendata-transport data
//! @param status The current OpendataTransportStatus, which may have changed.
typedef void(OpendataTransportCallback)(OpendataTransportInfo *info, OpendataTransportStatus status);

//! Initialize the opendata-transport library. The data is fetched after calling this, and should be accessed
//! and stored once the callback returns data, if it is successful.
void opendata_transport_init();

//! Important: This uses the AppMessage system. You should only use AppMessage yourself
//! either before calling this, or after you have obtained your opendata-transport data.
//! @param callback Callback to be called once the opendata-transport.
//! @return true if the fetch message to PebbleKit JS was successful, false otherwise.
bool opendata_transport_fetch(int stop, OpendataTransportCallback *callback);

//! Deinitialize and free the backing OpendataTransportInfo.
void opendata_transport_deinit();

//! Peek at the current state of the opendata-transport library. You should check the OpendataTransportStatus of the
//! returned OpendataTransportInfo before accessing data members.
//! @return OpendataTransportInfo object, internally allocated.
//! If NULL, opendata_transport_init() has not been called.
OpendataTransportInfo* opendata_transport_peek();
