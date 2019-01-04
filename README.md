# LINE_Simple_Beacon_ESP32
Arduino code for LINE Simple Beacon work with ESP32

## Prerequisites
* [Create a channel on the LINE Developers console](https://developers.line.me/en/docs/line-login/getting-started/)
* [Install Arduino IDE](https://www.arduino.cc/en/main/software)

## Setup
* Install ESP32s Boards Manager
    * Preferences > Additional Boards Manager URLs > add
    ```
    'https://dl.espressif.com/dl/package_esp32_index.json'
    ```
    * Tools > Boards Manager > search 'esp32' > Install
* [Issue LINE Simple Beacon Hardware ID](https://admin-official.line.me/beacon/register)
<img src="https://user-images.githubusercontent.com/30001185/50584877-afe5a900-0ea4-11e9-9130-69c3c893a301.png" />
<img src="https://user-images.githubusercontent.com/30001185/50584907-e3283800-0ea4-11e9-8f3f-6645e1797785.png" />
<img src="https://user-images.githubusercontent.com/30001185/50584909-e7545580-0ea4-11e9-97f2-063cfb1bfd8d.png" />

## Steps
* Update Hardware ID
* [Adjust the signal length](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/bluetooth/controller_vhci.html?highlight=esp_ble_tx_power_set#_CPPv220esp_ble_tx_power_set20esp_ble_power_type_t17esp_power_level_t)
```
esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, adjust the power level that you want);
```
* Power level
    * -12dbm ~ 5-10m
    * -9dbm ~ 10-12m
    * -6dbm ~ 13-18m
    * -3dbm ~ 30-40m
    * 0dbm ~ 50m
    * 3dbm ~ 80m
* Upload the code to hardware

## Reference
* https://github.com/godda/LINE_Simple_Beacon_ESP32
* https://medium.com/@chawanwitpoolsri/line-beacon-from-esp32-node32-lite-72c0fc7dc646
* https://engineering.linecorp.com/ja/blog/detail/149/
* https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/bluetooth/index.html
