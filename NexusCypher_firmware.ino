/*
 * Nexus Cypher -- LoRa RF Dev Board firmware
 * RP2040 + SX1262, built with the Arduino-Pico core and RadioLib
 * Project: https://github.com/WIZTHECYBER/LoRa-RF-PCB
 *
 * AI assistance: this firmware was written with help from Claude
 * (Anthropic). Edit this line to describe your own contribution.
 *
 * WHAT IT DOES
 *   Point-to-point LoRa chat between two Nexus Cypher boards.
 *   Type a line in the USB Serial Monitor, press Enter, and it is sent
 *   over the air. Messages from the other board are printed with RSSI
 *   and SNR, which doubles as a range-test tool.
 *   No LoRaWAN, gateway or network credentials needed.
 *
 * OPTIONAL FEATURES (switch on in the settings below)
 *   USE_ENCRYPTION      ChaCha20-Poly1305 encrypted + authenticated
 *                       messages with replay protection.
 *   BEACON_INTERVAL_MS  Auto-send a test packet every N ms. Handy for
 *                       checking a single board with an SDR receiver.
 *
 * SETUP
 *   1. Arduino IDE -> add the arduino-pico core (Earle Philhower).
 *   2. Library Manager -> install "RadioLib".
 *      (Only for USE_ENCRYPTION 1: also install "Crypto" by Rhys
 *      Weatherley and create a secrets.h next to this file holding
 *      the 32-byte key:  static const uint8_t SECRET_KEY[32] = {...};
 *      Never commit secrets.h to GitHub.)
 *   3. Board: "Raspberry Pi Pico" (or Generic RP2040).
 *   4. Put this file in a folder named NexusCypher_firmware.
 *   5. Set NODE_NAME ("A" on one board, "B" on the other) and flash both.
 *   6. Serial Monitor: 115200 baud, line ending "Newline".
 *
 * ATTACH THE ANTENNA (U.FL) BEFORE POWERING ON.
 *
 * PIN MAP (from the board schematic)
 *   SX1262 SPI0: SCK=GPIO2  MOSI=GPIO3  MISO=GPIO4  NSS=GPIO5
 *   SX1262 BUSY=GPIO6  RESET=GPIO7  DIO1=GPIO8
 *   DIO2 drives the PE4259 RF switch directly (handled by the radio)
 *   Status LED = GPIO25    OLED header J3 (I2C0): SDA=GPIO0  SCL=GPIO1
 *   BOOT / RESET buttons are hardware-only (bootsel / RUN), not readable.
 */

#include <RadioLib.h>

// ---- Optional features ----
#define USE_ENCRYPTION      0     // 1 = encrypt messages (needs secrets.h)
#define BEACON_INTERVAL_MS  0     // 0 = off, e.g. 5000 = beacon every 5 s

#if USE_ENCRYPTION
  #include <EEPROM.h>
  #include <Crypto.h>
  #include <ChaChaPoly.h>
  #include "secrets.h"
#endif

// ---- Who am I? Use "B" on the second board ----
#define NODE_NAME "A"

// ---- Pin definitions ----
#define PIN_NSS    5
#define PIN_DIO1   8
#define PIN_RESET  7
#define PIN_BUSY   6
#define PIN_LED    25

// ---- Radio settings (MUST be identical on both boards) ----
#define LORA_FREQUENCY          915.0  // MHz. Use your region's legal ISM band
                                        // (868.0 for EU). The board's filter is
                                        // centered near 900 MHz.
#define LORA_BANDWIDTH           125.0 // kHz
#define LORA_SPREADING_FACTOR    10    // 7 = fast/short range, 12 = slow/long range
#define LORA_CODING_RATE         7     // 4/7
#define LORA_SYNC_WORD           0x12  // private network (0x34 = LoRaWAN public)
#define LORA_TX_POWER            14    // dBm, chip max is 22. Check local limits.
#define LORA_PREAMBLE_LEN        8

#define MAX_MSG_LEN              200   // characters typed per message
#define MAX_PACKET               255   // LoRa payload limit in bytes

SX1262 radio = new Module(PIN_NSS, PIN_DIO1, PIN_RESET, PIN_BUSY);

volatile bool operationDone = false;
bool isTransmitting = false;
bool hasPending = false;
String inputBuffer = "";
String pendingMsg = "";
uint8_t txPacket[MAX_PACKET];
uint32_t lastBeacon = 0;
uint32_t beaconCount = 0;

// ---------------------------------------------------------------
// Encryption (only compiled when USE_ENCRYPTION is 1)
// Packet on the air: [12-byte nonce][ciphertext][16-byte tag]
// ---------------------------------------------------------------
#if USE_ENCRYPTION

