/*
	LCD5110_ggt - 
	Created by Glenn Gil Tamayo, February 11, 2019
*/

#include <Arduino.h>
#include "LCD5110_ggt.h"

LCD5110_ggt::LCD5110_ggt(){
	
}

void LCD5110_ggt::begin(byte _dcPin, byte _scePin, byte _rstPin, byte _blPin, byte _sdinPin, byte _sclkPin) {
  dcPin = _dcPin;
  scePin = _scePin;
  rstPin = _rstPin;
  blPin = _blPin;
  sdinPin = _sdinPin;
  sclkPin = _sclkPin;
	
  //Configure control pins
  pinMode(scePin, OUTPUT);
  pinMode(rstPin, OUTPUT);
  pinMode(dcPin, OUTPUT);
  pinMode(sdinPin, OUTPUT);
  pinMode(sclkPin, OUTPUT);
  pinMode(blPin, OUTPUT);

  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  //Reset the LCD to a known state
  digitalWrite(rstPin, LOW);
  digitalWrite(rstPin, HIGH);

  LCDWrite(LCD_COMMAND, 0x21); //Tell LCD extended commands follow
  LCDWrite(LCD_COMMAND, 0xB0); //Set LCD Vop (Contrast)
  LCDWrite(LCD_COMMAND, 0x04); //Set Temp coefficent
  LCDWrite(LCD_COMMAND, 0x14); //LCD bias mode 1:48 (try 0x13)
  //We must send 0x20 before modifying the display control mode
  LCDWrite(LCD_COMMAND, 0x20);
  LCDWrite(LCD_COMMAND, 0x0C); //Set display control, normal mode.
}

void LCD5110_ggt::LCDWrite(byte data_or_command, byte data)
{
  //Tell the LCD that we are writing either to data or a command
  digitalWrite(dcPin, data_or_command);

  //Send the data
  digitalWrite(scePin, LOW);
  SPI.transfer(data); //shiftOut(sdinPin, sclkPin, MSBFIRST, data);
  digitalWrite(scePin, HIGH);
  
}

void LCD5110_ggt::setPixel(int x, int y, boolean bw)
{
  // First, double check that the coordinate is in range.
  if ((x >= 0) && (x < LCD_WIDTH) && (y >= 0) && (y < LCD_HEIGHT))
  {
	byte shift = y % 8;

	if (bw) // If black, set the bit.
	  displayMap[x + (y/8)*LCD_WIDTH] |= 1<<shift;
	else   // If white clear the bit.
	  displayMap[x + (y/8)*LCD_WIDTH] &= ~(1<<shift);
  }
}

void LCD5110_ggt::setPixel(int x, int y)
{
  setPixel(x, y, BLACK); // Call setPixel with bw set to Black
}

void LCD5110_ggt::clearPixel(int x, int y)
{
  setPixel(x, y, WHITE); // call setPixel with bw set to white
}

void LCD5110_ggt::setLine(int x0, int y0, int x1, int y1, boolean bw)
{
  int dy = y1 - y0; // Difference between y0 and y1
  int dx = x1 - x0; // Difference between x0 and x1
  int stepx, stepy;

  if (dy < 0)
  {
	dy = -dy;
	stepy = -1;
  }
  else
	stepy = 1;

  if (dx < 0)
  {
	dx = -dx;
	stepx = -1;
  }
  else
	stepx = 1;

  dy <<= 1; // dy is now 2*dy
  dx <<= 1; // dx is now 2*dx
  setPixel(x0, y0, bw); // Draw the first pixel.

  if (dx > dy)
  {
	int fraction = dy - (dx >> 1);
	while (x0 != x1)
	{
	  if (fraction >= 0)
	  {
		y0 += stepy;
		fraction -= dx;
	  }
	  x0 += stepx;
	  fraction += dy;
	  setPixel(x0, y0, bw);
	}
  }
  else
  {
	int fraction = dx - (dy >> 1);
	while (y0 != y1)
	{
	  if (fraction >= 0)
	  {
		x0 += stepx;
		fraction -= dy;
	  }
	  y0 += stepy;
	  fraction += dx;
	  setPixel(x0, y0, bw);
	}
  }
}

