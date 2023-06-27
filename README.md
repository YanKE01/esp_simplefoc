# ESP32-IDF SimpleFOC Component
 
Features:
1. Support voltage control.
2. Support Host computer control.

## Usage Considerations
1. The configTICK_RATE_HZ of Freertos must be set to 1000. If it is not set, a motor exception may occur.

## License
esp-simplefoc is licensed under the Apache License, see license.txt for more information.