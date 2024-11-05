#include <stdlib.h>
#include <msp430.h>
#include "peripherals.h"
//Test diff
typedef enum{
  INIT,
  OUTPUT,
  WAIT_FOR_INPUT,
  CHECK_INPUT,
  INPUT_TRUE,
  INPUT_FALSE,
  GAME_OVER,
} state;

void fake_delay(int seconds)
{
  int n = 0;
  while (n < (seconds * 650)) {
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

void main()
{
  // Initialize the MSP430
  initLeds();
  configDisplay();
  configKeypad();

  // Initialize the game_state struct and variables
  int loop_num;
  char answer_key[16];
  state game_state = INIT;
  while(1) {
    switch(game_state) {
    case INIT: {
      // Display the starting text
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "SIMON", 5, 48, 15, TRANSPARENT_TEXT);
      Graphics_drawStringCentered(&g_sContext, "Press any key to start", 24, 48, 35, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Initialize the loop counter
      loop_num = 1;
      // Initialize the answer array to random characters 1, 2, 3, 4.
      for (int i = 0; i < 16; i++) {
        // Generate random numbers, take the remainder, then add an offset to convert to the correct representation as char
        answer_key[i] = (rand() % 4) + '1';
      }
      // Wait in "Press any key to start"
      get_input();
      game_state = OUTPUT;
      break;
    }
    case OUTPUT: {
      // Enter loop for N times
      for (int i = 0; i < loop_num; i++) {
        // Blink LED
        setLeds(answer_key[i] - '0');
        // Sound Buzzer
        // WRITE A FUNCTION FOR SPECIFIC NOTES
        fake_delay(2);
      }
      game_state = WAIT_FOR_INPUT;
      break;
    }
    case WAIT_FOR_INPUT: {
      // Enter loop for N times
      for (int i = 0; i < loop_num; i++) {
        // Store user input
        char ans = get_input();
        // Set LEDs
        setLeds(ans - '0');
        // Play buzzer
        if (ans != answer_key[i])
          game_state = INPUT_FALSE;
      }
      game_state = INPUT_TRUE;
      break;
    }
    case INPUT_TRUE: {
      // Display Correct!
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "Correct!", 8, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Increment counter
      loop_num++;
      fake_delay(1);
      // Branch to WAIT_FOR_INPUT
      game_state = WAIT_FOR_INPUT;
      break;
    }
    case INPUT_FALSE: {
      // Display WRONG
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "WRONG!", 6, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      // Branch to GAME_OVER
      fake_delay(1);
      game_state = GAME_OVER;
      break;
    }
    case GAME_OVER: {
      Graphics_clearDisplay(&g_sContext); // Clear the display
      Graphics_drawStringCentered(&g_sContext, "GAME OVER", 9, 48, 15, TRANSPARENT_TEXT);
      Graphics_flushBuffer(&g_sContext);
      fake_delay(1);
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
