// 
// a6d82bced638de3def1e9bbb4983225c
// 

#include "Glx_SWindows.h"
static uint16_t data[30000];

// Bodmers BMP image rendering function

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(fs::File &f) {
	uint16_t result;
	((uint8_t *)&result)[0] = f.read(); // LSB
	((uint8_t *)&result)[1] = f.read(); // MSB
	return result;
}

uint32_t read32(fs::File &f) {
	uint32_t result;
	((uint8_t *)&result)[0] = f.read(); // LSB
	((uint8_t *)&result)[1] = f.read();
	((uint8_t *)&result)[2] = f.read();
	((uint8_t *)&result)[3] = f.read(); // MSB
	return result;
}


//====================================================================================
// This is the function to draw the icon stored as an array in program memory (FLASH)
//====================================================================================

// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64

// Draw array "icon" of defined width and height at coordinate x,y
// Maximum icon size is 255x255 pixels to avoid integer overflow

void drawIcon(const unsigned short* icon, int16_t x, int16_t y, uint16_t width, uint16_t height) {

	uint16_t  pix_buffer[BUFF_SIZE];   // Pixel buffer (16 bits per pixel)

									   // Set up a window the right size to stream pixels into
	tft.setWindow(x, y, x + width - 1, y + height - 1);

	// Work out the number whole buffers to send
	uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

	// Fill and send "nb" buffers to TFT
	for (int i = 0; i < nb; i++) {
		for (int j = 0; j < BUFF_SIZE; j++) {
			pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
		}
		tft.pushColors(pix_buffer, BUFF_SIZE);
	}

	// Work out number of pixels not yet sent
	uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

	// Send any partial buffer left over
	if (np) {
		for (int i = 0; i < np; i++) pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
		tft.pushColors(pix_buffer, np);
	}
}

void drawBmp(const char *filename, int16_t x, int16_t y) {

	if ((x >= tft.width()) || (y >= tft.height())) return;

	fs::File bmpFS;

	// Open requested file on SD card
	bmpFS = SPIFFS.open(filename, "r");

	if (!bmpFS) {
		Serial.print("File not found");
		return;
	}
	uint32_t seekOffset;
	uint16_t w, h, row, col;
	uint8_t  r, g, b;

	uint32_t startTime = millis();
	uint16_t primo;
	do {
	primo= read16(bmpFS);
	Serial.println(primo, HEX);
	} while (primo != 0x4D42);
	if ( primo== 0x4D42) {
		read32(bmpFS);
		read32(bmpFS);
		seekOffset = read32(bmpFS);
		read32(bmpFS);
		w = read32(bmpFS);
		h = read32(bmpFS);

		if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0)) {
			y += h - 1;

			tft.setSwapBytes(true);
			bmpFS.seek(seekOffset);

			uint16_t padding = (4 - ((w * 3) & 3)) & 3;
			uint8_t lineBuffer[w * 3 + padding];
			for (row = 0; row < h; row++) {

				bmpFS.read(lineBuffer, sizeof(lineBuffer));
				uint8_t*  bptr = lineBuffer;
				uint16_t* tptr = (uint16_t*)lineBuffer;
				// Convert 24 to 16 bit colours
				for (uint16_t col = 0; col < w; col++) {
					b = *bptr++;
					g = *bptr++;
					r = *bptr++;
					*tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
				}

				// Push the pixel row to screen, pushImage will crop the line if needed
				// y is decremented as the BMP image is drawn bottom up
				tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
			}
			Serial.print("Loaded in "); Serial.print(millis() - startTime);
			Serial.println(" ms");
		} else
			Serial.println("BMP format not recognized.");
	} else Serial.println(" Format not good");
	bmpFS.close();
}

void Glx_GWindowsClass::Graf::init( uint16_t col)
{
	if (ymax == 0 && ymin == 0)
	{
		xmax = x[0]; xmin = x[0]; ymax = YSign*y[0]; ymin = YSign*y[0];
	}
	color = col;
	for (byte i = 0; i < nval; i++)
	{
		if( x [ i]>xmax)xmax= x[i];
		if (x[i]<xmin)xmin = x[i];
		if (y[i]*YSign>ymax)ymax = YSign*y[i];
		if (y[i] * YSign<ymin)ymin = YSign*y[i];
		
	}
	windowsW = winXmax - winXmin;
	
	x0 = xmin;
	y0 = ymin;
	windowsH = winYmax - winYmin;
	scay =  windowsH/(ymax-ymin);
	// serial.println(x0);
	Serial.println(y0);
	Serial.println(scay);
	Serial.println(winYmin);
	scax =   windowsW/(xmax - xmin);
	// serial.println(scax);
	Serial.println(ymax);
	//Serial.println(ymin);
}

boolean Glx_GWindowsClass::Graf::draw()
{
	

	//tft.drawPixel(int(x[0] * scax) - x0, int(y[0] * scay) - y0, color);
	for (int i = 1; i < nval; i++)
		//if (x[i] >winXmin && x[i] < winXmax )
			tft.drawLine(int((x[i - 1]  - x0)*scax+winXmin), 
						int((YSign*y[i - 1] - ymin)*scay+winYmin),
						int((x[i ] - x0)*scax+winXmin),
						int((YSign*y[i] - ymin)*scay + winYmin),  color);
	return true;
	
}

