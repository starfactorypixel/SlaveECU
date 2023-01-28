#include <Arduino.h>
#include <L2Wrapper.h>
#include <BMSReader.h>

// Не получилось убрать HardwareSerial внутрь класса.
HardwareSerial BMSSerial(PB11, PB10);
BMSReader BMS(BMSSerial);

L2Wrapper L2;

void OnBMSData(BMSPacket::params_t params);

void setup()
{
    Serial2.begin(115200, SERIAL_8N1);

    BMS.Init(OnBMSData);
    
    return;
}

void loop()
{
    uint32_t time = millis();
    
    BMS.Processing(time);
    
    return;
}

void OnBMSData(BMSPacket::params_t params)
{
    Serial.print("Total volt: ");
    Serial.print(params.voltage);
    Serial.println("V;");
    
    return;
}
