#pragma once

#include <pebble.h>

#define OPENDATA_TRANSPORT_BUFFER_SIZE 32

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
  //! API key was bad
  OpendataTransportStatusBadKey,
  //! Location not available
  OpendataTransportStatusLocationUnavailable
} OpendataTransportStatus;

//! Struct containing opendata-transport data
typedef struct {
  //! stop e.g: "Bern, Elfenau"
  char stop[OPENDATA_TRANSPORT_BUFFER_SIZE];
  //! line e.g: "NFB19"
  char line[OPENDATA_TRANSPORT_BUFFER_SIZE];
  //! destination, e.g: "Blinzern, Köniz"
  char destination[OPENDATA_TRANSPORT_BUFFER_SIZE];
  //! departure time, e.g. "23.13"
  char departure[OPENDATA_TRANSPORT_BUFFER_SIZE];
  //! Date that the data was received
  // time_t timestamp;
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
bool opendata_transport_fetch(OpendataTransportCallback *callback);

//! Deinitialize and free the backing OpendataTransportInfo.
void opendata_transport_deinit();

//! Peek at the current state of the opendata-transport library. You should check the OpendataTransportStatus of the
//! returned OpendataTransportInfo before accessing data members.
//! @return OpendataTransportInfo object, internally allocated.
//! If NULL, opendata_transport_init() has not been called.
OpendataTransportInfo* opendata_transport_peek();
