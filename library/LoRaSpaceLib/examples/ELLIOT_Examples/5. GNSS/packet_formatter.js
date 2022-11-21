function decodeUplink(input) {
  if (input.bytes.length == 17) {
    var gnss_unix_time = (input.bytes[0]) | (input.bytes[1] << 8) | (input.bytes[2] << 16) | (input.bytes[3] << 24);
    var gnss_latitude = (input.bytes[4]) | (input.bytes[5] << 16) | (input.bytes[6] << 24) | (input.bytes[7] << 8);
    var gnss_longitude = (input.bytes[8]) | (input.bytes[9] << 8) | (input.bytes[10] << 16) | (input.bytes[11] << 24);
    var gnss_altitude = (input.bytes[12]) | (input.bytes[13] << 8) | (input.bytes[14] << 16) | (input.bytes[15] << 24);
    var gnss_num_sat = input.bytes[16];

    return {
      data: {
        gnss_unix_time: gnss_unix_time,
        gnss_latitude: gnss_latitude,
        gnss_longitude: gnss_longitude,
        gnss_altitude: gnss_altitude,
        gnss_num_sat: gnss_num_sat,
        bytes: input.bytes
      },
      warnings: [],
      errors: []
    };
  }
  else {
    return {
      data: {
        len: input.bytes.length,
        bytes: input.bytes
      },
      warnings: [],
      errors: ["Wrong packet length"]
    };
  }
}