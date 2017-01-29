/*
   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  Serial.begin(9600);		// Initialize serial communications with the PC
  while (!Serial);	  	// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();		    	// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Arduino Amiibo writer. Put your NFC-tag to the reader"));
}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // hexdump -v -e " 4/1 \"0x%02X, \" \"\n\"" Chibi-Robo.encoded.bin
  byte pages          = 135;
  byte dataBlock[]    = {

  //    >>> Paste your dump here <<<

  };

  MFRC522::StatusCode status;

  // Write main data
  for (byte page = 3; page < pages; page++) {
    // Write data to the page
    Serial.print(F("Writing data into page ")); Serial.print(page);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock + (page * 4), 4); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(page, dataBlock + (page * 4), 4);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();
  }

  // Write lock bytes - the last thing you should do.
  // If you write them too early - your tag is wasted.
  // Write the Dynamic Lock Bytes
  byte DynamicLockBlock[]    = {
    0x01, 0x00, 0x0F, 0xBD
  };
  Serial.print(F("Writing Dynamic Lock Bytes into page 130"));
  Serial.println(F(" ..."));
  dump_byte_array(DynamicLockBlock, 4); Serial.println();
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(130, DynamicLockBlock, 4);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

  // Now we can write Static Lock Bytes
  byte StaticLockBlock[]    = {
    0x0F, 0xE0, 0x0F, 0xE0
  };
  Serial.print(F("Writing Static Lock Bytes into page 2"));
  Serial.println(F(" ..."));
  dump_byte_array(StaticLockBlock, 4); Serial.println();
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(2, StaticLockBlock, 4);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

  Serial.println("Write finished! Now please take your Amiibo card away!");
  delay(30000);
}

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
