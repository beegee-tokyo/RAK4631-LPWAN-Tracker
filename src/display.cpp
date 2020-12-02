/**
 * @file display.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Display functions
 * @version 0.1
 * @date 2020-07-19
 * 
 * @copyright Copyright (c) 2020
 * 
 * @note Writing to the display is done by adding new lines
 * to the display line buffer. If all available display lines
 * are used up, the display is scrolled up and the new line
 * is added at the bottom
 */
#include "main.h"

/** Line buffer for messages */
char buffer[NUM_OF_LINES+1][32] = {0};

/** Current line used */
uint8_t currentLine = 0;

/** Display class */
SSD1306Wire display(0x3c, PIN_WIRE_SDA, PIN_WIRE_SCL, GEOMETRY_128_64);

/**
 * @brief Initialize the display
 */
void initDisplay(void)
{
	delay(500); // Give display reset some time
	taskENTER_CRITICAL();
	display.setI2cAutoInit(true);
	display.init();
	display.displayOff();
	display.clear();
	display.displayOn();
	display.flipScreenVertically();
	display.setContrast(128);
	display.setFont(ArialMT_Plain_10);
	display.display();
	taskEXIT_CRITICAL();
}

/**
 * @brief Write the top line of the display
 */
void dispWriteHeader(void)
{
	taskENTER_CRITICAL();
	display.setFont(ArialMT_Plain_10);

	// clear the status bar
	display.setColor(BLACK);
	display.fillRect(0, 0, OLED_WIDTH, STATUS_BAR_HEIGHT);

	display.setColor(WHITE);
	display.setTextAlignment(TEXT_ALIGN_LEFT);

	display.drawString(0, 0, "RAK4631 LoRaWan OTAA");

	// draw divider line
	display.drawLine(0, 11, 128, 11);
	display.display();
	taskEXIT_CRITICAL();
}

/**
 * @brief Add a line to the display buffer
 * 
 * @param line Pointer to char array with the new line
 */
void dispAddLine(char *line)
{
	taskENTER_CRITICAL();
	if (currentLine == NUM_OF_LINES)
	{
		// Display is full, shift text one line up
		for (int idx = 0; idx < NUM_OF_LINES; idx++)
		{
			memcpy(buffer[idx], buffer[idx + 1], 32);
		}
		currentLine--;
	}
	snprintf(buffer[currentLine], 32, "%s", line);

	if (currentLine != NUM_OF_LINES)
	{
		currentLine++;
	}

	dispShow();
	taskEXIT_CRITICAL();
}

/**
 * @brief Update display messages
 * 
 */
void dispShow(void)
{
	display.setColor(BLACK);
	display.fillRect(0, STATUS_BAR_HEIGHT + 1, OLED_WIDTH, OLED_HEIGHT);

	display.setFont(ArialMT_Plain_10);
	display.setColor(WHITE);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	for (int line = 0; line < currentLine; line++)
	{
		display.drawString(0, (line * LINE_HEIGHT) + STATUS_BAR_HEIGHT + 1, buffer[line]);
	}
	display.display();
}
