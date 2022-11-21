#include "RFThings.h"

const char* rft_status_to_str(rft_status_t status) {
	switch (status) {
		case RFT_STATUS_OK:
			return "OK";
			break;
		case RFT_STATUS_TX_DONE:
			return "TX DONE";
			break;
		case RFT_STATUS_RX_DONE:
			return "RX DONE";
			break;
		case RFT_STATUS_WRONG_MIC:
			return "Wrong MIC";
			break;
		case RFT_STATUS_TX_TIMEOUT:
			return "TX TIMEOUT";
			break;
		case RFT_STATUS_RX_TIMEOUT:
			return "RX TIMEOUT";
			break;
		case RFT_STATUS_ERROR_HARDWARE:
			return "Hardware Error";
			break;
		case RFT_STATUS_ERROR_INVALID_PARAM:
			return "Invalid Parameter";
			break;
		case RFT_STATUS_ERROR_INVALID_REGION:
			return "Invalid Region";
			break;
		case RFT_STATUS_PREAMBLE_DETECT_FAIL:
			return "Detect Preamble Fail";
			break;
		case RFT_STATUS_ERROR_WRONG_LR11XX_FIRMWARE_VERSION:
			return "Wrong LR11xx Firmware Versions";
			break;
		case RFT_STATUS_ERROR_INAVLID_LORAWAN_ACTIVATION_TYPE:
			return "Invalid Parameter";
			break;
		default:
			return "Unknown/Invalid Status";
			break;
	}
}

void worship(void) {
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@@@@@@@@@@@@@,@@..@@,@@@@@@@@@@@@@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@)))@@@@@@@@@,@@..@@,@@@@@@@@@(((@@");
	Serial.println(" @@@@@@........................@@@@@@");
	Serial.println(" @..................................@");
	Serial.println(" @@................................@@");
	Serial.println(" @@@..............................@@@");
	Serial.println(" @@@@............................@@@@");
	Serial.println(" @@@@@@........................@@@@@@");
	Serial.println(" @@@@@@@@....................@@@@@@@@");
	Serial.println(" @@@@@@@@@..................@@@@@@@@@");
	Serial.println(" @@@@@@@@@..................@@@@@@@@@");
	Serial.println(" @@@@@@@,,,,,@@@@@@@@@@@@,,,,,@@@@@@@");
}