#define NONCE_LEN 12
#define TAG_LEN   16
#define KEY_LEN   32

#define DEC_OK         0
#define DEC_TOO_SHORT  1
#define DEC_BAD_TAG    2
#define DEC_REPLAY     3

ChaChaPoly cipher;

// A nonce must never repeat with the same key. It is built from this
// node's name, a boot counter kept in flash, a per-boot message
// counter and some timing bits.
uint32_t bootCount = 0;
uint32_t msgCounter = 0;

// Replay protection: highest (boot, counter) seen from each sender.
bool     seenSender[256];
uint32_t lastBoot[256];
uint32_t lastCtr[256];

bool keyIsDefault() {
  for (int i = 0; i < KEY_LEN; i++) {
    if (SECRET_KEY[i] != 0) return false;
  }
  return true;
}

void loadBootCount() {
  EEPROM.begin(16);
  EEPROM.get(0, bootCount);
  if (bootCount == 0xFFFFFFFF) bootCount = 0;  // fresh flash
  bootCount++;
  EEPROM.put(0, bootCount);
  EEPROM.commit();
}

size_t encryptMessage(const String &msg) {
  uint8_t nonce[NONCE_LEN];
  uint32_t t = micros();

  nonce[0] = (uint8_t)NODE_NAME[0];
  memcpy(nonce + 1, &bootCount, 4);
  memcpy(nonce + 5, &msgCounter, 4);
  nonce[9]  = t & 0xFF;
  nonce[10] = (t >> 8) & 0xFF;
  nonce[11] = (t >> 16) & 0xFF;
  msgCounter++;

  size_t len = msg.length();
  memcpy(txPacket, nonce, NONCE_LEN);

  cipher.clear();
  cipher.setKey(SECRET_KEY, KEY_LEN);
  cipher.setIV(nonce, NONCE_LEN);
  cipher.encrypt(txPacket + NONCE_LEN, (const uint8_t *)msg.c_str(), len);
  cipher.computeTag(txPacket + NONCE_LEN + len, TAG_LEN);
  cipher.clear();

  return NONCE_LEN + len + TAG_LEN;
}

int decryptMessage(const uint8_t *in, size_t inLen, String &out) {
  if (inLen < NONCE_LEN + TAG_LEN + 1) return DEC_TOO_SHORT;

  const uint8_t *nonce = in;
  size_t len = inLen - NONCE_LEN - TAG_LEN;
  const uint8_t *ct = in + NONCE_LEN;
  const uint8_t *tag = in + NONCE_LEN + len;

  static uint8_t plain[MAX_PACKET + 1];

  cipher.clear();
  cipher.setKey(SECRET_KEY, KEY_LEN);
  cipher.setIV(nonce, NONCE_LEN);
  cipher.decrypt(plain, ct, len);
  bool ok = cipher.checkTag(tag, TAG_LEN);
  cipher.clear();

  if (!ok) return DEC_BAD_TAG;  // wrong key, or packet was tampered with

  // Authenticated. Now reject anything already seen (replay).
  uint8_t sender = nonce[0];
  uint32_t boot, ctr;
  memcpy(&boot, nonce + 1, 4);
  memcpy(&ctr, nonce + 5, 4);

  if (seenSender[sender]) {
    if (boot < lastBoot[sender] || (boot == lastBoot[sender] && ctr <= lastCtr[sender])) {
      return DEC_REPLAY;
    }
  }
  seenSender[sender] = true;
  lastBoot[sender] = boot;
  lastCtr[sender] = ctr;

  plain[len] = 0;
  out = String((char *)plain);
  return DEC_OK;
}

#endif  // USE_ENCRYPTION

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------
void onRadioInterrupt() {
  operationDone = true;
}

void blinkLed(int times, int ms) {
  for (int i = 0; i < times; i++) {
    digitalWrite(PIN_LED, HIGH);
    delay(ms);
    digitalWrite(PIN_LED, LOW);
    delay(ms);
  }
}

// Turns a text message into the bytes that go on the air.
size_t buildPacket(const String &msg) {
#if USE_ENCRYPTION
  return encryptMessage(msg);
#else
  size_t len = msg.length();
  memcpy(txPacket, msg.c_str(), len);
  return len;
#endif
}

// Reads a received packet and prints it.
void handleIncoming() {
  uint8_t rxBuf[MAX_PACKET];
  size_t len = radio.getPacketLength();
  if (len > sizeof(rxBuf)) len = sizeof(rxBuf);

  int state = radio.readData(rxBuf, len);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("[error] readData failed, code "));
    Serial.println(state);
    return;
  }

  String text;