boolean Glx_GWindowsClass::Graf::drawAxX(float y,float xstep)
{
//	tft.drawPixel(Glx_GWindowsClass::winXmin, y*scay-y0, color);
	tft.drawFastHLine(winXmin,(y- ymin)*scay +winYmin,
		winXmax- winXmin, color);
	byte ntics = (xmax - xmin) / xstep;
	int i = 1;
	
	if (xmax - xmin > 1)
		while (ntics > 10) {
			xstep = xstep*i++;
			ntics = (xmax - xmin) / xstep;
			
		}
	   // serial.println(xstep);
	for (byte i = 0; i < ntics; i++) {
		int xp = (i*xstep+xmin-x0)*scax+winXmin;
		tft.drawFastVLine(xp , (y - ymin)*scay+winYmin - TICKSIZE, 2*TICKSIZE, color);
		tft.setTextColor(ILI9341_BLACK);
		tft.drawFloat(i*xstep+xmin,1, xp-6,int(( y - ymin)*scay+winYmin)+1, 1);
}
	return true;
}
void Glx_GWindowsClass::Graf::changeScaX(float dx) {
	scax = scax*dx;
	tft.fillRect(winXmin, winYmin, winXmax - winXmin, winYmax - winYmin, backgroundColor);
	
}
void Glx_GWindowsClass::Graf::scroll(float dx)
{
	x0 += (xmax-xmin)/dx;
	tft.fillRect(winXmin, winYmin, winXmax - winXmin, winYmax - winYmin, backgroundColor);
}
boolean Glx_GWindowsClass::Graf::drawAxX(float  y,float xstep,byte ty)  //yaxis at screen pos 0 for left 320 for right
{   
	byte ntics = (xmax - xmin) / xstep;
	tft.drawFastHLine(winXmin, (y - ymin)*scay + winYmin, winXmax - winXmin, color);

	float substep;
	int i, j = 0;
	if (ty == TIME_SCA) {
		int Xwind = winXmax - winXmin;
		if (Xwind/scax > 1440) { xstep = 120; substep = 30; }
		if (Xwind/scax > 720) { xstep = 60; substep = 10; }
		if (Xwind/scax > 240) { xstep = 30; substep = 5; }
		if (Xwind/scax > 120) { xstep = 10; substep = 2; }
		if (Xwind/scax > 60) { xstep = 5; substep = 1; }

	}
	else {
		float divisor[3] = { 1,2,5 };
		substep = 0;
		for (int j = -10; j < 10; j++)
		{
			for (byte k = 0; k < 3; k++)
				if ((winXmax - winXmin) / scax < pow(10., j)*divisor[k])
				{
					//if (k > 0)xstep = pow(10, j)*divisor[k - 1];
					//else xstep = pow(10, j - 1)*divisor[2];
					xstep = pow(10, j - 1)*divisor[k];
					substep = xstep*0.2; break;
				}
			if (substep != 0)break;
		}
	}
	ntics = (xmax - xmin) / substep;
//	Serial.print((winXmax-winXmin)/scax); Serial.print(' '); Serial.println(substep);
//	Serial.print(xstep); Serial.print(' '); Serial.println(substep);
// serial.println(xstep);
for (byte i = 0; i < ntics; i++) {
	int xp = (i*substep + xmin - x0)*scax + winXmin;
	tft.drawFastVLine(xp,(y - ymin)*scay + winYmin - TICKSIZE,  2 * TICKSIZE, color);
	tft.setTextColor(ILI9341_BLACK);
//	Serial.println(i*substep/xstep);
	if (i*substep/xstep-int(i*substep/xstep)== 0) {
		int ivalue(i*substep + xmin); if (ty == TIME_SCA)ivalue = ivalue / 60;
		tft.drawFastVLine(xp,(y - ymin)*scay + winYmin - TICKSIZEL,  2 * TICKSIZEL, color);
		tft.drawNumber(ivalue, xp - 6, int((y - ymin)*scay + winYmin), 1);
	}

	}
	return boolean();
}

boolean Glx_GWindowsClass::Graf::drawAxy(int x_pos, float step, byte ty)
{
	byte ntics = (xmax - xmin) / step;

	float substep;
	int i, j = 0;

	//if (ty == TIME_SCA) {
	//	if ((xmax - xmin)*scay > 1440) { step = 60; substep = 30; }
	//	if ((xmax - xmin)*scay > 720) { step = 60; substep = 10; }
	//	if ((xmax - xmin)*scay > 240) { step = 60; substep = 5; }
	//	if ((xmax - xmin)*scay > 120) { step = 60; substep = 1; }
	//	if ((xmax - xmin)*scay > 60)xstep = 1;
	//}
	//else {
		for (int j = -10; j < 10; j++)
			if ((ymax - ymin)*scay <pow (10.,j) ){ step = pow(10.,j )/2; substep = step*.2; break; }
	//}
	ntics = (ymax - ymin)*scay / substep;
	

	// serial.println(xstep);
	for (byte i = 0; i < ntics; i++) {
		int yp = (i*substep + ymin - y0)*scay + winYmin;
		tft.drawFastHLine(LegXmargin-TICKSIZE,yp, TICKSIZE, color);
		tft.setTextColor(ILI9341_BLACK);
		if (int(i*substep) % int(step) == 0) {
			tft.drawFastHLine(LegXmargin-TICKSIZEL, yp,  TICKSIZEL, color);
			tft.drawFloat(i*step + xmin, 1,LegXmargin-3, yp -8, 1);
		}

	}
	return boolean();
}

void Glx_GWindowsClass::title(String title, uint16_t col, int font) {
	drawIcon(myIcon, winXmin, winYmin, myIconWidth, myIconHeight);
	if (font > 0) {
		tft.setTextFont(font);
		
	}
	else {
#ifdef TITLE_FONT 
		tft.setFreeFont(TITLE_FONT);
#endif
	}
	Htitle = 10 + tft.fontHeight(font);
	if (myIconHeight > Htitle)Htitle = myIconHeight;
	tft.setTextDatum(CC_DATUM);
	tft.setTextColor(col);
	tft.drawString(title, myIconWidth+(winXmax - myIconWidth) / 2 + winXmin,winYmin+ Htitle/2);
#ifdef TITLE_FONT 
//	tft.setFreeFont();
	tft.setTextFont(1);
#endif
	


}


