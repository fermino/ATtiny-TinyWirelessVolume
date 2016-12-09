/**
 * Thanks to
 * https://thewanderingengineer.com/2014/08/11/rotary-encoder-on-the-attiny85-part-2/
 * http://bildr.org/2012/08/rotary-encoder-arduino/
 */

	#include <VirtualWire.h>

	#include <avr/interrupt.h>;
	#include <util/delay.h>

	#ifndef cbi
		#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
	#endif
	#ifndef sbi
		#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
	#endif

	// Rotary encoder configuration

		#define ENCODER_BUTTON_PIN 0

		#define ENCODER_PIN_A 3
		#define ENCODER_PIN_B 4
		
		#define ENCODER_BUTTON_THRESHOLD 25

	// RF Module configuration

		#define RF_TX_PIN 1

		// Most of the times with a lower bitrate the RF module's range is better, but, somehow, with lower speeds it works worse
		// If it doesn't work, try changing it to 200+

		#define RF_BITRATE 2000

		#define RF_REPEAT 3

	// Shared variables used by the interrupt

	volatile uint8_t LastEncoderRead = 0;
	volatile int16_t EncoderValue = 0;
	 
	void setup()
	{
		// Configure VirtualWire library to work with 433MHz RF module

		vw_set_tx_pin(RF_TX_PIN);
		vw_setup(RF_BITRATE);

		// Set encoder's pins as inputs with pullup resistors (encoder is tied to ground)

		pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

		pinMode(ENCODER_PIN_A, INPUT_PULLUP);
		pinMode(ENCODER_PIN_B, INPUT_PULLUP);

		// Disable interrupts before configuration
		cli();

		// Enable Pin Change Interrupts in pins 3 and 4

		sbi(GIMSK, 5);

		sbi(PCMSK, 3);
		sbi(PCMSK, 4);

		// Turn on interrupts
		sei();
	}

	void loop()
	{
		if(ReadPulse(ENCODER_BUTTON_PIN) > ENCODER_BUTTON_THRESHOLD)
		{
			char Data[] = {random(255), 0x11};

			for(int i = 0; i < RF_REPEAT; i++)
			{
				vw_send((uint8_t*) Data, 2);
				vw_wait_tx();
			}
		}

		if(EncoderValue >= 3 || EncoderValue <= -3)
		{
			char Data[] = {random(255), 0x10, EncoderValue >> 8, EncoderValue & 0xFF};

			for(int i = 0; i < RF_REPEAT; i++)
			{
				vw_send((uint8_t*) Data, 4);
				vw_wait_tx();
			}

			EncoderValue = 0;

			_delay_ms(500);
		}
	}

	uint16_t ReadPulse(uint8_t Pin)
	{
		uint16_t Time = 0;

		while(digitalRead(Pin) == LOW)
		{
			Time++;
			delay(1);
		}

		return Time;
	}

	ISR(PCINT0_vect)
	{
		uint8_t Read = (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);

		switch((LastEncoderRead << 2) | Read)
		{
			case 0b1101:
			case 0b0100:
			case 0b0010:
			case 0b1011:
				EncoderValue++;
				break;

			case 0b1110:
			case 0b0111:
			case 0b0001:
			case 0b1000:
				EncoderValue--;
				break;
		}

		LastEncoderRead = Read;
	}