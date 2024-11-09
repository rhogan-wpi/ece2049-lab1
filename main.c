#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"
typedef enum{
  INIT,
  OUTPUT,
  WAIT_FOR_INPUT,
  CHECK_INPUT,
  INPUT_TRUE,
  INPUT_FALSE,
  GAME_OVER,
} state;
// Test diff
void fake_delay(int seconds)
{
  int n = 0;
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
    output_mask = output_mask | BIT4;
  if (key == 2)
    output_mask = output_mask | BIT3;
  if (key == 3)
    output_mask = output_mask | BIT1;
  if (key == 4)
    output_mask = output_mask | BIT2;
  P6OUT = P6OUT | output_mask;
}

void main()
{
  //REQUIRED variables
  unsigned char currKey=0, dispSz = 3;
  unsigned char dispThree[3];

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
  char answer_key[16];
  state game_state = INIT;
  while(1) {
    switch(game_state) {
    case INIT: {
      // Display the starting text
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "SIMON", 5, 48, 15, TRANSPARENT_TEXT);
      Graphics_drawStringCentered(&g_sContext, "Press any key", 15, 48, 35, TRANSPARENT_TEXT);
      Graphics_drawStringCentered(&g_sContext, "to start", 8, 48, 45, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Initialize the loop counter
      loop_num = 1;
      // Initialize the answer array to random characters 1, 2, 3, 4.
      for (i = 0; i < 16; i++) {
        // Generate random numbers, take the remainder, then add an offset to convert to the correct representation as char
        answer_key[i] = (rand() % 4) + '1';
      }
      // Wait in "Press any key to start"
      get_input();
      fake_delay(1);
      game_state = OUTPUT;
      break;
    }
    case OUTPUT: {
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "Watch closely...", 16, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Enter loop for N times
      for (i = 0; i < loop_num; i++) {
        // Blink LED
        set_leds(answer_key[i]);
        // Sound Buzzer
        // WRITE A FUNCTION FOR SPECIFIC NOTES
        fake_delay(6);
        set_leds('0');
        fake_delay(3);
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
        // Set LEDs
        set_leds(ans);
        fake_delay(1);
        // Play buzzer
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