void Glx_GWindowsClass::init( int x0, int y0, int x1, int y1,uint16_t col_back)
{
	{ backgroundColor = col_back;
	Serial.printf("background col=%d\r\n", backgroundColor);
	winXmax = x1;// -xMargin;
	winXmin = x0;// +xMargin + LegXmargin;
	winYmax = y1;// -yMargin - LegYmargin;
	winYmin = y0;// +yMargin;
		tft.fillRect(x0, y0, x1 - x0, y1 - y0, col_back);
	}
}

int Glx_GWindowsClass::xpressed(int x,int y)
{
	if (y>winYmin && y<winYmax) 
return		  (x - winXmin) / scax + xmin;
	else return -32000;
}


int Glx_GWindowsClass::ypressed(int x, int y)
{
	if (y>winYmin && y<winYmax)
		return		  (y - winYmin) / scay +ymin;
	else return -32000;
}

Glx_List Glx_GWindowsClass::list(byte size) {

	return Glx_List(size);
	//  winXmin, winYmin + Htitle, winYmax);
}


void Glx_MWindowsClass::init(int x0, int y0, int x1, int y1)

	{
		Xmax = x1;
		Xmin = x0;
		Ymax = y1;
		Ymin = y0; 
}

int Glx_MWindowsClass::getPressed(uint16_t x,uint16_t y)
{	//Serial.print(x); Serial.print('_'); Serial.println(y);
// ----------------------------check if is a button-----------------------
	for (byte i = 0; i < menu[curr_m].nbutton; i++)
		
	{
		menu[curr_m].button[i].press(menu[curr_m].button[i].contains(x, y));
		if (menu[curr_m].button[i].isPressed())
		{
			menu[curr_m].button[i].drawButton(true);
			
				while (touchIsTouching()){}
				{  menu[curr_m].button[i].press(false);
					menu[curr_m].button[i].drawButton();

					if (menu[curr_m].menuIndex[i] == 0) {
						if (menu[curr_m].handlerF[i] != NULL)menu[curr_m].handlerF[i]();
						return (i + curr_m * 10 + 1);
					}
					else
					{
						byte prev_menu = curr_m;
						curr_m = menu[curr_m].menuIndex[i] - 1;
						menu[curr_m].init();
						menu[curr_m].draw();
						if (menu[prev_menu].handlerF[i] != NULL)menu[prev_menu].handlerF[i]();
						return -curr_m;
					}
				}
		}
		
	}
	//-------no button pressed-----------------------------------------------
	i_sel = -1;
	return 0;
}

