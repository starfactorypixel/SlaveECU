#include <Arduino.h>
#include <L2Wrapper.h>
#include <Manager.h>
#include <BMSReader.h>

Manager<16> MNG;
BMSReader BMS;
L2Wrapper L2;


/*
ManagerObjInterface params[] = 
{
    ManagerObj<uint32_t>(0x00A0, 1000, 1000, [](uint32_t &newval) -> bool { newval = 82421; return false; }),
    ManagerObj<uint8_t>(0x00AA, 3000, 10000, [](uint8_t &newval) -> bool { newval = 35; return false; })
};
*/


ManagerObj<uint32_t> HiVolt(0x00A0, 1000, 1000, [](uint32_t &newval) -> bool { newval = 82421; return false; });
ManagerObj<uint8_t> MaxTemp(0x00AA, 3000, 10000, [](uint8_t &newval) -> bool { newval = 35; return false; });



void OnBMSData(BMSPacket::params_t &params)
{
    Serial.print("Total volt: ");
    Serial.print(params.voltage);
    Serial.println("V;");
    
    return;
}


// Приняли пакет по CAN. Псевдокод.
void OnL2RX(uint16_t id, uint8_t *data, uint8_t length)
{

    // Определяем по автосгенерированной маске принадлежность пакета к текущему устроиству.
    if( MNG.IsHere(id) == true )
    {
        if(/*Проверяем тип пакета, типа запроса*/ true)
        {
            // Что то делаем..
        }
    }

}


void setup()
{
    Serial2.begin(115200, SERIAL_8N1);


    MNG.RegParam(HiVolt);
    MNG.RegParam(MaxTemp);
    MNG.RegSendFunc([](uint16_t id, uint8_t *data, uint8_t length)
    {
        Serial2.print("can send: ID: ");
        Serial2.print(id);
        Serial2.print(" data: ");
        Serial2.write(data, length);
        Serial2.println();
    });
    
    
    BMS.Init(OnBMSData);

    return;
}


void loop()
{
    uint32_t time = millis();
    
    MNG.Processing(time);
    BMS.Processing(time);
    
    return;
}