#if USE_ENCRYPTION
  int r = decryptMessage(rxBuf, len, text);
  if (r == DEC_BAD_TAG) {
    Serial.println(F("[dropped] failed authentication (wrong key or tampered)"));
    return;
  } else if (r == DEC_REPLAY) {
    Serial.println(F("[dropped] replayed packet"));
    return;
  } else if (r != DEC_OK) {
    Serial.println(F("[dropped] packet too short"));
    return;
  }
#else
  for (size_t i = 0; i < len; i++) {
    text += (char)rxBuf[i];
  }
#endif

  Serial.print(F("[recv] "));
  Serial.print(text);
  Serial.print(F("   (RSSI "));
  Serial.print(radio.getRSSI());
  Serial.print(F(" dBm, SNR "));
  Serial.print(radio.getSNR());
  Serial.println(F(" dB)"));
  blinkLed(2, 40);
}

// ---------------------------------------------------------------
// Setup
// ---------------------------------------------------------------
void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  Serial.begin(115200);
  uint32_t serialStart = millis();
  while (!Serial && millis() - serialStart < 3000) {
    // wait briefly for USB serial, but don't hang if nothing is connected
  }

  Serial.println();
  Serial.println(F("=== Nexus Cypher LoRa firmware ==="));

#if USE_ENCRYPTION
  if (keyIsDefault()) {
    while (true) {
      Serial.println(F("[STOP] secrets.h still has the all-zero placeholder key."));
      blinkLed(2, 100);
      delay(2000);
    }
  }
  loadBootCount();
  Serial.println(F("Encryption: ON (ChaCha20-Poly1305)"));
#else
  Serial.println(F("Encryption: OFF (messages are readable over the air)"));
#endif

  Serial.print(F("Node name: "));
  Serial.println(NODE_NAME);
  Serial.print(F("[SX1262] Initializing ... "));

  // tcxoVoltage = 0.0: this board uses a plain crystal, not a TCXO.
  int state = radio.begin(LORA_FREQUENCY, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                           LORA_CODING_RATE, LORA_SYNC_WORD, LORA_TX_POWER,
                           LORA_PREAMBLE_LEN, 0.0, false);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true) {
      blinkLed(1, 100);  // fast blink = radio init failed, check wiring/pins
    }
  }
  Serial.println(F("success!"));

  // The PE4259 RF switch CTRL pin is wired to the radio's DIO2, so let
  // the radio drive it automatically (high on TX, low otherwise).
  radio.setDio2AsRfSwitchCtrl(true);

  radio.setPacketReceivedAction(onRadioInterrupt);
  radio.setPacketSentAction(onRadioInterrupt);
  radio.startReceive();

  blinkLed(3, 150);  // three blinks = ready
  lastBeacon = millis();
  Serial.println(F("Ready. Type a message and press Enter to send."));
  Serial.println();
}

// ---------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------
void loop() {
  // 1) Collect typed characters until Enter
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;

    if (c == '\n') {
      if (inputBuffer.length() > 0) {
        if (isTransmitting || hasPending) {
          Serial.println(F("[busy] still sending, try again in a second"));
        } else {
          pendingMsg = String(NODE_NAME) + ": " + inputBuffer;
          hasPending = true;
        }
        inputBuffer = "";
      }
    } else if (inputBuffer.length() < MAX_MSG_LEN) {
      inputBuffer += c;
    }
  }

  // 2) Optional automatic beacon
#if BEACON_INTERVAL_MS > 0
  if (!hasPending && !isTransmitting && millis() - lastBeacon >= BEACON_INTERVAL_MS) {
    lastBeacon = millis();
    pendingMsg = String(NODE_NAME) + ": beacon #" + String(beaconCount++);
    hasPending = true;
  }
#endif

  // 3) Send the pending message
  if (hasPending && !isTransmitting) {
    hasPending = false;
    operationDone = false;
    isTransmitting = true;

    size_t packetLen = buildPacket(pendingMsg);
    int state = radio.startTransmit(txPacket, packetLen);
    if (state != RADIOLIB_ERR_NONE) {
      Serial.print(F("[error] startTransmit failed, code "));
      Serial.println(state);
      isTransmitting = false;
      radio.startReceive();
    }
  }

  // 4) Handle radio events (TX finished or packet received)
  if (operationDone) {
    operationDone = false;

    if (isTransmitting) {
      radio.finishTransmit();
      isTransmitting = false;
      Serial.print(F("[sent] "));
      Serial.println(pendingMsg);
      blinkLed(1, 40);
    } else {
      handleIncoming();
    }
    radio.startReceive();
  }
}