void Glx_MWindowsClass::handleMenus() {
	char c;

	uint16_t x, y;
#ifdef ESP8266
	if (touch.isTouching()) {
		touch.getPosition(x, y);

#else
	if (tft.getTouch(&x, &y)) {
		x = tft.width() - x;
		y = tft.height() - y;
#endif
		getPressed(x, y);
	}
}

void Glx_MWindowsClass::Menu::init() {
	int WindowsW = Xmax - Xmin;
	int WindowsH = Ymax - Ymin;
	byte buttonW = WindowsW / nbutton;
	byte buttonH =  WindowsH;

	//char buf[12];
	//Serial.println(nbutton);
	for (byte i = 0; i < nbutton; i++) {
		// strcpy(buf , menuName[i]);
		button[i].initButton(&tft,  i*buttonW+buttonW/2,Ymin+buttonH/2, buttonW, buttonH,
			ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
			menuName[i], 1);

	}
}

void Glx_MWindowsClass::Menu::draw()
{
	
	for (byte i = 0; i < nbutton; i++) {
		button[i].drawButton();
	}
}

int Glx_keypad::init(int x, int y, int de) {

	byte buttonW = KEYP_BUT_W;
	byte buttonH = KEYP_BUT_H;
	width = (buttonW + 4) * 4 + 10;
	height = 30 + (buttonH + 4) * 4;

	x0 = x; y0 = y; def = de;
	tft.readRect(x0, y0, width, height, data);
	tft.fillRect(x0, y0, width, height, TFT_WHITE);

	tft.drawNumber(def, x0 + 20, y0 + 10, 1);
	for (byte j = 0; j < 3; j++)
		for (byte i = 0; i < 4; i++) {
			char c[2] = { 0,0 };
			byte n = i + j * 4;
			if (n <= 9)
				c[0] = '0' + (n + 1) % 10;
			if (n == 10)
				c[0] = '-';
			if (n == 11)
				c[0] = '.';

			button[j * 4 + i].initButton(&tft,
				i*(buttonW + 4) + x0 + buttonW / 2 + 2,
				j*(buttonH + 4) + y0 + 30 + buttonH / 2,
				buttonW, buttonH,
				ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				c, 1);
			button[j * 4 + i].drawButton();
		}
	button[12].initButton(&tft,
		10 + x0 + buttonW,
		3 * (buttonH + 4) + y0 + 30 + buttonH / 2,
		2 * buttonW, buttonH,
		ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
		"OK", 1);
	button[13].initButton(&tft,
		20 + 3 * buttonW + x0,
		3 * (buttonH + 4) + y0 + 30 + buttonH / 2,
		2 * buttonW, buttonH,
		ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
		"exit", 1);
	button[12].drawButton();
	button[13].drawButton();
}
int Glx_keypad::input( int maxval) {

byte B_ind = 0;
	byte ii=0;
	char c[10];
	while (B_ind < 12) {
		uint16_t x = 0, y = 0;
		tft.getTouch(&x, &y);
		x = tft.width() - x;
		y = tft.height() - y;
		for (byte j = 0; j < 14; j++){
				button[j].press(button[j].contains(x, y));

				if (button[j].isPressed()) {
			
					B_ind = j;
					button[B_ind].drawButton(true);

					while (touchIsTouching()) {}
					
					if (B_ind < 10) c[ii++] = '0' + (B_ind+1)%10;
					else if (B_ind == 10) c[ii++] = '-';
					else if (B_ind == 11) c[ii++] = '.';
					c[ii] = 0;
					if (maxval != 0 && atoi(c) > maxval) {
						Serial.println(maxval);
						tft.setTextColor(TFT_YELLOW,TFT_RED); ii = 0;
					} else
						tft.setTextColor(TFT_BLACK, TFT_YELLOW);
					tft.drawString(c, x0 + 20, y0 + 10);
					button[B_ind].press(false);
					button[B_ind].drawButton();
			
				//	else  
				//		tft.drawString("            ", x0 + 200, y0 + 10);
				}
			}
		
	}
	tft.pushRect(x0, y0, width, height, data);
	if (B_ind == 12)
		return atoi(c);
	else
		return def;
}
void Glx_keypad::inputTime( time_t current) {

	byte B_ind = 0;
	byte ii = 0;
	//char c[20];
	//strcpy(c, cc);
	tft.setTextColor(TFT_BLACK, TFT_WHITE);

	tft.drawString("Enter Date", x0 + 20, y0 + 5);
	sprintf(c, "%2d:%2d:%2d %2d-%2d-%2d",hour(current),minute(current),second(current),day(current),month(current),year(current));
	//for(byte jj = 0; jj < 6;jj++)
	tft.drawString(c, x0 + 20, y0 + 20);

	Serial.println(c);
	while (B_ind < 12) {
		uint16_t x = 0, y = 0;
		tft.getTouch(&x, &y);
		x = tft.width() - x;
		y = tft.height() - y;
		for (byte j = 0; j < 14; j++) {
			button[j].press(button[j].contains(x, y));

			if (button[j].isPressed()) {

				B_ind = j;
				button[B_ind].drawButton(true);

				while (touchIsTouching()) {}
				if (B_ind == 12)ii++;
				if (B_ind < 10) c[ii++] = '0' + (B_ind + 1) % 10;
				if(ii<17) if ((ii+1) % 3 == 0)ii++;
				//			else if (B_ind == 10) c[ii++] = '-';
				//			else if (B_ind == 11) c[ii++] = '.';
				//			c[ii] = 0;
				//			if (maxval != 0 && atoi(c) > maxval) {
				//				Serial.println(maxval);
				//				tft.setTextColor(TFT_YELLOW, TFT_RED); ii = 0;
				//			} else
				//	tft.drawString(c, x0 + 20, y0 + 10);
//				Serial.println(c);
				tft.setCursor(x0 + 20, y0 + 20);
				for (byte jj = 0; jj < strlen(c); jj++) {
                	if (jj == ii)	tft.setTextColor(TFT_BLACK, TFT_YELLOW);
					else			tft.setTextColor(TFT_BLACK, TFT_WHITE);
					tft.write(c[jj]);
				}
				button[B_ind].press(false);
				button[B_ind].drawButton();

				//	else  
				//		tft.drawString("            ", x0 + 200, y0 + 10);
			}
		}

	}
	tft.pushRect(x0, y0, width, height, data);
//	if (B_ind == 12 ) {
//		Serial.println(c);
//		return c;
//	}
//	else
//		return c;
	Serial.println(c);
	//strcpy(cc, c);
}

void Glx_keyborad::init(byte uppercase)
{
/*	line[0][0] = "1234567890";		line[0][1] = "!\"?$%&/()=";
	line[1][0] = "qwertyuiop";		line[1][1] = "QWERTYUIOP";
	line[2][0] = "asdfghjkl+";		line[2][1] = "ASDFGHJKL*";
	line[3][0] = "zxcvbnm,.-";		line[3][1] = "ZXCVBNM;:_";
*/
	LastLine[0] = "<";
	LastLine[1] = "shift";
	LastLine[2] = "Space";
	LastLine[3] = "#";
	LastLine[4] = "'";
	LastLine[5] = "ret";

	 int x0 = 15;
	 byte buttonW = tft.width()/11;
	 byte buttonH = KEYB_BUT_H;
	 int y0 = _ypos;
		 tft.fillRect(0, _ypos - 30, tft.width(), y_size, TFT_WHITE);
	 for (byte j = 0; j < 4;j++)
		 for (byte i = 0; i < 10; i++)
		 {
			 char c[2];
			  c[0] = line[j][uppercase][i];
			  c[1] = 0;
			 button[j * 10 + i].initButton(&tft, 
				 i*(buttonW+SP4) + x0,
				 j*(buttonH+SP4)+y0, 
				 buttonW, buttonH,
				 ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				 c, 1);
			 button[j * 10 + i].drawButton();
		 }
	 int px = x0;
	 for (byte i = 0; i < 6; i++)
	 {   
		 byte WW = buttonW;
		 if (i == 1 || i == 2 || i == 5) WW = buttonW + 20;
		 button[40 + i].initButton( &tft,
			 px,
			 4 * (buttonH + SP4) + y0,
			 WW, buttonH,
			 ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
			 LastLine[i], 1);
		 px += WW +15;
		// Serial.println(px, DEC);
		 button[40 + i].drawButton();
	 }
}



char Glx_keyborad::isPressed(uint16_t x, uint16_t y)

{	char key[6] = { '<', ' ', ' ', '#', '\'', 13 };

	for (byte j = 0; j < 5;j++)
	for (byte i = 0; i < (j==4?6:10); i++)
	{     
		button[i + j * 10].press(button[i+j*10].contains(x, y));
		
		if (button[i + j * 10].isPressed())
		{
			//Serial.print('p');
			button[i + j * 10].drawButton(true);
	
			while (touchIsTouching()) {}
			{
				if (j < 4) {
					button[i + j * 10].press(false);
					button[i + j * 10].drawButton();
					Serial.printf("i= %d j=%d c=%c \r\n", i, j ,line[j][CaseStatus][i]);
					return (char)line[j][CaseStatus][i];
				}
				else {
					if (i == 1)
					{
						if (CaseStatus == 0) {
							CaseStatus = 1;
							init( 1);
							return 0;
						}
						else {
							CaseStatus = 0;
							init( 0);
							return 0;
						}
					}
					else
					{
						button[i +  40].press(false);
						button[i + 40].drawButton();

						return key[i];
					}
					}

			}
		}

	}
	//-------no button pressed-----------------------------------------------
	
	return 0;

}

bool Glx_keyborad::getChar() {
	uint16_t x, y, i = 0; char c;
	tft.setTextColor(TFT_BLACK);
	do {
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
			c = isPressed(x, y);
		
			if (c > 0) {
				Serial.println((byte)c);
				if (c != '<') {
				
					retChar[i++] = c;
					if (c == 13)retChar[i++] = 10;
				}
				else {
					i--;
					tft.fillRect(0, _ypos - 30, tft.width(), 10, TFT_WHITE);
				}
				retChar[i] = 0;
			//	Serial.println(retChar);
				tft.setTextColor(TFT_BLACK);
				
				tft.drawString(retChar, 20, _ypos - 30);
			}
		}
	}
	while (c != 13);

		return true;
}

