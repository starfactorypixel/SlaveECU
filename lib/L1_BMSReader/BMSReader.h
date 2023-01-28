/*

*/

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <BMSPacket.h>

class BMSReader
{
	public:

		using callback_t = void (*)(BMSPacket::params_t params);
		
		BMSReader(HardwareSerial &serial) : _serial(serial)
		{
			return;
		}
		
		/*
			Инициализация класса.
		*/
		void Init(callback_t callback)
		{
			// Не получилось убрать HardwareSerial внутрь класса.
			//HardwareSerial _serial(PB11, PB10);
			//_serial.setRx(PB11);
			//_serial.setTx(PB10);
			_serial.begin(19200, SERIAL_8N1);
			
			_packet.Init();
			
			_callback = callback;
			
			return;
		}
		
		/*
			Отправка запроса в BMS.
		*/
		void Request()
		{
			_serial.write(_packet.RequestData, sizeof(_packet.RequestData));
			
			return;
		}
		
		/*
			Проверяем ответ от BMS и вызываем callback.
		*/
		void Processing(uint32_t time)
		{
			
			while( _serial.available() > 0 )
			{
				uint8_t left = _packet.PutByte( _serial.read() );
				if(left == 0)
				{
					if(_packet.IsReady() == true)
					{
						_callback(_packet.params);
						
						_packet.Init();
					}
					else
					{
						// Переполнение.
					}
				}
			}
			
			return;
		}
	
	private:
		
		HardwareSerial &_serial;
		BMSPacket _packet;
		callback_t _callback;
		
};
