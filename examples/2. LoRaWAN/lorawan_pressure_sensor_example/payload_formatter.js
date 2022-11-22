function decodeUplink(input) {
  if (input.bytes.length == 12) {
    var pressure = (input.bytes[0]) | (input.bytes[1] << 8) | (input.bytes[2] << 16) | (input.bytes[3] << 24);
    pressure /= 1000;
    
    var altitude = (input.bytes[4]) | (input.bytes[5] << 8) | (input.bytes[6] << 16) | (input.bytes[7] << 24);
    altitude /= 1000;
    
    var temperature = (input.bytes[8]) | (input.bytes[9] << 8) | (input.bytes[10] << 16) | (input.bytes[11] << 24);
    temperature /= 1000;
    
    return {
      data: {
        Pressure: pressure,
        Altitude: altitude,
        Temperature: temperature,
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