void Glx_keyborad::end()
{
	tft.fillRect(0, _ypos-30, tft.width(),y_size, TFT_BLACK);
}
void Glx_TWindows::charPos(byte nline, byte nchar,byte mode) //y pos, x pos , mode >0 clean mode length to right border
{
	tft.setCursor(nchar, nline+yS);
	
	if (mode>0)	tft.fillRect(0,nline+yS,mode, 10, ILI9341_BLACK); //char heigth 10 pix
}
void Glx_TWindows::init(int x0, int y0, int x1, int y1,byte textH, byte mode)
{
	yS = y0+1;
	nLines = (y1 - y0) / textH-1;
	_fontH = textH;
	_x0 = x0 + 2;
	_x1=x1;
	_y1 = y1;
	tft.fillRect(x0, y0, x1 - x0, y1 - y0, TFT_BLACK);
	tft.drawRect(x0, y0, x1 - x0, y1- y0, TFT_CYAN);
	tft.setCursor(_x0, yS);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	stc[0] = 0;


#ifdef SCROLL_HW
	if (mode == 0) {
		TEXT_HEIGHT = textH;// Height of text to be printed and scrolled
		ScreenH = tft.height();
		BOT_FIXED_AREA = ScreenH - y1; // Number of lines in bottom fixed area (lines counted from bottom of screen)
		TOP_FIXED_AREA = y0;           // Number of lines in top fixed area (lines counted from top of screen)
		yStart = TOP_FIXED_AREA;
		yDraw = ScreenH - BOT_FIXED_AREA - TEXT_HEIGHT;

		// Keep track of the drawing x coordinate
		//	tft.fillRect(x0, y0, x1, y1, ILI9341_BLUE);
		tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
		for (byte i = 0; i < 100; i++) blank[i] = 0;
		setupScrollArea(TOP_FIXED_AREA,BOT_FIXED_AREA);
		Scroll = true;
	}
	else
#endif
		Scroll = false;
}
void Glx_TWindows::redraw(int ii) {
	tft.fillRect(_x0 - 2, yS - 1, _x1 - _x0 + 2, _y1 - yS + 1, TFT_BLACK);
	tft.drawRect(_x0 - 2, yS - 1, _x1 - _x0 + 2, _y1-yS+1, TFT_CYAN);
	tft.setCursor(_x0, yS);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	byte start = jcu - 1 + ii;
	if (start < 0)start = N50; if (start >= N50)start = 0;
	byte stop = jl + ii; if (ii > 0)stop = jl;
	if (stop < 0)stop = N50; if (stop >= N50)stop = 0;
	scroll(start,stop);
}
void Glx_TWindows::scroll(byte jc,byte jll)// start line and stop line :must be >0 & <max N50 dimension
{
	int i = stc[jc];
	tft.setCursor(_x0, yS);
	while (i++ != stc[jll]) {

		if (buffer[i - 1] == '\n') {
			tft.fillRect(tft.cursor_x, tft.cursor_y, tft.width() - 1 - tft.cursor_x, _fontH, TFT_BLACK);
			tft.write('\n');
			tft.setCursor(tft.cursor_x + _x0, tft.cursor_y);
		} else
			tft.write(buffer[i - 1]);
		if (i >= N500)i = 0;
	}
}
size_t Glx_TWindows::write(uint8_t data) {
	tft.setTextFont(0);
	tft.setTextSize(0);
	if (Scroll) {
		if (data == '\r' || xPos > tft.width()) {
			xPos = 0;
			yDraw = scroll_line(); // It takes about 13ms to scroll 16 pixel lines
			//long times = millis();
			delay(20);
		}
		if (data > 31 && data < 128) {
			//	if (millis()<times+20)

			xPos += tft.drawChar(data, xPos, yDraw, 2);
			blank[(18 + (yStart - TOP_FIXED_AREA) / TEXT_HEIGHT) % 19] = xPos; // Keep a record of line lengths
		}
		//change_colour = 1; // Line to indicate buffer is being emptied
	} else
#ifndef SCROLL_HW

	{
		//tft.setTextPadding(200);
		//  jl current line
		// jcu top of screen line
		buffer[ic++] = data;
		if (ic >= N500)ic = 0;
		//Serial.print(data);
		tft.write(data);
		if (data == '\n')					tft.setCursor(tft.cursor_x + _x0, tft.cursor_y);
		if (data == '\n') {
			Serial.printf("JL  %d %d %d \r\n", jl, jcu, ic);

			if (++jl >= N50)jl = 0;
			stc[jl] = ic;
			if (jl >= nLines)scrolla = true;

			if (scrolla) {
		
				{   scroll(jcu, jl);
		
					jcu++; if (jcu >= N50)jcu = 0;
					tft.fillRect(_x0 , tft.cursor_y, tft.width() - 2 - _x0, _fontH, TFT_BLACK);

					//				tft.drawString("                                        ", tft.cursor_x, tft.cursor_y);
				}
			}


		}
#else
		tft.write(data);
#endif
	}
}



