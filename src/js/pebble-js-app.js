/*************************** Weather library start ****************************/

function owmWeatherXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}

function owmWeatherSendToPebble(json) {
  console.log('OWMWeatherAppMessageKeyStop: ' + json.stationboard[0].stop.station.name);
  console.log('OWMWeatherAppMessageKeyLine: ' + json.stationboard[0].name);
  console.log('OWMWeatherAppMessageKeyDestination: ' + json.stationboard[0].to);
  console.log('OWMWeatherAppMessageKeyDeparture: ' + json.stationboard[0].stop.departure);

  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyReply': 1,
    'OWMWeatherAppMessageKeyStop': json.stationboard[0].stop.station.name,
    'OWMWeatherAppMessageKeyLine': json.stationboard[0].name,
    'OWMWeatherAppMessageKeyDestination': json.stationboard[0].to,
    'OWMWeatherAppMessageKeyDeparture': json.stationboard[0].stop.departure
  });
}

function owmWeatherLocationSuccess(pos) {
  var url = 'http://transport.opendata.ch/v1/stationboard?station=elfenau&limit=1';

  // get location, for later
  // var url = 'http://transport.opendata.ch/v1/location?x=' +
  //   pos.coords.latitude + '&y=' + pos.coords.longitude + '&limit=1';

  // var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
  //   pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + owmWeatherAPIKey;
  console.log('owm-weather: Location success. Contacting OpenWeatherMap.org: \n' + url);

  owmWeatherXHR(url, 'GET', function(responseText) {
    console.log('owm-weather: Got API response: \n' + responseText);
    if(responseText.length > 100) {
      owmWeatherSendToPebble(JSON.parse(responseText));
    } else {
      console.log('owm-weather: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'OWMWeatherAppMessageKeyBadKey': 1
      });
    }
  });
}

function owmWeatherLocationError(err) {
  console.log('owm-weather: Location error');
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyLocationUnavailable': 1
  });
}

function owmWeatherHandler(dict) {
  console.log('owm-weather: Got fetch request from C app');

  navigator.geolocation.getCurrentPosition(owmWeatherLocationSuccess, owmWeatherLocationError, {
    timeout: 15000,
    maximumAge: 60000
  });
}

/**************************** Weather library end *****************************/

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('appmessage: ' + JSON.stringify(e.payload));
  owmWeatherHandler(e);
});