void LCD5110_ggt::setRect(int x0, int y0, int x1, int y1, boolean fill, boolean bw)
{
  // check if the rectangle is to be filled
  if (fill == 1)
  {
	int xDiff;

	if(x0 > x1)
	  xDiff = x0 - x1; //Find the difference between the x vars
	else
	  xDiff = x1 - x0;

	while(xDiff > 0)
	{
	  setLine(x0, y0, x0, y1, bw);

	  if(x0 > x1)
		x0--;
	  else
		x0++;

	  xDiff--;
	}
  }
  else
  {
	// best way to draw an unfilled rectangle is to draw four lines
	setLine(x0, y0, x1, y0, bw);
	setLine(x0, y1, x1, y1, bw);
	setLine(x0, y0, x0, y1, bw);
	setLine(x1, y0, x1, y1, bw);
  }
}

void LCD5110_ggt::setCircle (int x0, int y0, int radius, boolean bw, int lineThickness)
{
  for(int r = 0; r < lineThickness; r++)
  {
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	setPixel(x0, y0 + radius, bw);
	setPixel(x0, y0 - radius, bw);
	setPixel(x0 + radius, y0, bw);
	setPixel(x0 - radius, y0, bw);

	while(x < y)
	{
	  if(f >= 0)
	  {
		y--;
		ddF_y += 2;
		f += ddF_y;
	  }
	  x++;
	  ddF_x += 2;
	  f += ddF_x + 1;

	  setPixel(x0 + x, y0 + y, bw);
	  setPixel(x0 - x, y0 + y, bw);
	  setPixel(x0 + x, y0 - y, bw);
	  setPixel(x0 - x, y0 - y, bw);
	  setPixel(x0 + y, y0 + x, bw);
	  setPixel(x0 - y, y0 + x, bw);
	  setPixel(x0 + y, y0 - x, bw);
	  setPixel(x0 - y, y0 - x, bw);
	}
	radius--;
  }
}

void LCD5110_ggt::setChar(char character, int x, int y, boolean bw)
{
  byte column; // temp byte to store character's column bitmap
  for (int i=0; i<5; i++) // 5 columns (x) per character
  {
	column = pgm_read_byte(&ASCII[character - 0x20][i]);
	for (int j=0; j<8; j++) // 8 rows (y) per character
	{
	  if (column & (0x01 << j)) // test bits to set pixels
		setPixel(x+i, y+j, bw);
	  else
		setPixel(x+i, y+j, !bw);
	}
  }
}

void LCD5110_ggt::setStr(char * dString, int x, int y, boolean bw)
{
  while (*dString != 0x00) // loop until null terminator
  {
	setChar(*dString++, x, y, bw);
	x+=5;
	if (*dString == '\n') //  NEWLINE
	{
	  x = 0;
	  y += 8;
	  *dString++;
	} else {
		for (int i=y; i<y+8; i++) {
		  setPixel(x, i, !bw);
		}
	}
	x++;
	if (x > (LCD_WIDTH - 5)) // Enables wrap around
	{
	  x = 0;
	  y += 8;
	}
  }
}

void LCD5110_ggt::setBitmap(const char * bitArray)
{
  for (int i=0; i<(LCD_WIDTH * LCD_HEIGHT / 8); i++)
  {
	char c = pgm_read_byte(&bitArray[i]);
	displayMap[i] = c;
  }
}

void LCD5110_ggt::clearDisplay(boolean bw)
{
  for (int i=0; i<(LCD_WIDTH * LCD_HEIGHT / 8); i++)
  {
	if (bw)
	  displayMap[i] = 0xFF;
	else
	  displayMap[i] = 0;
  }
}

void LCD5110_ggt::gotoXY(int x, int y)
{
  LCDWrite(0, 0x80 | x);  // Column.
  LCDWrite(0, 0x40 | y);  // Row.  ?
}

void LCD5110_ggt::updateDisplay()
{
  gotoXY(0, 0);
  for (int i=0; i < (LCD_WIDTH * LCD_HEIGHT / 8); i++)
  {
	LCDWrite(LCD_DATA, displayMap[i]);
  }
}

