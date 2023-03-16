function decodeUplink(input) {
  var packet_type = (input.bytes[0] === 1) ? 'satellite' : 'terrestrial';
  
  var send_epoch = (input.bytes[1] << 0) | (input.bytes[2] << 8) | (input.bytes[3] << 16) | (input.bytes[4] << 24);
  
  var bat_voltage_adc = (input.bytes[5] << 0) | (input.bytes[6] << 8);
  var battery_vol = bat_voltage_adc * 4.5 / 4095.0;
  
  var tle_age = (input.bytes[7] << 0) | (input.bytes[8] << 8) | (input.bytes[9] << 16);

  var next_pass_epoch = (input.bytes[10] << 0) | (input.bytes[11] << 8) | (input.bytes[12] << 16);
  next_pass_epoch += send_epoch;
  
  var next_gnss_update_epoch = (input.bytes[13] << 0) | (input.bytes[14] << 8);
  next_gnss_update_epoch += send_epoch;
  
  var last_gnss_fix_time = (input.bytes[15] << 0) | (input.bytes[16] << 8);
  
  var gnss_latitude = (input.bytes[17] << 0) | (input.bytes[18] << 8) | (input.bytes[19] << 16) | (input.bytes[20] << 24);
  gnss_latitude /= 1e7;
  var gnss_longitude = (input.bytes[21] << 0) | (input.bytes[22] << 8) | (input.bytes[23] << 16) | (input.bytes[24] << 24);
  gnss_longitude /= 1e7;

  if (input.bytes.length == 25) {
    return {
      data: {
        packet_type: packet_type,
        send_epoch: send_epoch,
        battery_vol: battery_vol,
        tle_age: tle_age,
        next_pass_epoch: next_pass_epoch,
        next_gnss_update_epoch: next_gnss_update_epoch,
        last_gnss_fix_time: last_gnss_fix_time,
        gnss_latitude: gnss_latitude,
        gnss_longitude: gnss_longitude,
        raw_data: input.bytes
      },
      warnings: [],
      errors: []
    };
  }
  else {
    return {
      data: {
        raw_data: input.bytes
      },
      warnings: ["Unrecognized payload"],
      errors: []
    };
  }
}