// ##############################################################################################
// Call this function to scroll the display one text line
// ##############################################################################################
int  Glx_TWindows::scroll_line() {
	int yTemp = yStart; // Store the old yStart, this is where we draw the next line
						// Use the record of line lengths to optimise the rectangle size we need to erase the top line
	tft.fillRect(0, yStart, 280/*blank[(yStart - TOP_FIXED_AREA) / TEXT_HEIGHT]*/, TEXT_HEIGHT, ILI9341_BLACK);

	// Change the top of the scroll area
	yStart += TEXT_HEIGHT;
	// The value must wrap around as the screen memory is a circular buffer
	if (yStart >= ScreenH - BOT_FIXED_AREA) yStart = TOP_FIXED_AREA + (yStart - ScreenH + BOT_FIXED_AREA);
	// Now we can scroll the display
	scrollAddress(yStart);
	return  yTemp;
}

// ##############################################################################################
// Setup a portion of the screen for vertical scrolling
// ##############################################################################################
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void  Glx_TWindows::setupScrollArea(uint16_t TFA, uint16_t BFA) {
	tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
	tft.writedata(TFA >> 8);
	tft.writedata(TFA);

	tft.writedata((ScreenH - TFA - BFA) >> 8);
	tft.writedata(ScreenH - TFA - BFA);
	tft.writedata(BFA >> 8);
	tft.writedata(BFA);
}

// ##############################################################################################
// Setup the vertical scrolling start address
// ##############################################################################################
void  Glx_TWindows::scrollAddress(uint16_t VSP) {
	tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling start address
	tft.writedata(VSP >> 8);
	tft.writedata(VSP);
}
void Glx_TWindows::textColor(int color) {
	tft.setTextColor(color);
}
void Glx_TWindows::throttle(int xpos, int ypos) {

	char cc[] = { '|','/','-','\\' };
	tft.setCursor(xpos, ypos+yS);
	tft.fillRect(xpos, ypos + yS, 8,10 , ILI9341_BLACK);
//	tft.setTextColor(ILI9341_BLACK);
//	tft.write(cc[index]);
	if (index++ == 3)index = 0;
	tft.setTextColor(ILI9341_WHITE);
	tft.write(cc[index]);
	
}

void Glx_PopWindowsClass::init(int x0, int y0, int x1, int y1, byte col, String title) {
	
	//dimensions 120*100
	byte HH = 100;
	byte WW = 120;
	byte W=tft.textWidth(title);
	if (W > WW-20) WW = W + 20;
	tft.fillRect(x0, y0, WW, 100, TFT_WHITE);
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_YELLOW);
	tft.setTextPadding(WW);
	tft.drawString(title, x0 , y0 + 10);
	byte buttonW = 60;
	byte buttonH = 50;
	buttonY.initButton(&tft,
		20 + x0,
		50 + y0,
		buttonW, buttonH,
		ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
		"OK", 1);
	buttonN.initButton(&tft,
		100 + x0,
		50 + y0,
		buttonW, buttonH,
		ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
		"can", 1);
	buttonN.drawButton(true); buttonY.drawButton(true);
}

int Glx_PopWindowsClass::Confirm(uint16_t x, uint16_t y) {
	
	buttonN.press(buttonN.contains(x, y));

	if (buttonN.isPressed()) {
	
		buttonN.drawButton(true);

		while (touchIsTouching()) {}
        buttonN.press(false);
		buttonN.drawButton();
		return -1;
	}
	buttonY.press(buttonY.contains(x, y));

	if (buttonY.isPressed()) {

		buttonY.drawButton(true);

		while (touchIsTouching()) {}
		buttonY.press(false);
		buttonY.drawButton();

		return 1;
	}
	return 0;
}

void Glx_PopWindowsClass::end() {
}

