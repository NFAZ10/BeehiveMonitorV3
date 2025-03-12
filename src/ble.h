#ifndef BLE_H
#define BLE_H

#include "BLEDevice.h"

void setupBLE();
void BLEloop();
bool connectToServer();
 // The remote service we wish to connect to.
extern  BLEUUID serviceUUID();
 // The characteristic of the remote service we are interested in.
extern  BLEUUID charUUID();
 
 extern  boolean doConnect;
 extern  boolean connected;
 extern  boolean doScan;
 extern  BLERemoteCharacteristic *pRemoteCharacteristic;
 extern  BLEAdvertisedDevice *myDevice;


#endif
