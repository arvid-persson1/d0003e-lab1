#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>

#define SET(x) (1 << x)

void initClk(void) {
    // CLKPCE: enable change of CLKPS.
    CLKPR = SET(CLKPCE);
    // CLKPS(3:0): division factor 1.
    // The other bits are unimportant.
    CLKPR = 0;
}

void initLcd(void) {
    // LCDEN: enables the driver.
    // LCDAB: low power waveform.
    LCDCRA = SET(LCDEN)   | SET(LCDAB);
    // LCDCS: asynchronous clock source.
    // LCDMUX(1:0): 1/4 duty.
    // LCDPM(2:0): 25 segments.
    LCDCRB = SET(LCDCS)
           | SET(LCDMUX1) | SET(LCDMUX0)
           | SET(LCDPM2)  | SET(LCDPM1)  | SET(LCDPM0);
    // LCDCD(2:0): 8 division ratio.
    LCDFRR = SET(LCDCD2)  | SET(LCDCD1)  | SET(LCDCD0);
    // LCDCC(3:0): 3.35 V contract control.
    LCDCCR = SET(LCDCC3)  | SET(LCDCC2)  | SET(LCDCC1)  | SET(LCDCC0);
}

void initTimer(void) {
    TCCR1B = SET(CS12);
}

void initButton(void) {
    PORTB = SET(PB7);
}

static const uint16_t sccTable[] = {
    0x1551, // 0
    0x0110, // 1
    0x1E11, // 2 
    0x1B11, // 3
    0x0B50, // 4
    0x1B41, // 5
    0x1F41, // 6
    0x0111, // 7
    0x1F51, // 8
    0x0B51  // 9
};

void writeChar(char ch, int pos) {
    if (pos < 0 || pos > 5)
        return;

    int_fast8_t num = ch - '0';
    if (num < 0 || num > 9) {
        clearChar(pos);
        return;
    }

    div_t qr = div(pos, 2);

    uint16_t scc = sccTable[num];
    volatile uint8_t *lcd = &LCDDR0 + qr.quot;
    uint8_t mask = qr.rem ? 0x0F : 0xF0;

    for (uint_fast8_t i = 0; i < 4; i++) {
        uint8_t nib = scc & 0xF;
        if (qr.rem)
            nib <<= 4;

        *lcd = (*lcd & mask) | nib;
        lcd += 0x5;

        scc >>= 4;
    }
}

void clearChar(int pos) {
    if (pos < 0 || pos > 5)
        return;

    volatile uint8_t *lcd = &LCDDR0;
    uint8_t mask = pos % 2 ? 0x0F : 0xF0;

    for (uint_fast8_t i = 0; i < 4; i++) {
        *lcd &= mask;
        lcd += 0x5;
    }
}

void writeLong(long n) {
    uint_fast8_t pos = 5;

    if (n == 0) {
        writeChar('0', pos--);
    } else {
        while (n > 0 && pos > 0) {
            div_t qr = div(n, 10);
            writeChar(qr.rem + '0', pos--);
            n = qr.quot;
        }
    }

    while (pos > 0) {
        clearChar(pos--);
    }
}

bool isPrime(unsigned long n) {
    for (unsigned long i = 2; i * i <= n; i++)
         if (!(n % i))
            return false;

    return true;
}

void primes(void) {
    unsigned long i = 1;
    while (true) {
        if (isPrime(i))
            writeLong(i);
        i++;
    }
}

static const int16_t FREQ = 31250;

void blink() {
    int16_t last = TCNT1, acc = 0;

    while (true) {
        int16_t time = TCNT1,
                diff = time - last;
        last = time;
        acc += diff;

        if (acc >= FREQ) {
            LCDDR3 ^= 1;
            acc -= FREQ;
        }
    }
}

void button(void) {
    LCDDR13 ^= 1;
    bool state = false;

    while(true) {
        if (PINB & SET(PINB7)) {
            state = true;
        } else if (state) {
            LCDDR13 ^= 1;
            LCDDR18 ^= 1;
            state = false;
        }
    }
}

void singlePrime(unsigned long *i) {
    do {
        (*i)++;
    } while (!isPrime(*i));

    writeLong(*i);
}

void checkBlink(int16_t *last, int16_t *acc) {
    int16_t time = TCNT1,
            diff = time - *last;
    *last = time;
    *acc += diff;

    if (*acc >= FREQ) {
        LCDDR3 ^= 1;
        *acc -= FREQ;
    }
}

void checkButton(bool *state) {
    if (PINB & SET(PINB7)) {
        *state = true;
    } else {
        LCDDR13 ^= 1;
        LCDDR18 ^= 1;
        *state = false;
    }
}

int main(void) {
    initClk();
    initLcd();

    // Part 1.
    // primes();

    // Part 2.
    // initTimer();
    // blink();

    // Part 3.
    // initButton();
    // button();
    
    // Part 4.
    // initTimer();
    // initButton();
    //
    // unsigned long prime = 1;
    // int16_t lastTime = 0, accTime = 0;
    // bool buttonState = false;
    // LCDDR13 ^= 1;
    //
    // while (true) {
    //     singlePrime(&prime);
    //     checkBlink(&lastTime, &accTime);
    //     checkButton(&buttonState);
    // }
}
