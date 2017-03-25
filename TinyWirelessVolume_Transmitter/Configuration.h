
		#define TX_DEVICE_ID_LENGTH 4

	/**
	 * Interface
	 */
		
		#define ENCODER_BUTTON_MUTE_THRESHOLD 100

		#define ENCODER_BUTTON_RESET_THRESHOLD 4000
	
	/**
	 * Encoder hardware configuration
	 */
		#define ENCODER_INPUT_PIN A3

		#define ENCODER_READ_TOLERANCE 40

		#define ENCODER_RA 10000 /* 21k */
		#define ENCODER_RB 20000 /* 10k5 */

		// 0 if your encoder has no button
		#define ENCODER_RBUTTON 33000 /* 31k */

		#define ENCODER_R2 10000 /* 10k8 */

	/**
	 * nRF24L01 configuration
	 */

		// Pinout
		#define NRF_CSN_PIN 4

		// RF
		// Default nRF24's channel is 76
		#define NRF_CHANNEL 76 
		#define NRF_DATA_RATE RF24_250KBPS

		#define NRF_PACKET_HEADER_LENGTH (TX_DEVICE_ID_LENGTH + 1)
		#define NRF_PACKET_BODY_LENGTH 5
		#define NRF_PACKET_LENGTH (NRF_PACKET_HEADER_LENGTH + NRF_PACKET_BODY_LENGTH)

		// Request ACK or not?
		#define NRF_MULTICAST false
		
		// Project0, Project1, Project2, API version, Channel number (0 for any to master, 1 for master to any)
		#define NRF_WRITING_PIPE {'T', 'W', 'V', 0, 0}