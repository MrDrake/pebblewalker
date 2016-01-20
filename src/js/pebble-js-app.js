Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  Pebble.openURL('https://mrdrake.github.io/pebblewalker/config-page.html');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  console.log('FOO: ', e.response);
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));

  // Prepare AppMessage payload
  var dict = {
    'KEY_POKEMON': config_data['pokemon'],
    'KEY_ROUTE': config_data['route']
  };

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
});