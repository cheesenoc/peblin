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
  console.log('PeblinAppMessageKeyDeparture: ' + json.stationboard[0].stop.departure);

  Pebble.sendAppMessage({
    'PeblinAppMessageKeyReply': 1,
    'PeblinAppMessageKeyStop': json.stationboard[0].stop.station.name,
    'PeblinAppMessageKeyLine': json.stationboard[0].name,
    'PeblinAppMessageKeyDestination': json.stationboard[0].to,
    'PeblinAppMessageKeyDeparture': json.stationboard[0].stop.departure
  });
}

function opendataTransportLocationSuccess(pos) {
  var url = 'http://transport.opendata.ch/v1/stationboard?station=elfenau&limit=1';

  // get location, for later
  // var url = 'http://transport.opendata.ch/v1/location?x=' +
  //   pos.coords.latitude + '&y=' + pos.coords.longitude + '&limit=1';

  console.log('opendata-transport: Location success. Contacting transport.opendata.ch: \n' + url);

  opendataTransportXHR(url, 'GET', function(responseText) {
    console.log('opendata-transport: Got API response: \n' + responseText);
    if(responseText.length > 100) {
      opendataTransportSendToPebble(JSON.parse(responseText));
    } else {
      console.log('opendata-transport: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'PeblinAppMessageKeyBadKey': 1
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
