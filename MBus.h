/*
MBus.h - Library to emulate Alpine M-Bus commands
Written by Oliver Mueller in October, 2012
*/
#ifndef MBus_h
#define MBus_h

	#include "Arduino.h"
	
	#define Shutdown 11142
	#define Ping 0x18
	#define	Play 0x11101
	#define PingOK 0x98
	#define Wait 0x9F00000
	#define OFF 0x11142
	#define Changing 0x9B910100001
	
	class MBus
	{
		public:
			MBus(uint8_t in, uint8_t out);
			void send(uint64_t message);
			boolean receive(uint64_t *message);
			void sendPlayingTrack(uint8_t Track,uint16_t Time);
			void sendChangedCD(uint8_t CD);
			void sendCDStatus(uint8_t CD);
		private:
			uint8_t _in;
			uint8_t _out;
			void sendZero();
			void sendOne();
			void writeHexBitWise(uint8_t message);
			boolean checkParity(uint64_t *message);
	};
#endif	