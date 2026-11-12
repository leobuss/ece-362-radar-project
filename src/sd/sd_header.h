#define CMD0 0x00 // go idle
#define CMD1 0x01 // initiates card initialization
#define CMD8 0x08 // checks voltage range
#define CMD55 0x37 // prefixes application specific command
#define ACMD41 0x29 // waits for card to exit initialization state
#define CMD58 0x3A // checks power supply voltage
#define CMD16 0x10 // set block length in bytes
#define CMD17 0x11 // read single block
#define CMD18 0x12 // read multiple blocks
#define CMD24 0x18 // write single blocks
#define CMD25 0x19 // write multiple block
#define CMD55 0x37 // prefix application specific command
#define ACMD41 0x29 // initialize SD card (always after CMD55)

#define SD_START_BLOCK 0xFE
#define SD_MULTI_WRITE_START 0xFC
#define SD_MULTI_WRITE_STOP 0xFD
#define SD_DATA_ACCEPTED 0x05