void Glx_List::draw_time_axis() {
	
	byte nstep = int((vmax - vmin)/60);  //hour steps

	for (byte i = 0; i < nstep; i++) {
		int val = (vmin/60 + i*(vmax - vmin)/60 / nstep);
		Serial.println(val);
		tft.setTextColor(TFT_WHITE,backgroundColor);
		tft.setCursor(RECT_X + 1 + i*RECT_W / nstep, y0-4);
		tft.print(val);
		//tft.drawNumber(val, RECT_X+1 + i*RECT_W / nstep,  y0,0);
		tft.drawFastVLine(RECT_X + i*RECT_W / nstep, y0, Ymax - y0,TFT_WHITE);
	if(nstep<=6)for (byte j=1;j<6;j++)tft.drawFastVLine(RECT_X + i*RECT_W / nstep+j*((RECT_W / nstep)/6), y0, Ymax - y0, TFT_DARKCYAN);
	}
}
void Glx_List::begin(int x,int  y,int yM) {
	x0 = x;
	y0 = y;
	Ymax = yM;
}
void Glx_List::makelist(char* names[], int* val1, int*val2, String* val3, byte tot, byte type, bool scroll) {
	//list = (List*) malloc(sizeof(List)*tot);
	//yy = (int*)malloc(tot * 2);
	Serial.printf("allocated %d\r\n", sizeof(List)*tot);
	Serial.println(names[2]);
	for (byte i = 0; i<tot; i++) {
		if (val1 != NULL)list[i].start = val1[i];
		Serial.println(i);
		list[i].name = names[i];
	//	if (val2 != NULL)list[i].stop = val2[i];
		list[i].stop = 0;
		if (val2 != NULL) {
			val2_present = true; list[i].stop = val2[i];
		}
		if (val3 != NULL)list[i].value = val3[i];
	//	if (val3 != NULL)list[i].value = val3[i];


	}
	_n = tot;
	_type = type;
	if (scroll) {
		char* cc[2] = { "^","v" };
		for (byte i = 0; i < 2; i++) {
			scrollBut[i].initButton(&tft,
				-20 + tft.width() - x0,
				i == 1 ? -20 + Ymax : y0+20 ,
				30, 30,
				ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				cc[i], 1);
			scrollBut[i].drawButton();
		}
	}
	_first = 0;
	//	x0 = x;
	//	y0 = y;
	//	Ymax = y + dy;
	Serial.println("endMake");

}
void Glx_List::makelist( String *names, int* val1, int*val2, String* val3, byte tot,byte type,bool scroll) {
	//list = (List*) malloc(sizeof(List)*tot);
	//yy = (int*)malloc(tot * 2);
	Serial.printf("allocated %d\r\n",sizeof(List)*tot);
	Serial.println(names[2]);
	for (byte i=0;i<tot;i++)
	{
		if (val1 != NULL)list[i].start = val1[i];
		Serial.println(i);
		list[i].name = names[i];
		list[i].stop = 0;
		if (val2 != NULL) {
			val2_present = true; list[i].stop = val2[i];
		}
		if(val3!=NULL)list[i].value = val3[i];

		Serial.printf("start=%d stop=%d \r\n", list[i].start, list[i].stop);
	}
	_n = tot;
	_type = type;
	if (scroll) {
		char* cc[2] = { "^","v" };
		for (byte i = 0; i < 2; i++) {
			scrollBut[i].initButton(&tft,
				-20 + tft.width() - x0,
				i == 1 ? -20 + Ymax : y0+20,
				30, 30,
				ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				cc[i], 1);
			scrollBut[i].drawButton();
		}
	}
	_first = 0;
//	x0 = x;
//	y0 = y;
//	Ymax = y + dy;
	Serial.println("endMake");

}
void Glx_List::makelist(str_funct f, byte tot, bool scroll) {
	//list = (List *)malloc(sizeof(List)*tot);
	//yy = (int*) malloc(tot * 2);
	for (byte i = 0; i<tot; i++) {
		list[i].name = f(i);
		

	}
	_n = tot;
	_type = -2;
	if (scroll) {
		char* cc[2] = { "^","v" };
		for (byte i = 0; i < 2; i++) {
			scrollBut[i].initButton(&tft,
				-20 + tft.width() - x0,
				i == 1 ? -20 + Ymax : y0 + Htitle,
				30, 30,
				ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
				cc[i], 1);
			scrollBut[i].drawButton();
		}
	}
	_first = 0;
	_funct = f;
	//	x0 = x;
	//	y0 = y;
	//	Ymax = y + dy;

}
void Glx_List::drawItem(int x, int y, byte i,bool select) {
	if (_type == -2)tft.drawString(list[i].name, x, y);
	else {
		x0 = RECT_X;
		int sign = 1;
	//	if (list[i].start < list[i].stop) { vmax = list[i].stop; sign = -1; }
	//	if (vmax < list[i].value)vmax = list[i].value;
		if (RECT_B_COL != backgroundColor) {
			Serial.printf("Bord col=%d ",RECT_B_COL);
			tft.drawRect(x0, y, RECT_W, RECT_H, RECT_B_COL);
			tft.fillRect(x0 + 1, y + 1, RECT_W - 2, RECT_H - 2, RECT_F_COL);
			if (select)	tft.fillRect(x0 + 1, y + 1, RECT_W - 2, RECT_H - 2, TFT_GREENYELLOW);

			if (_type == 0) {
				Serial.printf("dv=%d dv=%d\r\n", list[i].stop - vmin, (list[i].start - vmin));
				int v1 = long(RECT_W)*(list[i].start - vmin) / (vmax-vmin);
				int v2 = long(RECT_W)*(list[i].stop - vmin) / (vmax-vmin);
				Serial.printf("v1=%d v2=%d\r\n", v1, v2);
				if (v2 > RECT_W )v2 = RECT_W;
				if (v1 < 0)v1 = 0;
				if (v1 > RECT_W)v1 = RECT_W;
				if (v2 < 0)v2 = 0;

			//	if (sign == 1) { sign = v2; v2 = v1; v1 = sign; }
				Serial.print(v1); Serial.print(' '); Serial.println(v2);
				tft.drawRect(x0 + v1, y, v2 - v1, RECT_H - 2, RECT_B_COL);
				tft.fillRect(x0 + v1 + 1, y + 1, v2 - v1 - 2, RECT_H - 2, RECT_P_COL);

			}
		}
		tft.setTextDatum(MC_DATUM);
		tft.setTextColor( TEXT_COL);
		tft.setTextPadding(2);
		if (select) tft.setTextColor(TEXT_SEL_COL);
		Serial.printf("text col=%d background col=%d\r\n", TEXT_COL, backgroundColor);
		Serial.println(x0);
	//	Serial.println(x0 + RECT_W / 2);
		Serial.println(list[i].name);

		if (_type > 0) {
			tft.setTextDatum(ML_DATUM);
			tft.setTextFont(LIST_FONT);
		}
		else
#ifdef GWIN_FONT 
			tft.setFreeFont(GWIN_FONT);
#else 
			tft.setTextFont(LIST_FONT);
#endif
		
		tft.drawString(list[i].name,_type>0?x0+2: x0 + RECT_W / 2, y - 3 + RECT_H / 2);
		if (_type > 0) {
			tft.setTextDatum(MR_DATUM);
			tft.drawString(list[i].value, x0 + RECT_W, y - 3 + RECT_H / 2);
		}
		if (_type > 1) {
			tft.setTextDatum(MR_DATUM);
			tft.drawNumber(list[i].start, x0 + RECT_W-2, y - 3 + RECT_H / 2);
		}
		if (_type > 2)
			tft.drawNumber(list[i].stop, x0 + RECT_DX_VAL, y - 3 + RECT_H / 2);
	}
#ifdef GWIN_FONT 
//	tft.setFreeFont();
	tft.setTextFont(1);
	tft.setTextDatum(TL_DATUM);
#endif
	
}
#define LIST_UPPER_MARGIN 10
void Glx_List::drawList( byte n,byte first) {
	_n = n;
	byte ii = 0;
	if(_type==0)draw_time_axis();
	_first = first;
	for (byte i = first; i < first + n; i++) {
		yy[i] = -100;
		int dy = (Ymax - y0-5) / n;
		if (dy < RECT_H + 2)dy = RECT_H + 2;
		int y = y0 +(ii++)* dy+LIST_UPPER_MARGIN ;
		if(y+RECT_H<Ymax)
		drawItem(x0, y, i);
		yy[i] = y;
	}
}

