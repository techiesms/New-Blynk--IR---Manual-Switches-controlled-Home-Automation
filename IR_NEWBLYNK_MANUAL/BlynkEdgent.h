
extern "C" {
  void app_loop();
  void eraseMcuConfig();
  void restartMCU();
}

#include "Settings.h"
#include <BlynkSimpleEsp32_SSL.h>

#ifndef BLYNK_NEW_LIBRARY
#error "Old version of Blynk library is in use. Please replace it with the new one."
#endif

#if !defined(BLYNK_TEMPLATE_ID) || !defined(BLYNK_DEVICE_NAME)
#error "Please specify your BLYNK_TEMPLATE_ID and BLYNK_DEVICE_NAME"
#endif

#include "BlynkState.h"
#include "ConfigStore.h"
#include "ResetButton.h"
#include "ConfigMode.h"
#include "Indicator.h"
#include "OTA.h"
void irsense();
void swfeedback();
inline
void BlynkState::set(State m) {
  if (state != m && m < MODE_MAX_VALUE) {
    DEBUG_PRINT(String(StateStr[state]) + " => " + StateStr[m]);
    state = m;

    // You can put your state handling here,
    // i.e. implement custom indication
  }
}

void printDeviceBanner()
{
  Blynk.printBanner();
  DEBUG_PRINT("--------------------------");
  DEBUG_PRINT(String("Product:  ") + BLYNK_DEVICE_NAME);
  DEBUG_PRINT(String("Hardware: ") + BOARD_HARDWARE_VERSION);
  DEBUG_PRINT(String("Firmware: ") + BLYNK_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  if (configStore.getFlag(CONFIG_FLAG_VALID)) {
    DEBUG_PRINT(String("Token:    ...") + (configStore.cloudToken + 28));
  }
  DEBUG_PRINT(String("Device:   ") + BLYNK_INFO_DEVICE + " @ " + ESP.getCpuFreqMHz() + "MHz");
  DEBUG_PRINT(String("MAC:      ") + WiFi.macAddress());
  DEBUG_PRINT(String("Flash:    ") + ESP.getFlashChipSize() / 1024 + "K");
  DEBUG_PRINT(String("ESP sdk:  ") + ESP.getSdkVersion());
  DEBUG_PRINT(String("Chip rev: ") + ESP.getChipRevision());
  DEBUG_PRINT(String("Free mem: ") + ESP.getFreeHeap());
  DEBUG_PRINT("--------------------------");
}

void runBlynkWithChecks() {
  Blynk.run();
  if (BlynkState::get() == MODE_RUNNING) {
    if (!Blynk.connected()) {
      if (WiFi.status() == WL_CONNECTED) {
        BlynkState::set(MODE_CONNECTING_CLOUD);
      } else {
        BlynkState::set(MODE_CONNECTING_NET);
      }
    }
  }
}

class Edgent {

  public:
    void begin()
    {
      indicator_init();
      button_init();
      config_init();

      WiFi.persistent(false);
      WiFi.enableSTA(true);   // Needed to get MAC

      printDeviceBanner();

      if (configStore.getFlag(CONFIG_FLAG_VALID)) {
        BlynkState::set(MODE_CONNECTING_NET);
      } else if (config_load_blnkopt()) {
        DEBUG_PRINT("Firmware is preprovisioned");
        BlynkState::set(MODE_CONNECTING_NET);
      } else {
        BlynkState::set(MODE_WAIT_CONFIG);
      }
    }

    void run() {
      app_loop();
      switch (BlynkState::get()) {
        case MODE_WAIT_CONFIG:
        case MODE_CONFIGURING:       enterConfigMode();    break;
        case MODE_CONNECTING_NET:    enterConnectNet();    break;
        case MODE_CONNECTING_CLOUD:  enterConnectCloud();  break;
        case MODE_RUNNING:           runBlynkWithChecks(); break;
        case MODE_OTA_UPGRADE:       enterOTA();           break;
        case MODE_SWITCH_TO_STA:     enterSwitchToSTA();   break;
        case MODE_RESET_CONFIG:      enterResetConfig();   break;
        default:                     enterError();         break;
      }
    }

};

Edgent BlynkEdgent;
BlynkTimer B_timer;

void app_loop() {
  B_timer.run();
  irsense();
  swfeedback();
}





void irsense()
{
  if (irrecv.decode(&results)) {
    //Serial.print("Result - "); Serial.println(results.value);
    if ( results.value == REMOTE_LIGHT1_DATA)
    {
      relay1 = !relay1;
      digitalWrite(r1, relay1);
      Blynk.virtualWrite(V1, relay1);
      Serial.print("r1 - "); Serial.println(relay1);
      delay(100);
    }

    if ( results.value == REMOTE_LIGHT2_DATA)
    {
      relay2 = !relay2;
      digitalWrite(r2, relay2);
      Blynk.virtualWrite(V2, relay2);
      Serial.print("r2 - "); Serial.println(relay2);
      delay(100);
    }

    if ( results.value == REMOTE_LIGHT3_DATA)
    {
      relay3 = !relay3;
      digitalWrite(r3, relay3);
      Blynk.virtualWrite(V3, relay3);
      Serial.print("r3 - "); Serial.println(relay3);
      delay(100);
    }

    if ( results.value == REMOTE_LIGHT4_DATA)
    {
      relay4 = !relay4;
      digitalWrite(r4, relay4);
      Blynk.virtualWrite(V4, relay4);
      Serial.print("r4 - "); Serial.println(relay4);
      delay(100);
    }

    irrecv.resume();
  }
}


void swfeedback()
{
  if (digitalRead(s1) == HIGH && SWITCH5_FLAG == 1) {
    digitalWrite(r1, LOW);
    Blynk.virtualWrite(V1, LOW);
    Serial.println("r1 on");
    relay1 = 0;
    SWITCH5_FLAG = 0;
  }
  if (digitalRead(s1) == LOW && SWITCH5_FLAG == 0) {
    digitalWrite(r1, HIGH);
    Blynk.virtualWrite(V1, HIGH);
    Serial.println("r1 off");
    relay1 = 1;
    SWITCH5_FLAG = 1;
  }
  if (digitalRead(s2) == HIGH && SWITCH6_FLAG == 1) {
    digitalWrite(r2, LOW);
    Blynk.virtualWrite(V2, LOW);
    Serial.println("r2 on");
    relay2 = 0;
    SWITCH6_FLAG = 0;
  }
  if (digitalRead(s2) == LOW && SWITCH6_FLAG == 0) {
    digitalWrite(r2, HIGH);
    Blynk.virtualWrite(V2, HIGH);
    Serial.println("r2 off");
    relay2 = 1;
    SWITCH6_FLAG = 1;
  }
  if (digitalRead(s3) == HIGH && SWITCH7_FLAG == 1) {
    digitalWrite(r3, LOW);
    Blynk.virtualWrite(V3, LOW);
    Serial.println("r3 on");
    relay3 = 0;
    SWITCH7_FLAG = 0;
  }
  if (digitalRead(s3) == LOW && SWITCH7_FLAG == 0) {
    digitalWrite(r3, HIGH);
    Blynk.virtualWrite(V3, HIGH);
    Serial.println("r3 off");
    relay3 = 1;
    SWITCH7_FLAG = 1;
  }
  if (digitalRead(s4) == HIGH && SWITCH8_FLAG == 1) {
    digitalWrite(r4, LOW);
    Blynk.virtualWrite(V4, LOW);
    Serial.println("r4 on");
    relay4 = 0;
    SWITCH8_FLAG = 0;
  }
  if (digitalRead(s4) == LOW && SWITCH8_FLAG == 0) {
    digitalWrite(r4, HIGH);
    Blynk.virtualWrite(V4, HIGH);
    Serial.println("r4 off");
    relay4 = 1;
    SWITCH8_FLAG = 1;
  }
}
