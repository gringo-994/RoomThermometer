# RoomThermometer

Reads temperature values from sensor and public data on a BLE service

Device: CC2640r2f - ti-rtos

Tempereature sensor: linear temperature sensor LM35DZ 

the AdcBuffer is write from ADC through DMA, when the buffer is full the data is sent on main thread, that public data on a BLE service
