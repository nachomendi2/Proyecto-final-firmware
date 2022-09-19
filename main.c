#include <msp430.h> 
#include <stdio.h>

// TOKEN PARA GIT: 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

/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer (siempre se hace)
	printf("Hola MSP430!");
	
	return 0;
}
