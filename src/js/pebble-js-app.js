/*************************** OpendataTransport library start ****************************/

function opendataTransportXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function opendataTransportSendToPebble(json) {
  console.log('PeblinAppMessageKeyStop: ' + json.stationboard[0].stop.station.name);
  console.log('PeblinAppMessageKeyLine: ' + json.stationboard[0].name);
  console.log('PeblinAppMessageKeyDestination: ' + json.stationboard[0].to);
  console.log('PeblinAppMessageKeyDeparture: ' + json.stationboard[0].stop.departureTimestamp);

  Pebble.sendAppMessage({
    'PeblinAppMessageKeyReply': 1,
    'PeblinAppMessageKeyStop': json.stationboard[0].stop.station.name,
    'PeblinAppMessageKeyLine': json.stationboard[0].name,
    'PeblinAppMessageKeyDestination': json.stationboard[0].to,
    'PeblinAppMessageKeyDeparture': json.stationboard[0].stop.departureTimestamp
  });
}

function opendataTransportLocationSuccess(pos) {

  var url_locations = 'http://transport.opendata.ch/v1/locations?x=' +
    pos.coords.latitude + '&y=' + pos.coords.longitude + '&limit=1';

  console.log('opendata-transport: Location success. Contacting transport.opendata.ch: \n' + url_locations);

  opendataTransportXHR(url_locations, 'GET', function(responseTextLocation) {
    console.log('opendata-transport: Got API response for location: \n' + responseTextLocation);
    if(responseTextLocation.length > 100) {
      var station_id = JSON.parse(responseTextLocation).stations[0].id; //'008590063' for 'Bern, Elfenau';
      var url_stationboard = 'http://transport.opendata.ch/v1/stationboard?station=' + station_id + '&limit=1';

      opendataTransportXHR(url_stationboard, 'GET', function(responseTextStationboard) {
        console.log('opendata-transport: Got API response for stationboard: \n' + responseTextStationboard);
        if(responseTextStationboard.length > 100) {
          opendataTransportSendToPebble(JSON.parse(responseTextStationboard));
        } else {
          console.log('opendata-transport: API response for stationboard was bad. Wrong URL?');
          Pebble.sendAppMessage({
            'PeblinAppMessageKeyBadStationboardUrl': 1
          });
        }
      });
    } else {
      console.log('opendata-transport: API response for location was bad. Wrong URL?');
      Pebble.sendAppMessage({
        'PeblinAppMessageKeyBadLocationsUrl': 1
      });
    }
  });
}

function opendataTransportLocationError(err) {
  console.log('opendata-transport: Location error');
  Pebble.sendAppMessage({
    'PeblinAppMessageKeyLocationUnavailable': 1
  });
}

function opendataTransportHandler(dict) {
  console.log('opendata-transport: Got fetch request from C app');

  navigator.geolocation.getCurrentPosition(opendataTransportLocationSuccess, opendataTransportLocationError, {
    timeout: 15000,
    maximumAge: 60000
  });
}

/**************************** OpendataTransport library end *****************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('appmessage: ' + JSON.stringify(e.payload));
  opendataTransportHandler(e);
});
