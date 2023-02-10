/*

*/

#pragma once

#include <stdint.h>
#include <MenagerObj.h>

template <uint8_t _max_param = 16>
class Manager
{
    static const uint8_t _tick_time = 5;

    using send_t = void (*)(uint16_t id, uint8_t *data, uint8_t length);
    
    public:
        
        void RegParam(ManagerObjInterface &param)
        {
            if(_max_param > _objects_idx)
            {
                _objects[_objects_idx++] = &param;
                _id_mask |= param.GetId();
            }
            
            return;
        }
        
        void RegSendFunc(send_t func)
        {
            _send_func = func;
            
            return;
        }

        bool IsHere(uint16_t id)
        {
            return ((id & _id_mask) == id);
        }

        void Processing(uint32_t time)
        {
            if(time - _last_tick >= _tick_time)
            {
                _last_tick = time;
                
                for(uint8_t i = 0; i < _objects_idx; ++i)
                {
                    // Передавать time по ссылке и обновлять перед каждым вызовом?
                    uint8_t flags = _objects[i]->Processing(time);
                    if(flags & 0x02)
                    {
                        uint8_t *bytes;
                        uint8_t length;
                        _objects[i]->GetBytes(bytes, length);
                        _send_func(_objects[i]->GetId(), bytes, length);
                    }
                }
            }
            
            return;
        }

    private:

        ManagerObjInterface *_objects[_max_param];
        uint8_t _objects_idx = 0;

        send_t _send_func;

        uint16_t _id_mask = 0b0000000000000000;

        uint32_t _last_tick = 0;
        
};
