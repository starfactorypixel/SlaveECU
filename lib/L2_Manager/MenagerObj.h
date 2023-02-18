/*

*/

#pragma once

#include <stdint.h>

class ManagerObjInterface
{
    public:
        
        
        enum response_t : uint8_t
        {
            RESP_NONE,
            RESP_OK,
            RESP_ERROR,
        };
        
        enum error_t : uint8_t
        {
            ERROR_NONE,
            ERROR_LOW_CURRENT,
            ERROR_HIGH_CURRENT,
            ERROR_LEAKAGE_CURRENT,
        };
        
        enum event_t : uint8_t
        {
            EVENT_NONE = 0x00,
            EVENT_NORMAL = 0xE1,
            EVENT_WARNING = 0xE2,
            EVENT_CRITICAL = 0xE3,
            EVENT_ERROR = 0xE6,
        };
        
        #pragma pack(push, 1)
        struct packet_t
        {
            uint8_t length;
            uint8_t type;
            uint8_t data[7];
        };
        #pragma pack(pop)
        
        virtual ~ManagerObjInterface() = default;
        
        virtual uint8_t Processing(uint32_t time, packet_t &packet) = 0;

        virtual void InputPacket(packet_t &packet) = 0;
        
        virtual uint16_t GetId() = 0;

        virtual void GetBytes(uint8_t *bytes, uint8_t &length) = 0;

        //virtual void SetBytes(uint8_t *bytes, uint8_t length) = 0;
};

template <typename T> 
class ManagerObj : public ManagerObjInterface
{
    using update_t = event_t (*)(T &value, error_t &error);
    using input1_t = response_t (*)(T &value, error_t &error);
    using input2_t = response_t (*)(packet_t &packet, error_t &error);
    //typedef std::function<response_t(T &value)> input_t;
    
    public:


        
        /*
            @brief .................
            @param id CAN ID;
            @param update Время обновления значение, мс.
            @param send Время автоотправки значения в CAN, мс. 0 для отключения.
            @param updateFunc Функция запроса обновления значения. В качестве параметра передаётся ссылка на новое значение.
                Если возвращает true, то немедленно отправляет новое значение в CAN, иначе дожидается своего интервала.
        */
        ManagerObj(uint16_t id, uint16_t update, uint16_t send, update_t updateFunc, input1_t inputFunc)
         : _id(id), _update(update), _send(send), _updateFunc(updateFunc), _inputFunc1(inputFunc), _use_raw(false)
        {
            _force_update = true;
            
            return;
        }
        
        ManagerObj(uint16_t id, uint16_t update, uint16_t send, update_t updateFunc, input2_t inputFunc)
         : _id(id), _update(update), _send(send), _updateFunc(updateFunc), _inputFunc2(inputFunc), _use_raw(true)
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
        uint8_t Processing(uint32_t time, packet_t &packet) override
        {
            uint8_t flags = 0x00;
            
            if(_force_update == true || (_update > 0 && time - _last_update >= _update))
            {
                _last_update = time;
                _force_update = false;
                
                
                event_t func_event;
                error_t func_error = ERROR_NONE;
                func_event = _updateFunc(_value, func_error);

                switch(func_event)
                {
                    case EVENT_WARNING:
                    case EVENT_CRITICAL:
                    {
                        packet.length = sizeof(T);
                        packet.type = func_event;
                        *(T *)packet.data = _value;

                        flags = 0x03;

                        // Костыльный способ запрета отправки по таймеру.
                        _last_send = time;
                        
                        break;
                    }
                    case EVENT_ERROR:
                    {
                        packet.length = 2;
                        packet.type = EVENT_ERROR;
                        packet.data[0] = func_error;

                        flags = 0x03;

                        // Костыльный способ запрета отправки по таймеру.
                        _last_send = time;
                        
                        break;
                    }
                    default:
                    {
                        flags = 0x01;
                        
                        break;
                    }
                }
            
            
            }
            
            if(_force_send == true || (_send > 0 && time - _last_send >= _send))
            {
                _last_send = time;
                _force_send = false;

                packet.length = sizeof(T);
                packet.type = EVENT_NORMAL;
                *(T *)packet.data = _value;
                
                flags |= 0x02;
            }
            
            return flags;
        }
        
        /*
            Обработка входящего пакета.
        */
        void InputPacket(packet_t &packet) override
        {
            
            response_t func_resp;
            error_t func_error = ERROR_NONE;
            
            // Если обработчик входящего пакета работает с парсенными значениями.
            if(_use_raw == false)
            {
                /*
                T in_value;
                ParseBytes(packet.data, packet.length, in_value);
                */
                
                T *in_value;
                in_value = (T *)packet.data;
                
                func_resp = _inputFunc1(*in_value, func_error);

                *(T *)packet.data = *in_value;
            }
            // Если обработчик входящего пакета работает с сырыми данными.
            else
            {
                func_resp = _inputFunc2(packet, func_error);
            }
            
            switch(func_resp)
            {
                case RESP_OK:
                {
                    packet.type |= 0x40;
                    
                    break;
                }
                case RESP_ERROR:
                {
                    packet.length = 2;
                    packet.type |= 0xC0;
                    packet.data[0] = func_error;
                    
                    break;
                }
                default:
                {
                    break;
                }
            }
            
            return;
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
        
        /*
            Вставить полученное значение.
        */
        void ParseBytes(uint8_t *bytes, uint8_t length, T &value)
        {
            memcpy(&value, &bytes, sizeof(T));

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
        input1_t _inputFunc1;
        input2_t _inputFunc2;

        bool _use_raw;

        T _value;
};