byte Glx_List::selectList(uint16_t x, uint16_t y) {

	if(x<RECT_DX_VAL*4)
	for (byte i = _first; i <_first+ _n; i++) {
		if(!val2_present||list[i].stop>=0)
				if (y > yy[i]  && y < yy[i] + RECT_H )return i;
	}
	return 255;
}

void Glx_List::editList(int item )// item to edit= 1:start 2:stop 3:string
{
	bool done = false;
	byte n0 = _first;
	byte n = _n;
	do {
		uint16_t x = 0, y = 0;
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
			byte n = selectList(x, y);
			if (n < 255) {
				selected = n;
				drawItem(x0, yy[n], n, true);
				while (touchIsTouching()) {};
				Serial.println(n);
				if (item < 3) {
					Glx_keypad pad;
					pad.init(100, 280, 0);
					int newval = pad.input(list[n].stop);
					if (item == 1)list[n].start = newval;
					else list[n].stop = newval;
					//if (_type == -2)values[n] = newval;
					tft.fillRect(x0, y0, tft.width() - 100, Ymax - y0,backgroundColor );

				} else {
					Glx_keyborad keyb;
					keyb._ypos = 250;
					keyb.init(0);
					if (keyb.getChar())list[n].value = keyb.retChar;
					keyb.end();
					//keyb.init(0);
					tft.fillRect(0, keyb._ypos-50, tft.width() , keyb.y_size+50, backgroundColor);

				}
				//else if (x < x0 + RECT_DX_VAL + 50) list[n].start = newval;
                //else if (x < x0 + 2 * RECT_DX_VAL + 50) list[n].stop = newval;
			    //else if (x < x0 + 3 * RECT_DX_VAL + 50) list[n].value = newval;
				done = true;
				//tft.fillRect(x0, y0, tft.width() - 100, Ymax - y0, GWIN_B_COL);
     // str_funct f user function to generate new strings
			if (_type == -2)for (byte i = 0; i < _n; i++)list[i].name = _funct(i);
			
				drawList(_n,_first);
				//drawItem(x0, yy[n], n);
				break;

			}

			scrollList(x, y);
		}
		
	} while (!done);
}
void Glx_List::editList(String * values) {
	bool done = false;
	byte n0 = _first;
	byte n = _n;
	do {
		uint16_t x = 0, y = 0;
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
			byte n = selectList(x, y);
			if (n < 255) {
				drawItem(x0, yy[n], n, true);
				while (touchIsTouching()) {};
				Serial.println(n);

				Glx_keyborad keyb;
				keyb._ypos = 250;
				 keyb.init(0);
				 if(keyb.getChar())values[n] = keyb.retChar;
				 keyb.end();
				done = true;
				tft.fillRect(0, keyb._ypos, tft.width() , Ymax - keyb._ypos, GWIN_B_COL);
				// str_funct f user function to generate new strings
				if (_type == -2)for (byte i = 0; i < _n; i++)list[i].name = _funct(i);

				drawList(_n, _first);
				//drawItem(x0, yy[n], n);
				break;

			}

			scrollList(x, y);
		}

	} while (!done);
}
void Glx_List::scrollList(uint16_t x, uint16_t y) {
	byte i;
	for ( i = 0; i < 2; i++) {
		scrollBut[i].press(scrollBut[i].contains(x, y));

		if (scrollBut[i].isPressed()) {

			scrollBut[i].drawButton(true);

			while (touchIsTouching()) {}
			scrollBut[i].press(false);
			scrollBut[i].drawButton();
			break;
		}
	}
	if (i > 1)return;
	if (i == 1)_first--;
	if (i == 0)_first++;
	Serial.println(_n);
	Serial.println(_first);
	drawList(_n,_first);
	return;



}

void Glx_List::end() {
	tft.fillRect(x0, y0, tft.width(), Ymax - y0, TFT_WHITE);
}