void LCD5110_ggt::setContrast(byte contrast)
{
  LCDWrite(LCD_COMMAND, 0x21); //Tell LCD that extended commands follow
  LCDWrite(LCD_COMMAND, 0x80 | contrast); //Set LCD Vop (Contrast): Try 0xB1(good @ 3.3V) or 0xBF if your display is too dark
  LCDWrite(LCD_COMMAND, 0x20); //Set display mode
}

void LCD5110_ggt::invertDisplay()
{
  /* Direct LCD Command option
  LCDWrite(LCD_COMMAND, 0x20); //Tell LCD that extended commands follow
  LCDWrite(LCD_COMMAND, 0x08 | 0x05); //Set LCD Vop (Contrast): Try 0xB1(good @ 3.3V) or 0xBF if your display is too dark
  LCDWrite(LCD_COMMAND, 0x20); //Set display mode  */

  /* Indirect, swap bits in displayMap option: */
  for (int i=0; i < (LCD_WIDTH * LCD_HEIGHT / 8); i++)
  {
	displayMap[i] = ~displayMap[i] & 0xFF;
  }
  updateDisplay();
}



/* This function serves as a fun demo of the graphics driver
functions below. */
void LCD5110_ggt::funTime()
{
  clearDisplay(WHITE); // Begin by clearing the display
  randomSeed(analogRead(A0));
  
  /* setPixel Example */
  const int pixelCount = 100;
  for (int i=0; i<pixelCount; i++)
  {
    // setPixel takes 2 to 3 parameters. The first two params
    // are x and y variables. The third optional variable is
    // a "color" boolean. 1 for black, 0 for white.
    // setPixel() with two variables will set the pixel with
    // the color set to black.
    // clearPixel() will call setPixel with with color set to
    // white.
    setPixel(random(0, LCD_WIDTH), random(0, LCD_HEIGHT));
    // After drawing something, we must call updateDisplay()
    // to actually make the display draw something new.
    updateDisplay();
    delay(10);
  }
  setStr("full of stars", 0, LCD_HEIGHT-8, BLACK);
  updateDisplay();
  delay(1000);
  // Seizure time!!! Err...demoing invertDisplay()
  for (int i=0; i<5; i++)
  {
    invertDisplay(); // This will swap all bits in our display
    delay(200);
    invertDisplay(); // This will get us back to where we started
    delay(200);
  }
  delay(2000);
  
  /* setLine Example */
  clearDisplay(WHITE); // Start fresh
  int x0 = LCD_WIDTH/2;
  int y0 = LCD_HEIGHT/2;
  for (float i=0; i<2*PI; i+=PI/8)
  {
    // Time to whip out some maths:
    const int lineLength = 24;
    int x1 = x0 + lineLength * sin(i);
    int y1 = y0 + lineLength * cos(i);
    
    // setLine(x0, y0, x1, y1, bw) takes five variables. The
    // first four are coordinates for the start and end of the 
    // line. The last variable is the color (1=black, 0=white).
    setLine(x0, y0, x1, y1, BLACK);
    updateDisplay();
    delay(100);
  }
  // Demo some backlight tuning
  for (int j=0; j<2; j++)
  {
    for (int i=255; i>=0; i-=5)
    {
      analogWrite(blPin, i); // blPin is ocnnected to BL LED
      delay(20);
    }
    for (int i=0; i<256; i+=5)
    {
      analogWrite(blPin, i);
      delay(20);
    }
  }
  
  /* setRect Example */
  clearDisplay(WHITE); // Start fresh
  
  // setRect takes six parameters (x0, y0, x1, y0, fill, bw)
  // x0, y0, x1, and y0 are the two diagonal corner coordinates
  // fill is a boolean, which determines if the rectangle is
  // filled in. bw determines the color 0=white, 1=black.
  for (int x=0; x<LCD_WIDTH; x+=8)
  { // Swipe right black
    setRect(0, 0, x, LCD_HEIGHT, 1, BLACK);
    updateDisplay();
    delay(10);
  }
  for (int x=0; x<LCD_WIDTH; x+=8)
  { // Swipe right white
    setRect(0, 0, x, LCD_HEIGHT, 1, WHITE);
    updateDisplay();
    delay(10);
  }
  for (int x=0; x<12; x++)
  { // Shutter swipe
    setRect(0, 0, x, LCD_HEIGHT, 1, 1);
    setRect(11, 0, x+12, LCD_HEIGHT, 1, BLACK);
    setRect(23, 0, x+24, LCD_HEIGHT, 1, BLACK);
    setRect(35, 0, x+36, LCD_HEIGHT, 1, BLACK);
    setRect(47, 0, x+48, LCD_HEIGHT, 1, BLACK);
    setRect(59, 0, x+60, LCD_HEIGHT, 1, BLACK);
    setRect(71, 0, x+72, LCD_HEIGHT, 1, BLACK);
    updateDisplay();
    delay(10);
  }
  // 3 Dee!
  setRect(25, 10, 45, 30, 0, WHITE);
  setRect(35, 20, 55, 40, 0, WHITE);
  setLine(25, 10, 35, 20, WHITE);
  setLine(45, 30, 55, 40, WHITE);
  setLine(25, 30, 35, 40, WHITE);
  setLine(45, 10, 55, 20, WHITE);
  updateDisplay();
  delay(2000);
  
  /* setCircle Example */
  clearDisplay(WHITE);
  // setCircle takes 5 parameters -- x0, y0, radius, bw, and
  // lineThickness. x0 and y0 are the center coords of the circ.
  // radius is the...radius. bw is the color (0=white, 1=black)
  // lineThickness is the line width of the circle, 1 = smallest
  // thickness moves in towards center.
  for (int i=0; i<20; i++)
  {
    int x = random(0, LCD_WIDTH);
    int y = random(0, LCD_HEIGHT);
    
    setCircle(x, y, i, BLACK, 1);
    updateDisplay();
    delay(100);
  }
  delay(2000);
  
  /* setChar & setStr Example */
  // setStr takes 4 parameters: an array of characters to print,
  // x and y coordinates for the top-left corner. And a color
  setStr("Modern Art", 0, 10, WHITE);
  updateDisplay();
  delay(2000);
  
  /* setBitmap Example */
  // setBitmap takes one parameter, an array of the same size
  // as our screen.
  setBitmap(xkcdSandwich);
  updateDisplay();
  
}

