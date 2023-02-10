/*

*/

#pragma once

#include <stdint.h>

class ManagerObjInterface
{
    public:
        
        virtual ~ManagerObjInterface() = default;
        
        virtual uint8_t Processing(uint32_t time) = 0;
        
        virtual uint16_t GetId() = 0;

        virtual void GetBytes(uint8_t *bytes, uint8_t &length) = 0;
};

template <typename T> 
class ManagerObj : public ManagerObjInterface
{
    using update_t = bool (*)(T &newvalue);
    
    public:
        
        /*
            @brief .................
            @param id CAN ID;
            @param update Время обновления значение, мс.
            @param send Время автоотправки значения в CAN, мс. 0 для отключения.
            @param updateFunc Функция запроса обновления значения. В качестве параметра передаётся ссылка на новое значение.
                Если возвращает true, то немедленно отправляет новое значение в CAN, иначе дожидается своего интервала.
        */
        ManagerObj(uint16_t id, uint16_t update, uint16_t send, update_t updateFunc)
         : _id(id), _update(update), _send(send), _updateFunc(updateFunc)
        {
            _force_update = true;
            
            return;
        }
        
        ~ManagerObj() = default;
        
        /*
            Получить текущее значение параметра.
        */
        T GetVal()
        {
            return _value;
        }
        
        /*
            Принудительно обновить значение параметра.
            Обновление произойдёт в следующий тик Processing().
        */
        void Update()
        {
            _force_update = true;
            
            return;
        }
        
        /*
            Принудительно отправить значение в CAN.
            Отправка произойдёт в следующий тик Processing().
        */
        void Send()
        {
            _force_send = true;
            
            return;
        }
        
        /*
            Обработка логики объекта.
        */
        uint8_t Processing(uint32_t time) override
        {
            uint8_t flags = 0x00;
            
            if(_force_update == true || time - _last_update >= _update)
            {
                _last_update = time;
                _force_update = false;
                
                // Если вернули true, то выставляем флаги обновления и отправки данных, иначе только обновления.
                flags = _updateFunc(_value) ? 0x03 : 0x01;
            }
            
            if(_force_send == true || (_send > 0 && time - _last_send >= _send))
            {
                _last_send = time;
                _force_send = false;
                
                flags |= 0x02;
            }
            
            return flags;
        }
        
        /*
            Вернуть CAN ID параметра.
        */
        uint16_t GetId() override
        {
            return _id;
        }
        
        /*
            Получить значение параметра в виде массива байт и его длинны.
        */
        void GetBytes(uint8_t *bytes, uint8_t &length) override
        {
            memcpy(bytes, &_value, sizeof(T));
            length = sizeof(T);
            
            return;
        }
        
    private:
        
        uint16_t _id;               // CAN ID параметра. 0x0000 .. 0x07FF.
        uint16_t _update;           // Интервал автоматического обновления значения.
        uint16_t _send;             // Интервал автоматической отправки значения в CAN.

        uint32_t _last_update;      // Время последнего обновления данных.
        uint32_t _last_send;        // Время последней отправки данных в CAN.
        
        bool _force_update;         // Флаг принудительного обновления значения.
        bool _force_send;           // Флаг принудительной отправки значения в CAN.
        
        update_t _updateFunc;

        T _value;
};
