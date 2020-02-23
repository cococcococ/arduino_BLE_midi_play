#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <PSG810.h>
PSG810* psg;

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"
#define DEVIVE_NAME "MIDIduino"

#define buttonPin 18 //esp32 =>arduno 13pin
#define LED 2 //Soldered LED[IO2]

int buttonState = 0; // variable for reading the pushbutton status
int scale;
int note_name;

BLEServer *pServer;
BLEAdvertising *pAdvertising;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

int pos = 0;
char midi[5];

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      pos = 0;

      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++)
        {
          midi[pos] = rxValue[i];
          pos++;
          if(pos == 5)
          {
            Serial.print(midi[2],HEX);
            Serial.print(" ");
            Serial.print(midi[3],HEX);
            Serial.print(" ");
            Serial.println(midi[4],HEX);
            Serial.println();
            pos = 0;
            
            switch (midi[2]) {
              case 0x90:
                // Note on message
                note_name = midi[3] % 12;
                scale = (midi[3] / 12) + 1;
//                Serial.print(scale);
//                Serial.print(" ");
//                Serial.println(note_name);
//                Serial.println();
                switch (note_name) {
                  case 0:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'C'));
                    break;
                  case 1:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'C', true));
                    break;
                  case 2:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'D'));
                    break;
                  case 3:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'D', true));
                    break;
                  case 4:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'E'));
                    break;
                  case 5:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'F'));
                    break;
                  case 6:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'F', true));
                    break;
                  case 7:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'G'));
                    break;
                  case 8:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'G', true));
                    break;
                  case 9:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'A'));
                    break;
                  case 10:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'A', true));
                    break;
                  case 11:
                    psg->SetFrequency(PSG::PSG_A, CalcFreqByMML(scale, 'B'));
                    break;
                }
                digitalWrite(LED, HIGH);
                //delay(200);
                break;
              case 0x80:
                // Note off message
                digitalWrite(LED, LOW);
                psg->SetFrequency(PSG::PSG_A, noteFreq[127]);
                //Serial.println("Note off");
                //delay(200);
                break;
            }
          }          
        }
      }
    }
};

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200);

  BLEDevice::init(DEVIVE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));
  pCharacteristic = pService->createCharacteristic(
    BLEUUID(MIDI_CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ   |
    BLECharacteristic::PROPERTY_WRITE  |
    BLECharacteristic::PROPERTY_NOTIFY |
    BLECharacteristic::PROPERTY_WRITE_NR
  );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04);
  oAdvertisementData.setCompleteServices(BLEUUID(MIDI_SERVICE_UUID));
  oAdvertisementData.setName(DEVIVE_NAME);
  pAdvertising = pServer->getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->start();

  psg = new PSG810();
  psg->SetMixer(0x110);
  psg->SetVolume(PSG::PSG_A, 0x0f);
}

void loop() {
  delay(10);
}
