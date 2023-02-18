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


/* Данный параметр каждую секунду получает новые данные и автоматически отправляет их в CAN тоже каждую секунду.
   Первая лямбда отвечает за получение нового значения и отправляет его немедленно если лямбда возвращает true. 
    В протвином случае  отправка происходит через указанное время.
   Вторая лямбда отвечает за обработку полученного запроса из CAN и выполняет действие в зависимости от возвращаемого значения.
*/
ManagerObj<uint32_t> HiVolt(0x00A0, 1000, 1000, 
    [](uint32_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::event_t { value = 82421; return ManagerObjInterface::EVENT_NONE; },
    [](uint32_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::response_t { /*Включаем \ Выключаем фары*/ /*Формирует ответ редактируя входные данные*/ return ManagerObjInterface::RESP_OK; }
);
ManagerObj<uint8_t> MaxTemp(0x00AA, 3000, 10000, 
    [](uint8_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::event_t { value = 35; return ManagerObjInterface::EVENT_NONE; },
    [](uint8_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::response_t { /*Включаем \ Выключаем фары*/ /*Формирует ответ редактируя входные данные*/ return ManagerObjInterface::RESP_OK; }
);



/*
    0. По умолчанию система молчит и ничего не отправляет.
    1. Пытаемся включить / выключить фары, замеряем ток.
    2.1 Если ток в норме, то отвечаем текущим пакетом с поправленным типом |= 0x40 (успех).
    2.2 Если ток не в норме, то отвечаем пакетом с поправленным типом |= 0xC0 (ошибка) + код ошибки в данных.
    3. Переодически опрашиваем ток и сравниваем его с нормой.
    4.1 Если ток в норме, то ничего не делаем.
    4.2 Если ток не в норме, то формируем ошибку с типом |= 0xC0 (ошибка) + код ошибки в данных.
*/




ManagerObj<uint8_t> LowBeam(0x0123, 5, 0, 
    [](uint8_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::event_t
    {
        /* Если фары включены */
        /* Проверяем фары на корректную работу */
        /* Если всё ок, то возвращаем EVENT_NONE */
        /* Если нет, то выключаем фары, выставляем код ошибки и возвращаем EVENT_ERROR, что приведёт к отправке сообщения в CAN */
        
        return ManagerObjInterface::EVENT_NONE;
    },
    [](uint8_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::response_t
    {
        /* Включаем \ Выключаем фары */
        /* Проверяем что фары включились успешно */
        /* Если всё ок, то возвращаем RESP_OK. Так-же т.к. у нас не реализован PWM на свете, то любое значение > 0 мы приводим к 0xFF. */
        /* Если нет, то выключаем фары, выставляем код ошибки и возвращаем RESP_ERROR */
        /* Любой результат приведёт к отправке сообщения в CAN */
        
        return ManagerObjInterface::RESP_OK;
    }
);



ManagerObj<uint32_t> HighVoltage(0x0555, 1000, 1000, 
    [](uint32_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::event_t
    {
        /* Каждые 1000мс получаем актуальное значение напряжения */
        /* Записываем его в переменную */
        /* Если всё ок, то возвращаем EVENT_NONE */
        /* Если нет, то выставляем код ошибки error_t и тип важности события event_t, что приведёт к отправке сообщения в CAN */

        return ManagerObjInterface::EVENT_NONE;
    },
    [](uint32_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::response_t
    {
        /* Если приходит запрос на текущее напряжение */
        /* То получаем его, преобразовываем в мВ */
        /* Если всё ок, то выставляем значение и возвращаем RESP_OK. */
        /* Если нет, то выставляем код ошибки и возвращаем RESP_ERROR. */
        /* Любой результат приведёт к отправке сообщения в CAN */
        
        return ManagerObjInterface::RESP_OK;
    }
);







ManagerObj<uint8_t> BlockCfg(0x0789, 0, 0, 
    [](uint8_t &value, ManagerObjInterface::error_t &error) -> ManagerObjInterface::event_t { return ManagerObjInterface::EVENT_NONE; },
    [](ManagerObjInterface::packet_t &packet, ManagerObjInterface::error_t &error) -> ManagerObjInterface::response_t
    {
        /* Включаем \ Выключаем фары */
        /* Проверяем что фары включились успешно */
        /* Формирует ответ редактируя входные данные если нужно */
        
        return ManagerObjInterface::RESP_OK;
    }
);



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
    // Или используем Manager.GetMask() для передачи в CAN контроллер в качестве маски фильтра.
    if( MNG.IsHere(id) == true )
    {
        if(/*Проверяем тип пакета, типа запроса*/ true)
        {
            // Что то делаем..

            MNG.InputPacket(id, data, length);
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
