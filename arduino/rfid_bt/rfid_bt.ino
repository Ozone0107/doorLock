#include <SPI.h>
#include <MFRC522.h>
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;


#define MISO_PIN        12
#define MOSI_PIN        13
#define SCK_PIN         14
#define SS_PIN          15
#define RST_PIN         16

String uidStr;


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
	delay(2000);
	Serial.begin(115200);		// Initialize serial communications with the PC
	//while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)

	// void SPIClass::begin(sck, miso, mosi, ss)
	SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(40);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  SerialBT.begin("ESP32"); // 這是藍芽裝置名稱
  Serial.println("Bluetooth started. Pair with 'ESP32_RFID'");
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}


  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i) uidStr += ":";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  // 同步到 Serial Monitor
  Serial.println("UID: " + uidStr);
  // **透過藍牙送到電腦**
  SerialBT.println(uidStr);

  mfrc522.PICC_HaltA();
  delay(1000);
}