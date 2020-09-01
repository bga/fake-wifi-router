#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#include <!cpp/Random/XorShift32.h>
#include <!cpp/bitManipulations.h>
#include <!cpp/common.h>

#include "common.h"

enum {
	timeQuant = 2000UL / 256,

	eth1Port = 0,
	eth2Port = 1,
	eth3Port = 2,
	wlanPort = 3,
	modemPort = 4,

	PORTBXorMask = _BV(eth1Port) | _BV(eth2Port) | _BV(eth3Port)
};

#define msToTicksCount(msArg) ((msArg) / timeQuant)

Random::XorShift32 random;

inline void togglePort(U8 portIndex) {
	PINB = _BV(portIndex);
}

inline U8 setPortBValue(U8 portValue) {
	PORTB = portValue ^ PORTBXorMask;

	return portValue;
}


#define Self SoftTimer
class Self {
	public:
	void onTick() {

	}
};
#undef Self

#define Self EthernetLedTimer
template<int portArg> class Self {
	public:
	U8 delay = 1;
	enum { port = portArg };

	void onTick() {
		if(--(*this).delay == 0) {
			togglePort(port);
			(*this).delay = 5 + random.generate(4);
		}
		else {

		}
	}
};
#undef Self

#define Self WlanLedTimer
template<int portArg> class Self {
	public:
	U8 delay = 32;

	enum { port = portArg };

	void onTick() {
		if(--(*this).delay == 0) {
			togglePort(port);
			(*this).delay = 2 + random.generate(2);
			if(PINB & _BV(port)) {
				(*this).delay <<= 4;
			}
			else {

			}
		}
		else {

		}
	}
};
#undef Self

#define Self ModemLedTimer
template<int portArg> class Self {
	public:
	U8 delay = 64;

	enum { port = portArg };

	void onTick() {
		if(--(*this).delay == 0) {
			togglePort(port);
			(*this).delay = 64;
		}
		else {

		}
	}
};
#undef Self


EthernetLedTimer<eth1Port> ethernetLed1Timer;
EthernetLedTimer<eth2Port> ethernetLed2Timer;
EthernetLedTimer<eth3Port> ethernetLed3Timer;
WlanLedTimer<wlanPort> wlanLedTimer;
ModemLedTimer<modemPort> modemLedTimer;

volatile U16 bootSeq = 0;

ISR(TIM0_COMPA_vect){
	wdt_reset();
	if(bootSeq != msToTicksCount(5000)) {
		;(bootSeq == 0) && (setPortBValue(0));
		;(bootSeq == msToTicksCount(1200)) && (setPortBValue(_BV(eth1Port) | _BV(eth2Port) | _BV(eth3Port) | _BV(wlanPort)));
		;(bootSeq == msToTicksCount(1500)) && (setPortBValue(0));
		;(bootSeq == msToTicksCount(3000)) && (setPortBValue(_BV(wlanPort)));
		;(bootSeq == msToTicksCount(4000)) && (setPortBValue(_BV(wlanPort) | _BV(modemPort)));
		bootSeq += 1;

	}
	else {
		ethernetLed1Timer.onTick();
		ethernetLed2Timer.onTick();
		ethernetLed3Timer.onTick();
		wlanLedTimer.onTick();
		modemLedTimer.onTick();
	}
	// PINB = _BV(1);
	//if(--delayElapced == 0) {
		// PINB = _BV(0);
	//}
}

void setup() {
	//# set cpu clock prescaler to 1
	if(0) {
	  CLKPR = 0x80;
  	CLKPR = 0x00;
	}

	#if 1
	//# enable and reset watchdog from prev mcu reset
	MCUSR = 0;

	#if WDTON_FUSE_ENABLED
	wdt_reset();
	WDTCR |= (1 << WDCE) | (1 << WDE);
	WDTCR = (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (0 << WDP0);
	#else
	WDTCR = (1 << WDE) | (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (0 << WDP0);
	wdt_reset();
	#endif // WDTON_FUSE_ENABLED

	#endif // 0

	DDRB = _BV(eth1Port) | _BV(eth2Port) | _BV(eth3Port) | _BV(wlanPort) | _BV(modemPort);
	//PORTB |= (1 << heartBeatPin);
//	PORTB = _BV(0) | _BV(1);


	//# setup timer
	TCCR0A = TCCR0B = 0;

	//prescale timer to 1024
	TCCR0B |= (1 << CS02) | (1 << CS00);

	//# set compare match register for 1000 Hz increments
	const auto counter = F_CPU / (1024UL * (1000 / timeQuant));
	static_assert(counter < 256, "timer counter should be less 256");
	static_assert(1 < counter, "timer counter should be more than 1");
	OCR0A = counter - 1;

	//# turn on CTC mode
	TCCR0A |= (1 << WGM01);

	//# enable timer overflow interrupt
	TIMSK0 = (1 << OCIE0A);

	sei();
}

int main() {
	setup();

	while(1) {
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();
	}
  return 0;
}
