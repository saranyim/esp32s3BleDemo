#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#define SERVICE_UUID           "0002EC01-0000-0000-4543-553D53484F50"

#define CHARACTERISTIC_UUID "57524502-5448-002D-4543-553D53484F50"
#define CHARACTERISTIC_UUID_TX "57524501-5448-002D-4543-553D53484F50"

BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristicTx;
BLEService *pService;
BLEServer *pServer;
BLEAdvertising *pAdvertising;
BLEAdvertisementData bleAdsData;
// put function declarations here:

char bleTxBuff[128];

char txBuff[32];
std::string tmpStr;
std::string bleRecvStr;

bool fBLEDataReady;

class MyServerCallbacks: public BLEServerCallbacks {  
	void onConnect(BLEServer* _pServer) {
		Serial.println("Connected");
	};
	void onDisconnect(BLEServer* _pServer) {
		Serial.println("Disconnected");
		pAdvertising->start();
   // vTaskDelete(hTaskBLE);
	}
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
		bleRecvStr += pCharacteristic->getValue();
		if(bleRecvStr.find("\r\n") != -1){
			tmpStr = bleRecvStr;
			bleRecvStr = "";
			fBLEDataReady = true;
		}
      //  Serial.print(bleRecvStr.c_str());
    }
};

void DeinitBLE(){
	
	BLEDevice::deinit();
	

}


void InitBLE(){

	
	BLEDevice::init("strDevName");
	
	pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());
	pService = pServer->createService(SERVICE_UUID);

//-------------------- P RX  -------------------------
	pCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_WRITE |
		BLECharacteristic::PROPERTY_WRITE_NR

	);
	pCharacteristic->addDescriptor(new BLEDescriptor((uint16_t) 0x2901));
	// pCharacteristic->addDescriptor(new BLE2902());
	pCharacteristic->setCallbacks(new MyCallbacks());
	pCharacteristic->setValue("IOTDevRx");


//----------------------------------------------------

//-------------------- P TX  -------------------------
	pCharacteristicTx = pService->createCharacteristic(
		CHARACTERISTIC_UUID_TX,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_NOTIFY  
	);
	pCharacteristicTx->addDescriptor(new BLEDescriptor((uint16_t) 0x2901));
	// pCharacteristicTx->addDescriptor(new BLE2902());
	//pCharacteristicTx->setCallbacks(new MyCallbacks());

	pCharacteristicTx->setValue("IOTDevTx");

//----------------------------------------------------

	

	
	pService->start();

	//bleAdsData.addData("My dog name boo");
	pAdvertising = pServer->getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	//pAdvertising->setAdvertisementData(bleAdsData);
	pAdvertising->start();
	// Serial.println("Characteristic defined! Now you can read it in your phone!");

	
}


void BLETx(){
	uint16_t lenToSend = strlen(bleTxBuff);
	uint16_t loop = 0;
	// Serial.print("Tx : ");
	// Serial.println(bleTxBuff);
	while(lenToSend > 20){
		memcpy(txBuff,bleTxBuff + (loop++ * 20),20);
		pCharacteristicTx->setValue((uint8_t *) txBuff,20);
		pCharacteristicTx->notify();
		lenToSend -= 20;
	}
	memcpy(txBuff,bleTxBuff + (loop++ * 20),lenToSend);
	pCharacteristicTx->setValue((uint8_t *) txBuff,lenToSend);
	pCharacteristicTx->notify();
	//
}

void setup() {
  // put your setup code here, to run once:
  InitBLE();
}

void loop() {
  // put your main code here, to run repeatedly:
}

