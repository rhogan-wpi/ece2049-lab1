#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"
//Testing laptop commit
typedef enum{
  INIT,
  OUTPUT,
  WAIT_FOR_INPUT,
  INPUT_TRUE,
  INPUT_FALSE,
  GAME_OVER,
} state;
// Test diff

void fake_delay(int seconds)
{
  volatile unsigned int n = 0;
  while (n < (seconds * 6500)) {
    n++;
  }
}

char get_input()
{
  char key = 0;
  while (key == 0) {
    key = getKey();
  }
  return key;
}

void set_leds(char key)
{
  key = key - '0';
  unsigned char output_mask = 0;
  // Zero output register
  P6OUT &= ~(BIT4|BIT3|BIT2|BIT1);
  if (key == 1)
    output_mask = output_mask | BIT2;
  if (key == 2)
    output_mask = output_mask | BIT1;
  if (key == 3)
    output_mask = output_mask | BIT3;
  if (key == 4)
    output_mask = output_mask | BIT4;
  P6OUT = P6OUT | output_mask;
}

void buzzer_on(char key)
{
  key = key - '0';
  // Initialize PWM output on P3.5, which corresponds to TB0.5
  P3SEL |= BIT5; // Select peripheral output mode for P3.5
  P3DIR |= BIT5;

  TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
  TB0CTL  &= ~TBIE;                       // Explicitly Disable timer interrupts for safety

  // Now configure the timer period, which controls the PWM period
  // Doing this with a hard coded values is NOT the best method
  // We do it here only as an example. You will fix this in Lab 2.
  TB0CCR0   = 128 * n;                    // Set the PWM period in ACLK ticks
  TB0CCTL0 &= ~CCIE;                  // Disable timer interrupts

  // Configure CC register 5, which is connected to our PWM pin TB0.5
  TB0CCTL5  = OUTMOD_7;                   // Set/reset mode for PWM
  TB0CCTL5 &= ~CCIE;                      // Disable capture/compare interrupts
  TB0CCR5   = TB0CCR0/2;                  // Configure a 50% duty cycle
}



void main()
{

  WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
  // You can then configure it properly, if desired


  // Initialize the MSP430
  initLeds();
  configDisplay();
  configKeypad();
  // Clear the display
  Graphics_clearDisplay(&g_sContext);

  // Initialize the game_state struct and variables
  int loop_num, i;
  char answer_key[32];
  state game_state = INIT;
  while(1) {
    switch(game_state) {
    case INIT: {
      // Display the starting text
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "SIMON", 5, 48, 15, TRANSPARENT_TEXT);
      Graphics_drawStringCentered(&g_sContext, "Press * to", 10, 48, 35, TRANSPARENT_TEXT);
      Graphics_drawStringCentered(&g_sContext, "start", 5, 48, 45, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Initialize the loop counter
      loop_num = 1;
      // Initialize the answer array to random characters 1, 2, 3, 4.
      for (i = 0; i < 32; i++) {
        // Generate random numbers, take the remainder, then add an offset to convert to the correct representation as char
        answer_key[i] = (rand() % 4) + '1';
      }
      // Wait in "Press any key to start"
      char ans =get_input();
      if (ans != '*')
        break;
      fake_delay(1);
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "3...", 4, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      fake_delay(1);
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "2...", 4, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      fake_delay(1);
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "1...", 4, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      fake_delay(1);
      game_state = OUTPUT;
      break;
    }
    case OUTPUT: {
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "Watch closely...", 16, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      set_leds(0);
      fake_delay(1);
      // Enter loop for N times
      for (i = 0; i < loop_num; i++) {
        // Blink LED
        set_leds(answer_key[i]);
        // Sound Buzzer
        buzzer_on(answer_key[i]);
        // WRITE A FUNCTION FOR SPECIFIC NOTES
        fake_delay(7);
        BuzzerOff();
        set_leds('0');
        fake_delay(4);
      }
      fake_delay(2);
      set_leds('0');
      game_state = WAIT_FOR_INPUT;
      break;
    }
    case WAIT_FOR_INPUT: {
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "now input", 9, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Enter loop for N times
      for (i = 0; i < loop_num; i++) {
        // Store user input
        char ans = get_input();
        char disp[2] = {ans, '\0'};
        Graphics_clearDisplay(&g_sContext); // Clear the display
        Graphics_drawStringCentered(&g_sContext, disp, 1, 48, 35, TRANSPARENT_TEXT);
        Graphics_flushBuffer(&g_sContext);
        // Set LEDs
        set_leds(ans);
        // Play buzzer
        buzzer_on(ans);
        fake_delay(1);
        BuzzerOff();
        if (ans != answer_key[i]) {
          fake_delay(1);
          game_state = INPUT_FALSE;
          break;
        }
      }
      if (game_state != INPUT_FALSE) {
        fake_delay(1);
        game_state = INPUT_TRUE;
      }
      break;
    }
    case INPUT_TRUE: {
      // Display Correct!
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "Correct!", 8, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Increment counter
      loop_num++;
      fake_delay(6);
      // Branch to OUTPUT
      game_state = OUTPUT;
      break;
    }
    case INPUT_FALSE: {
      // Display WRONG
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "WRONG!", 6, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Branch to GAME_OVER
      fake_delay(4);
      game_state = GAME_OVER;
      break;
    }
    case GAME_OVER: {
      set_leds('0'); // Clear LEDs
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "GAME OVER", 9, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      fake_delay(3);
      // Display Play again?
      Graphics_drawStringCentered(&g_sContext, "Play again?", 12, 48, 45, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Branch to INIT
      get_input();
      game_state = INIT;
      // Display Quit?
      // Exit
      // Timeout after 5 seconds
    }
    }
  }
}
