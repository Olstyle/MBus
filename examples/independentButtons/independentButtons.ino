/***
Example to the Mbus library.
* This example uses the CD-Buttons of the headunit to do individual tasks
* as well as the ffwd, the fbwd and the skip buttons.
* 


Copyright [2012] [Oliver Mueller]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***/

/*******************************************************************
initialisation
*******************************************************************/
#include <MBus.h>

#define mBusIn 10 //input port
#define mBusOut 12 //output port

MBus mBus(mBusIn, mBusOut); //make an object of MBus to work with 

void setup()
{
	//default to cd 1 track 1
	mBus.sendChangedCD(1,1);
	mBus.sendCDStatus(1);
	mBus.sendPlayingTrack(1,0);
}
/***************************************************************
loop
****************************************************************/
void interpretDisc(int disc) //set your function per cd-button here
{
	switch(disc)
	{
	case 1:
		/*do something*/
		break;
	case 2:
		/*do something*/
		break;
	case 3:
		/*do something*/
		break;
	case 4:
		/*do something*/    
		break;
	case 5:
		/*do something*/
		break;
	case 6:
		/*do something*/
		break;
	default:
		break; 
	}
}


void fwOrBw(uint8_t oldTrack, uint8_t newTrack) // to interpret track changes as consecutive forward/backward skips
{
	uint8_t i=0;
	if(oldTrack<newTrack)
	{
		for(i=oldTrack;i<newTrack;i++)
		{
			/*skip forward*/
		}
	}
	else
	{
		for(i=newTrack;i<oldTrack;i++)
		{
			/*skip backwards*/
		}
	}
}

boolean		ignoreNext=false;
boolean		wasCD=false;
uint8_t		currentCD=1;
uint8_t		currentTrack=1;
uint64_t	receivedMessage=0;
uint64_t	lastMessage=0;
uint64_t	nextTime=0;
uint64_t	timeout=0;
boolean		isOn=true;

void loop()
{
	if(nextTime<millis()&&isOn)
	{
		mBus.sendPlayingTrack(currentTrack,(uint16_t)(millis()/1000)); 
		nextTime=millis()+500;
	}
	if(mBus.receive(&receivedMessage))
	{
		if(ignoreNext)
		{
			ignoreNext=false;
			if(wasCD)
			{
				mBus.sendCDStatus(currentCD);
				wasCD=false;
			}
		}
		
	
		else if(receivedMessage == Ping)
		{
			mBus.send(PingOK);//acknowledge Ping
		}
		else if(receivedMessage == 0x19)
		{
			mBus.sendChangedCD(currentCD,currentTrack); //'still at cd ...'
			delay(50);
			mBus.sendCDStatus(currentCD);
			delay(50);
			mBus.sendPlayingTrack(currentTrack,0);
		}
		else if(receivedMessage==OFF&&isOn)
		{
			/*do something before shutdown*/
			mBus.send(Wait);//acknowledge
			isOn=false;
		}
		else if(receivedMessage==Play)
		{
			mBus.sendPlayingTrack(currentTrack,(uint16_t)(millis()/1000));    
		}
		else if(receivedMessage==0x11104)
		{
			mBus.send(Wait);//acknowledge
			/* do somethinmg on ffwd button*/     
		}
		else if(receivedMessage==0x11108)
		{
			mBus.send(Wait);//acknowledge
			/* do somethinmg on fbwd button*/        
		}
		else if(receivedMessage>>(4*5)==0x113)//'please change cd'
		{
			uint64_t test=(receivedMessage >>(4*4))-(uint64_t)0x1130; 
			if(test>0)//same cd as before
			{
				mBus.sendChangedCD(test,1);//'did change'
				delay(50);
				mBus.sendCDStatus(currentCD);
				currentCD=test;
				currentTrack=1;
				wasCD=true;
				
				if(timeout<millis())//debounce
				{
					interpretDisc(currentCD);
					timeout=millis()+1000;
				}
			}
			else
			{
				uint8_t lastTrack=currentTrack;
				currentTrack=(receivedMessage&((uint64_t)0xF<<(4*2)))>>(4*2);
				currentTrack+=((receivedMessage&((uint64_t)0xF<<(4*3)))>>(4*3))*10;
				
				mBus.sendChangedCD(currentCD,currentTrack); //'still at cd...'
				delay(50);
				mBus.sendCDStatus(currentCD);
				delay(50);
				mBus.sendPlayingTrack(currentTrack,0);
				if(timeout<millis())//debounce
				{
					if(currentTrack!=lastTrack&&currentTrack>1)
					{
						fwOrBw(lastTrack, currentTrack);
					}
					else if(currentTrack==1)
					{
						interpretDisc(currentCD);
						wasCD=true;
					}
					else
					{
						/* Error */
					}
					timeout=millis()+1000;
				}
			}
			ignoreNext=true;
		}
	}
}




