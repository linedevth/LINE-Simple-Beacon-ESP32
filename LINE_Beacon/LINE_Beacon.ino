#include "bt.h"
#include "esp_gap_ble_api.h"

static const uint8_t HWID[5] = {0xff, 0xff, 0xff, 0xff, 0xff};// ผูก Hardware ID
static const uint8_t ADC_PIN = 35;  // 電圧入力ピン
static const float R1 = 4000.0;
static const float R2 = 2000.0;

static const uint16_t UUID_FOR_LINECORP = 0xFE6F;
static const uint8_t  MAX_SIMPLEBEACON_DEVICEMESSAGE_SIZE = 13;

static uint8_t deviceMessageSize = 1;
static uint8_t deviceMessage[MAX_SIMPLEBEACON_DEVICEMESSAGE_SIZE];

static const uint8_t  MAX_BLE_ADVERTISING_DATA_SIZE = 31;
static const uint16_t HCI_LE_Set_Advertising_Data   = (0x08 << 10) | 0x0008;
static const uint16_t HCI_LE_Set_Advertising_Enable = (0x08 << 10) | 0x000A;


static void _dump(const char * title, uint8_t *data, size_t dataSize) {
  Serial.printf("%s [%d]:", title, dataSize);
  for (size_t i = 0; i < dataSize; i++) {
    Serial.printf(" %02x", data[i]);
  }
  Serial.println();
}

static void putUint8(uint8_t** bufferPtr, uint8_t data) {
  *(*bufferPtr)++ = data;
}

static void putUint16LE(uint8_t** bufferPtr, uint16_t data) {
  *(*bufferPtr)++ = lowByte(data);
  *(*bufferPtr)++ = highByte(data);
}

static void putArray(uint8_t** bufferPtr, const void* data, size_t dataSize) {
  memcpy(*bufferPtr, data, dataSize);
  (*bufferPtr) += dataSize;
}

// HCIを使い、指定されたopCode、dataのコマンドをBluetooth Controllerに書き込みます
static void executeBluetoothHCICommand(uint16_t opCode, const uint8_t * hciData, uint8_t hciDataSize) {
  uint8_t buf[5 + MAX_BLE_ADVERTISING_DATA_SIZE]; //uint8_t buf[4 + MAX_BLE_ADVERTISING_DATA_SIZE]; แก้ไขเพื่อแก้เรื่อง fix for Stack overflow
  uint8_t* bufPtr = buf;

  putUint8(&bufPtr, 1);// 1 = H4_TYPE_COMMAND
  putUint16LE(&bufPtr, opCode);
  putUint8(&bufPtr, hciDataSize);
  putArray(&bufPtr, hciData, hciDataSize);

  uint8_t bufSize = bufPtr - buf;

  while (!esp_vhci_host_check_send_available());
  esp_vhci_host_send_packet(buf, bufSize);
}

static void updateSimpleBeaconDeviceMessage(char dm[], int dmsize) {
  memset(deviceMessage, 0x00, MAX_SIMPLEBEACON_DEVICEMESSAGE_SIZE);
  uint8_t* deviceMessagePtr = deviceMessage;



  const uint8_t voltageStringSize = dmsize - 1;
  char voltageString[voltageStringSize];


  //snprintf(voltageString, voltageStringSize, "V=%f", dm);
  //putArray(&deviceMessagePtr, voltageString, voltageStringSize);
  putArray(&deviceMessagePtr, dm, voltageStringSize);
  //deviceMessageSize = deviceMessagePtr - deviceMessage;
  deviceMessageSize = 13;

}


static void updateAdvertisingData() {

  uint8_t data[MAX_BLE_ADVERTISING_DATA_SIZE];
  uint8_t* dataPtr = data;

  {
    // flag
    putUint8(&dataPtr, 1 + 1);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_FLAG);
    putUint8(&dataPtr, ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
  }
  {
    // complete list of service uuid
    putUint8(&dataPtr, 1 + 2);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_16SRV_CMPL);
    putUint16LE(&dataPtr, UUID_FOR_LINECORP);
  }
  {
    // simple beacon
    putUint8(&dataPtr, 1 + 9 + deviceMessageSize);
    putUint8(&dataPtr, ESP_BLE_AD_TYPE_SERVICE_DATA);
    putUint16LE(&dataPtr, UUID_FOR_LINECORP);
    putUint8(&dataPtr, 0x02);    // frame type
    putArray(&dataPtr, HWID, 5);
    putUint8(&dataPtr, 0x7F);    // measured txpower
    putArray(&dataPtr, deviceMessage, deviceMessageSize);
  }

  uint8_t dataSize = dataPtr - data;
  _dump("simple beacon", data, dataSize);

  uint8_t hciDataSize = 1 + MAX_BLE_ADVERTISING_DATA_SIZE;
  uint8_t hciData[hciDataSize];

  hciData[0] = dataSize;
  memcpy(hciData + 1, data, dataSize);

  executeBluetoothHCICommand(HCI_LE_Set_Advertising_Data, hciData, hciDataSize);
}

static void enableBluetoothAdvertising() {
  uint8_t enable = 1;
  executeBluetoothHCICommand(HCI_LE_Set_Advertising_Enable, &enable, 1);
}



static void notifyHostSendAvailableHandler() {
  //  _dump("notify_host_send_available", NULL, 0);
}
static int notifyHostRecvHandler(uint8_t *data, uint16_t len) {
  //  _dump("notify_host_recv", data, len);
  return 0;
}

static esp_vhci_host_callback_t vhciHostCallback = {
  notifyHostSendAvailableHandler,
  notifyHostRecvHandler
};


void setup() {
  Serial.begin(115200, SERIAL_8N1);

  // 互換性を考慮し、ESP32のデフォルト値で上書きする
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  // bluetoothを初期化
  btStart();
  esp_vhci_host_register_callback(&vhciHostCallback);
  enableBluetoothAdvertising();
}

void loop() {

  // 12bit幅（0...4095）で電圧を取得。4095が最大電圧(ADC_11dbなので3.9V)を表します
  uint16_t analog12bit = analogRead(ADC_PIN);

  // 分圧前の電源電圧を計算
  float voltage = analog12bit / 4095.0 * 3.9 * (R1 + R2) / R2;

  Serial.printf("analogRead: %d,  voltage: %f", analog12bit, voltage);
  Serial.println();

  char dm[] = "Hello World";
  int dm_size = sizeof(dm) / sizeof(dm[0]);;
  Serial.println(dm_size);
  updateSimpleBeaconDeviceMessage(dm, dm_size);
  //updateSimpleBeaconDeviceMessage(voltage);
  updateAdvertisingData();

  delay(5000);
}
