/*
MBus.cpp - Library to emulate Alpine M-Bus commands
Written by Oliver Mueller in October, 2012
*/
#include "MBus.h"


MBus::MBus(uint8_t in, uint8_t out)
{
	_in=in;
	_out=out;
	
	pinMode(_in, INPUT);
	pinMode(_out,OUTPUT);
}

void MBus::sendZero()
{
	digitalWrite(_out, HIGH);
	delayMicroseconds(600);
	digitalWrite(_out, LOW);
	delayMicroseconds(2400);
}

void MBus::sendOne()
{
	digitalWrite(_out, HIGH);
	delayMicroseconds(1800);
	digitalWrite(_out, LOW);
	delayMicroseconds(1200);
}

void MBus::writeHexBitWise(uint8_t message)
{
		for(uint8_t i=3; i>-1; i--)
	{
		uint8_t output=((message & (1<<i) )>>i);
		if(output==1)
		{
			sendOne();
		}
		else
		{
			sendZero();
		}
	}
}

boolean MBus::checkParity(uint64_t *message)
{
	uint8_t parity=0;
	uint8_t test=0;
	for(uint8_t i=16; i>0; i--)
	{
		test=((*message & ((uint64_t)0xF<<i*4) )>>i*4);
		parity=parity^test;
	}
	parity+=1;
	
	if(parity==(*message & ((uint64_t)0xF)))
		return true;
	else
	{
		return false;
	}
}

void MBus::send(uint64_t message)
{
	uint8_t printed=0;
	uint8_t parity=0;
	for(uint8_t i=16; i>=0; i--)
	{
		uint8_t output=((message & ((uint64_t)0xF<<i*4) )>>i*4);
	parity=parity^output;
		if(!output&&!printed)
		{
			//do nothing
		}
		else
		{
			writeHexBitWise(output);
			printed++;
		}
	}
	parity+=1;
	writeHexBitWise(parity);
	printed++;
}

boolean MBus::receive(uint64_t *message)
{
	*message=0;
	if(digitalRead(_in)==LOW)
	{
		unsigned long time=micros();

		boolean gelesen=false; 
		uint8_t counter=0;

	while((micros()-time)<4000)
	{
		if(digitalRead(_in)==HIGH&&!gelesen)
		{
			if((micros()-time)<1400&&(micros()-time)>600)//0 ist in between 600 and 1700 microseconds
			{
			  *message*=2;
			  counter++;
			  gelesen=true; 
			}
			else if((micros()-time)>1400)//1 ist longer then 1700 microseconds
			{
			  *message*=2;
			  *message+=1;
			  counter++;
			  gelesen=true;
			  
			}
		}
		if(gelesen&&digitalRead(_in)==LOW)  
		{
			gelesen=false;
			time=micros();
		  }  
	}
	if(counter%4||!checkParity(message)||counter==0)
	{
		//message is not ok
		*message=0;
                return false;
	}
	else
	{
		(*message)=(*message)>>4;//ingnore parity
                return true;
	}
	}

}

/*
 CD-changer emulation from here on
*/
void MBus::sendPlayingTrack(uint8_t Track,uint16_t Time)
{
	uint64_t play=0x994000100000001ull;
	play|=(uint64_t)Track<<(10*4);
	
	play|=(uint64_t)(Time%10)<<(4*4);
	play|=(uint64_t)((Time%100)/10)<<(5*4);
	play|=(uint64_t)((Time/60)%10)<<(6*4);
	play|=(uint64_t)(((Time/60)%100)/10)<<(7*4);
	
	send(play);
}

void MBus::sendChangedCD(uint8_t CD)
{
	uint64_t play=0x9B900100001ull;
	play|=(uint64_t)CD<<(7*4);
	send(play);
}

void MBus::sendCDStatus(uint8_t CD)
{
	uint64_t play=0x9C001999999Full;
	play|=(uint64_t)CD<<(9*4);
	send(play);
}