void LCD5110_ggt::backlight(int brightness) {
	analogWrite(blPin, brightness);
}

bool LCD5110_ggt::backlight(unsigned long duration, bool trigger) {
	unsigned long currentMillis = millis();
	static unsigned long previousMillis = 0;
	
	if(trigger == 1) {
		backlight(0);
		previousMillis = currentMillis;
	} else {
		if ((currentMillis - previousMillis) > duration) {
			backlight(255);
			previousMillis = currentMillis;
		}
	}
	
}

bool LCD5110_ggt::serialCom() {
	bool serialAvailable = 0;
	static int cursorX = 0;
	static int cursorY = 0;

	if (serialAvailable = Serial.available()) {
		char c = Serial.read();

		switch (c)
		{
		case '\n': // New line
		  
		  
		case '\r': // Return feed
		  cursorX = 0;
		  cursorY += 8;
		  break;
		case '~': // Use ~ to clear the screen.
		  clearDisplay(WHITE);
		  updateDisplay();
		  cursorX = 0; // reset the cursor
		  cursorY = 0;
		  break;
		default:
		  setChar(c, cursorX, cursorY, BLACK);
		  updateDisplay();
		  cursorX += 6; // Increment cursor
		  break;
		}
		// Manage cursor
		if (cursorX >= (LCD_WIDTH - 4)) 
		{ // If the next char will be off screen...
		  cursorX = 0; // ... reset x to 0...
		  cursorY += 8; // ...and increment to next line.
		  if (cursorY >= (LCD_HEIGHT - 7))
		  { // If the next line takes us off screen...
			cursorY = 0; // ...go back to the top.
			clearDisplay(WHITE);
		  }
		}
		if (cursorY >= (LCD_HEIGHT - 7)) { // If the next line takes us off screen...
			cursorY = 0; // ...go back to the top.
			clearDisplay(WHITE);
		}
	}
	//Serial.println(serialAvailable);
	return serialAvailable;
	
}