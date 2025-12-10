# Weather Station

### This project is realated about one way to store in a pouch.db database from different devices.

Express Backend receives and stores data sent via wifi from different devices created with ESP8266 module, DHT22 sensor and 1602 LCD screen with I2C.
React Frontend shows the information received from these devices.

If new device is connected, it will be added to the frontend automatically.
If a device is disconneted, it is removed from the frontend automatically.

Expected to add more functionalities:
    data statistics by date
    activate one device (air aconditioning) if the temperature reach a value and deactivate if below that value
    activate one device (dehumidifier) if the humidity reach a value and deactivate if below that value
