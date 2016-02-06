/*************************** OpendataTransport library start ****************************/
var number;

function opendataTransportXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function shorten(stop) {
  // console.log('shorten:' + stop);
  var comma = stop.indexOf(",");
  if (comma > 2) {
    return stop.substring(0, 2) + "." + stop.substring(comma+1);
  } else {
    return stop;
  }
}

function opendataTransportSendToPebble(json) {
  var stops = shorten(json.station.name) + "\n";
  var times = number + "\n";
  var now = Date.now();
  // console.log('now: ' + now);

  for (var i = 0; i < json.stationboard.length; i++) {
    // console.log('timestamp: ' + json.stationboard[i].stop.departureTimestamp);
    var sec = json.stationboard[i].stop.departureTimestamp * 1000 - now;
    // console.log('sec: ' + sec);
    var min = sec / 60000 | 0;
    // console.log('min: ' + min);
    if (min < 100) {
      times = times + min + "\n";
      stops = stops + json.stationboard[i].number + " " + shorten(json.stationboard[i].to) + "\n";
    }
  }

  console.log('PeblinAppMessageKeyStops: ' + stops);
  console.log('PeblinAppMessageKeyTimes: ' + times);

  Pebble.sendAppMessage({
    'PeblinAppMessageKeyReply': 1,
    'PeblinAppMessageKeyStops': stops + "\n",
    'PeblinAppMessageKeyTimes': times + "\n"
  });
}

function opendataTransportLocationSuccess(pos) {

  var url_locations = 'http://transport.opendata.ch/v1/locations?x=' +
    pos.coords.latitude + '&y=' + pos.coords.longitude + '&limit=' + number;

  console.log('opendata-transport: Location success. Contacting transport.opendata.ch: \n' + url_locations);

  opendataTransportXHR(url_locations, 'GET', function(responseTextLocation) {
    // console.log('opendata-transport: Got API response for location: \n' + responseTextLocation);
    if(responseTextLocation.length > 100) {
      var station_id = JSON.parse(responseTextLocation).stations[number-1].id; //'008590063' for 'Bern, Elfenau';
      var url_stationboard = 'http://transport.opendata.ch/v1/stationboard?station=' + station_id + '&limit=10';

      opendataTransportXHR(url_stationboard, 'GET', function(responseTextStationboard) {
        // console.log('opendata-transport: Got API response for stationboard: \n' + responseTextStationboard);
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
  // console.log(dict.type);
  number = dict.payload.PeblinAppMessageKeyStops;
  console.log('opendata-transport: Got fetch request from C app with number: ' + number);

  navigator.geolocation.getCurrentPosition(opendataTransportLocationSuccess, opendataTransportLocationError, {
    timeout: 15000,
    maximumAge: 10000
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
