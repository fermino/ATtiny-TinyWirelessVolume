/**
 * Thanks to
 * 
 * Rotary encoder ideas: 
 * https://thewanderingengineer.com/2014/08/11/rotary-encoder-on-the-attiny85-part-2/
 * http://bildr.org/2012/08/rotary-encoder-arduino/
 * http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
 * 
 * RF and VirtualWire info: 
 * http://www.lydiard.plus.com/hardware_pages/433mhz_modules.htm
 * 
 * nRF24L01 info: 
 * WHAT, MISO AND MOSI ARE INVERTED?
 * https://forum.arduino.cc/index.php?topic=413878.msg2849746#msg2849746
 * 
 * Add this flag to avr-gcc: -DRANDOM_32b=0x`hexdump -e '"%x"' -n4 /dev/random`
 */

	#include "Configuration.h"

	#include <avr/eeprom.h>

	// Encoder lib (based on buxtronix's one)
	#include <OneWireRotaryEncoder.h>

	// RF
	#include <RF24.h>

	#ifndef cbi
		#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
	#endif
	#ifndef sbi
		#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
	#endif

	OneWireRotaryEncoder<ENCODER_INPUT_PIN> Encoder(ENCODER_R2, ENCODER_RA, ENCODER_RB, ENCODER_RBUTTON, ENCODER_READ_TOLERANCE);
	int8_t EncoderValue = 0;
	int16_t EncoderButton_PressedAt = 0;

	// Use pin 5 for Chip-Enable if reset pin is disabled, else, tie CE pin to ground
	RF24 RF(6, NRF_CSN_PIN);
	uint8_t RF24WritingPipe[5] = NRF_WRITING_PIPE;

	uint8_t DeviceID[TX_DEVICE_ID_LENGTH];

	void setup()
	{
		// Get device's id from eeprom (address 0)
		// This gets written when the tiny is programmed
		for(uint8_t Address = 0; Address < TX_DEVICE_ID_LENGTH; Address++)
			DeviceID[Address] = eeprom_read_byte(Address);

		// Configure RF module

		RF.begin();
		RF.setChannel(NRF_CHANNEL);
		RF.setDataRate(NRF_DATA_RATE);

		// Disable if there are race conditions
		RF.setAutoAck(1);
		RF.setRetries(2, 15);

		RF.openWritingPipe(RF24WritingPipe);
	}

	void loop()
	{
		uint8_t EncoderRead = Encoder.process();

		if(EncoderRead == DIR_CW)
			EncoderValue++;
		else if(EncoderRead == DIR_CCW)
			EncoderValue--;

		if(EncoderValue != 0)
		{
			uint8_t Message[2]
			{
				0x11,				// Command	=> Relative position move
				EncoderValue		// Data		=> Relative position
			};

			SendMessage(Message, 2);

			// Reset counter to 0
			EncoderValue = 0;
		}

		if(Encoder.buttonPressed())
		{
			// If -1, the button is disabled until the user releases it
			if(EncoderButton_PressedAt >= 0)
			{
				if(EncoderButton_PressedAt > 0)
				{
					if(millis() - EncoderButton_PressedAt >= ENCODER_BUTTON_THRESHOLD)
					{
						uint8_t Message[1]
						{
							0x12	// Command => Mute
						};
						
						SendMessage(Message, 1);

						EncoderButton_PressedAt = -1;

						delay(ENCODER_BUTTON_DEBOUNCE);
					}
				}
				else
					EncoderButton_PressedAt = millis();
			}
		}
		else
			EncoderButton_PressedAt = 0;
	}

	void SendMessage(uint8_t* Data, uint8_t DataLength)
	{
		if(DataLength > NRF_PACKET_BODY_LENGTH)
			return;

		uint8_t Message[NRF_PACKET_LENGTH];

		// Device ID (should be different in each chip)
		Message[0] = DeviceID[0];
		Message[1] = DeviceID[1];
		Message[2] = DeviceID[2];
		Message[3] = DeviceID[3];

		// Message ID
		Message[4] = random(255); // Use millis() ?

		for(uint8_t i = NRF_PACKET_HEADER_LENGTH; i < NRF_PACKET_LENGTH; i++)
		{
			if((i - NRF_PACKET_HEADER_LENGTH) < DataLength)
				Message[i] = Data[i - NRF_PACKET_HEADER_LENGTH];
			else
				Message[i] = 0x00;
		}

		// Non-blocking write
		RF.startWrite(Message, NRF_PACKET_LENGTH, NRF_MULTICAST);
	}