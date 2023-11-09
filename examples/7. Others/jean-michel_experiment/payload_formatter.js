function decodeUplink(input) {
  if (input.bytes.length == 23) {
    var packet_type = (input.bytes[0] === 0) ? "Terrestrial" : "Satellite";
    
    var temperature_c = input.bytes[1];
    temperature_c |= input.bytes[2] << 8;
    temperature_c /= 100.0;
    
    var gnss_latitude = input.bytes[3];
    gnss_latitude |= input.bytes[4] << 8;
    gnss_latitude |= input.bytes[5] << 16;
    gnss_latitude |= input.bytes[6] << 24;
    gnss_latitude /= 1.0e7;
    
    var gnss_longitude = input.bytes[7];
    gnss_longitude |= input.bytes[8] << 8;
    gnss_longitude |= input.bytes[9] << 16;
    gnss_longitude |= input.bytes[10] << 24;
    gnss_longitude /= 1.0e7;
    
    var gnss_altitude = input.bytes[11];
    gnss_altitude |= input.bytes[12] << 8;
    gnss_altitude |= input.bytes[13] << 16;
    gnss_altitude |= input.bytes[14] << 24;
    gnss_altitude /= 1.0e3;
    
    var accX = input.bytes[15];
    accX = accX * 2000.0 / 127.0;
    var accY = input.bytes[16];
    accY = accY * 2000.0 / 127.0;
    var accZ = input.bytes[17];
    accZ = accZ * 2000.0 / 127.0;
    
    var battery_voltage = input.bytes[18];
    battery_voltage |= (input.bytes[19] << 8);
    battery_voltage = (battery_voltage / 4095.0) * 4.5;
    
    var gnss_siv = input.bytes[20];
    var tle_age_days = input.bytes[21];
    var gnss_fix_time = input.bytes[22];
    
    return {
      data: {
        packet_type: packet_type,
        temperature_c: temperature_c,
        gnss_latitude: gnss_latitude,
        gnss_longitude: gnss_longitude,
        gnss_altitude: gnss_altitude,
        accX: accX,
        accY: accY,
        accZ: accZ,
        gnss_siv: gnss_siv,
        tle_age_days: tle_age_days,
        gnss_fix_time: gnss_fix_time,
        battery_voltage: battery_voltage,
        raw: input.bytes
      },
      warnings: [],
      errors: []
    };
  } else {
    return {
      data: {
        raw: input.bytes
      },
      warnings: ["Unrecognized payload"],
      errors: []
    };
  }
}