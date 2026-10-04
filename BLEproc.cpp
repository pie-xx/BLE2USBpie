
#include <NimBLEDevice.h>
static NimBLEUUID hidServiceUUID((uint16_t)0x1812);
static NimBLEUUID reportUUID((uint16_t)0x2A4D);
static NimBLEAdvertisedDevice *keyboardDevice = nullptr;
static NimBLEClient *client = nullptr;

#include "keyboardProc.h"
extern CustomKeyboard Keyboard;

#include "keyDefine.h"
#include "mouseProc.h"
#include "BLEproc.h"

enum ACTMODE actMode;

extern QueueHandle_t bleKeyQueue;

// ============================================================
// BLE Client callbacks
// 接続・切断はこちら
// ============================================================

class ClientCallbacks : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient* pClient) override
    {
        debugPrint("BLE connected");
    }

    void onDisconnect(
        NimBLEClient* pClient,
        int reason
    ) override
    {
      debugPrint(
            String("BLE disconnected. reason = ")+
            String(reason)+" 0x"+
            String(reason, HEX)
        );

        actMode = AM_DISCONNECT_SCAN;

        // 再スキャン開始
        debugPrint("Restart scanning...");
        NimBLEDevice::getScan()->start(0, false);
    }
};

ClientCallbacks clientCallbacks;


// -----------------------------------------------------
// BLE notification
// -----------------------------------------------------

void notifyCallback(
    NimBLERemoteCharacteristic *characteristic,
    uint8_t *data,
    size_t length,
    bool isNotify)
{
  KeyReport report = {};

  if (length > 0) {
      report.modifiers = data[0];
  }
  if (length > 1) {
      report.reserved = data[1];
  }

  for (size_t i = 0; i < 6 && i + 2 < length; i++) {
      report.keys[i] = data[i + 2];
  }

  xQueueSend(bleKeyQueue, &report, 0);

}

// -----------------------------------------------------
// キーボード接続
// -----------------------------------------------------
NimBLEAddress keyboardAddress;
bool keyboardKnown = false;

bool connectKeyboard()
{
    client = NimBLEDevice::createClient();
    client->setClientCallbacks(
        &clientCallbacks,
        false
    );

    if (!client->connect(keyboardDevice)) {
        return false;
    }

    // HID Service
    NimBLERemoteService *hidService =
        client->getService(NimBLEUUID((uint16_t)0x1812));

    if (hidService == nullptr) {
        return false;
    }

    // Characteristic一覧取得
    const auto &characteristics =
        hidService->getCharacteristics(true);

    for (auto *ch : characteristics)
    {
        // HID Report Characteristic
        if (ch->getUUID() ==
            NimBLEUUID((uint16_t)0x2A4D))
        {
            debugPrint("This is HID REPORT");

            // Report Reference Descriptor
            NimBLERemoteDescriptor *desc =
                ch->getDescriptor(
                    NimBLEUUID((uint16_t)0x2908)
                );

            if (desc != nullptr)
            {
                NimBLEAttValue value =
                    desc->readValue();

                String report = "";
                for (size_t i = 0;
                     i < value.size();
                     i++)
                {
                    report += String(value[i], HEX) + " ";
                }
            }

            // Notification登録
            if (ch->canNotify())
            {
                bool ok =
                    ch->subscribe(
                        true,
                        notifyCallback
                    );
                
                if(ok) {
                    debugPrint(
                        "Subscribe SUCCESS"
                    );
                } else {
                    debugPrint(
                        "Subscribe FAILED"
                    );
                }
            }
        }
    }

  keyboardAddress = keyboardDevice->getAddress();
  keyboardKnown = true;

  debugPrint(
      String("Remember keyboard: ")+
      keyboardAddress.toString().c_str()
  );
  actMode = AM_KEY;

  return true;
}

// -----------------------------------------------------
// BLE Scan Callback
// -----------------------------------------------------

class ScanCallbacks : public NimBLEScanCallbacks
{
    void onResult(
        const NimBLEAdvertisedDevice *device
    ) override
    {
        debugPrint(
            String("ADV: ")+
            device->toString().c_str()
        );

        bool found = false;

        // 初回：HID Service UUIDで発見
        if (device->isAdvertisingService(hidServiceUUID))
        {
            debugPrint("HID device found");
            found = true;
        }

        // 2回目以降：以前接続したアドレスでも判定
        if (keyboardKnown &&
            device->getAddress() == keyboardAddress)
        {
            debugPrint("Known keyboard found");
            found = true;
        }

        if (found)
        {
            keyboardDevice =
                new NimBLEAdvertisedDevice(*device);

            NimBLEDevice::getScan()->stop();

            actMode = AM_DO_CONNECT;
        }
    }

};

void BLEprocInit(){
  
    NimBLEDevice::init("");

    NimBLEScan *scan =
        NimBLEDevice::getScan();

    scan->setScanCallbacks(
        new ScanCallbacks()
    );

    scan->setActiveScan(true);

    actMode = AM_BOOT_SCAN;
    debugPrint("scan start");
    scan->start(0, false);


}