// Glx_SWindows.h


#ifndef _GLX_SWINDOWS_h
#define _GLX_SWINDOWS_h
#define tft_t Adafruit_ILI9341
#define WIN_TEXT_SCROLL
#define WIN_GRAPH
#define WIN_BUTTONS

#define STL_VECTOR
#define PLOT_VECTOR_SIZE 40
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#ifdef ESP8266
#include <Adafruit_ILI9341esp.h>
#include <XPT2046.h>
#else
#include <TFT_eSPI.h>
#define ILI9341_BLACK TFT_BLACK
#define ILI9341_BLUE TFT_BLUE
#define ILI9341_WHITE TFT_WHITE
#define ILI9341_DARKCYAN TFT_DARKCYAN
#define ILI9341_GREENYELLOW TFT_GREENYELLOW
#endif
#include <TimeLib.h>
#include <Adafruit_GFX.h>
#include <vector>
#include "Free_fonts.h"
#include "myIcon.h"
// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15
#define ILI9341_VSCRDEF 0x33
#define ILI9341_VSCRSADD 0x37

#ifdef ESP8266
extern Adafruit_ILI9341 tft; //= Adafruit_ILI9341(TFT_CS, TFT_DC);
extern XPT2046 touch;//(/*cs=*/ 16, /*irq=*/ 0);
#else
extern TFT_eSPI tft;

#endif

//___________GRAF DEFINES_____________________________________________________
#define TIME_SCA 1
#define TICKSIZEL 3
#define xMargin 2
#define yMargin 2
#define YSign 1
#define LegYmargin 0
#define LegXmargin 4
#define TICKSIZE 2
//________________________________________________________multi language_______________________________________________________________
#include <Preferences.h>
#define TRANSLATE(x) Prerences lang;lang.begin(x); 
#define T(x) ilang>0?pref.getString(x):x

static int ScreenH;
//_____________________FONT SELECTION___________________________________
//   1. FF# where # is a number determined by looking at the list below
//       an example being:
//
//       tft.setFreeFont(FF32);
//
//    2. An abbreviation of the file name. Look at the list below to see
//       the abbreviations used, for example:
//
//       tft.setFreeFont(FSSBO24)
//
//       Where the letters mean:
//       F = Free font
//       M = Mono
//      SS = Sans Serif (double S to distinguish is form serif fonts)
//       S = Serif
//       B = Bold
//       O = Oblique (letter O not zero)
//       I = Italic
//       # =  point size, either 9, 12, 18 or 24
//
//static  int winXmax, winXmin, winYmax, winYmin, backgroundColor;// xMargin, yMargin;
//_______LIST DEFINES________________________
#define RECT_X 50                      //margine destro sinistro
#define RECT_DX_VAL 50                 //distanza colonne valori
#define RECT_H 24                      //altezza rettangoli
#define RECT_S_H 14                    //altezza caratteri
#define RECT_W (tft.width()-100)       //largherra rettangoli
#define RECT_B_COL BorCol              //colore bordo 
#define RECT_F_COL TFT_WHITE           //colore filling
#define RECT_P_COL TFT_YELLOW          //colore progress bar
#define RECT_T_COL TFT_BLACK           //colore backgroud
#define TEXT_COL  text_color           //colore text
#define TEXT_SEL_COL TFT_BLUE           //font color when selected
#define LIST_FONT fontList				//font used on list
 
//_____________KEYBOARD KEYPAD KEYPAD DEFINE_________________________
#define SP4 3                             // space between button
#define KEYB_BUT_H 30
#define KEYB_BUT_W (tft.width()/11)
#define KEYB_H 180                         // 5*(KEYB_BUT_H+SPA)+15
#define KEYP_BUT_W 30
#define KEYP_BUT_H 27
#define KEYP_H  30 + (KEYP_BUT_H + 4) * 4   //166
//__________MENU DEFINES
#define MAX_MENU 10                    //max menu n.
#define MENU_H 30
//___________GWINDOWS DEFINES________________________
#define GWIN_TITLE  "OpenSprinkler"+data
#define GWIN_T_H    50
#define GWIN_T_COL  TFT_BLACK 
//#define GWIN_SCROLL
#define TITLE_FONT FF19
#define GWIN_FONT  FF18
#define LIST_TITLE_FONT TFont          //TitleFont
#define TITLE_TFONT FF18               //TitleFreeFont used if TFont==255
#define DEF_FONT FM9                   //default font for what?
//_____________TEXT WINDOW DEFINE____________________
#define TWIN_BACK_COL TFT_BLACK
#define TWIN_TITLE
//#define TWIN_T_H
#define TWIN_T_COL
#define TWIN_SCROLL
#define TWIN_FONT

//_______________________________WINDOWS DIMENSIONS___________________________POP UP__________
#define MENU_H  30              //   Title     |  Title   |              |
#define GRAPH_P 0               //             |          | GWIN         |
#define GRAPH_H 150             //    GWIN     |  text W  |_____________ |____________270
#define GWIN_B_COL TFT_WHITE    //    (List)   |  (scroll)|          270 |           |-------|300____
#define TEXT_H 40               //             |          | text W       |  keyboard |keypad | popUp 350
#define TEXT_P 220              //   --------------------------------450 |-----------|-------|------------
#define TEXT_S 20               //    MENU        MENU      MENU            menu       menu     menu
#define KEYB_Y 300   // windows start at -30   so 270
#define KEYP_X 100
#define KEYP_Y 280  //
typedef void(*THandlerFunction)(void);
typedef String(*str_funct)(int i);
#define RGB24TO16(x) ((x>>18)<<11)&(((x&0xFF00)>>11)>>6)&((x&0xFF)>>2)

void drawIcon(const unsigned short * icon, int16_t x, int16_t y, uint16_t width, uint16_t height);

void drawBmp(const char *filename, int16_t x, int16_t y);


static bool touchIsTouching() {
#ifdef ESP32	
	return tft.getTouchRawZ();
#else
	return touch.isTouching();
#endif
}

class Glx_keyborad {
public:
#ifdef ESP32
	TFT_eSPI_Button button[47];
#else
	Adafruit_GFX_Button button[47];
#endif
	int _ypos;
	void init(byte Uppercase);
	char* line[4][2] = {
		{ "1234567890",		"!\"?$%&/()="},
		{"qwertyuiop",		"QWERTYUIOP" },
		{"asdfghjkl+",		"ASDFGHJKL*"},
		{"zxcvbnm,.-",		"ZXCVBNM;:_"}
};
	char* LastLine[6];
	byte CaseStatus;
	char isPressed(uint16_t x,uint16_t y);
	bool getChar();
	char retChar[20];
	void end();
	byte y_size = 180;       //altezza keyboard

};
class Glx_PopWindowsClass :public TFT_eSPI{
	void init(int x0, int y0, int x1, int y1,byte col,String title);//screen pixels
	 TFT_eSPI_Button buttonY, buttonN;
	 int Confirm(uint16_t x,uint16_t y);
	 void end();
};
class Glx_keypad {
public:
	TFT_eSPI_Button button[15];
	char c[20];
	int width, height, x0, y0, def;
	int init(int x=KEYP_X, int y=KEYP_Y,int def=0);
	int input(int maxval=0);
	void inputTime(time_t current);

};
static 	 int Xmax, Xmin, Ymax, Ymin;
class Glx_MWindowsClass {
public:
	 byte curr_m;
	 int i_sel = 0;
	void init( int x0=0, int y0=tft.height()-MENU_H, int x1=tft.width(), int y1=tft.height());//screen pixels bottom menu
	byte nmenu;
	int getPressed(uint16_t x,uint16_t y);

	struct Menu {
#ifdef ESP32
		TFT_eSPI_Button button[5];
#else
		Adafruit_GFX_Button button[10];
#endif
		void init();
		byte nbutton;
		char menuName[5][12];
		void draw();                     //draw current menu all buttons
		byte  menuIndex[5];
		THandlerFunction handlerF[5] = { NULL,NULL,NULL,NULL,NULL };


	};
	void handleMenus();
	Menu menu[MAX_MENU];
};
struct List {
	String name;
	int start;
	int stop;
	String value;
};
class Glx_List {
public:
	byte fontList = 2,selected;  uint16_t BorCol = TFT_BLUE;
	byte TFont = 255;  uint16_t text_color = TFT_BLACK;
	List* list;// [45];
	int* yy;// [45];
	str_funct _funct;
	Glx_List(const byte size) {  list = new List[size]; yy = new int[size];}

	void draw_time_axis();

	void begin(int x=0,int y=0,int yMax=tft.height());
	void makelist(char * names[], int * val1, int * val2, String * val3, byte tot, byte type, bool scroll);
	                                      //  
                                        // y position of any element
	int x0, y0, Ymax, _type, _first, _n, val2_present;
	int vmin = 3*60;
	int vmax = 8*60;

	void makelist( String * names, int * val1, int * val2, String * val3, byte tot,byte type,bool scroll);//type=1 scroll vert
	void makelist(str_funct f, byte tot, bool scroll);

	void drawItem(int x, int y, byte i,bool s=false);    // to draw a single element
	void drawList(  byte n, byte first);                 // to draw a list generated by make list from element (first for n)
	byte selectList(uint16_t x, uint16_t y);             // to seclect a single element 
	void editList(int item );                  // to select and edit any element (stop and wait user action)
	void editList(String * values);

	void scrollList(uint16_t x, uint16_t y);
	TFT_eSPI_Button scrollBut[4];

	void end();
};

extern Glx_MWindowsClass Glx_MWindows;
static int winXmax, winXmin, winYmax, winYmin, Htitle=0;
static uint16_t backgroundColor;

class Glx_GWindowsClass {
public:
	static float xmax, ymax, xmin, ymin;
	static float scax, scay;
	void title(String title, uint16_t col, int font);
	void init( int x0, int y0, int x1, int y1, uint16_t backColor);//screen pixels
	int xpressed(int x, int y);
	int ypressed(int x, int y);
	Glx_List list (byte size);
	
	class Graf {
	public:

		//static int Xmax = winXmax; static int Xmin = winXmin; static int Ymax = winYmax;
		void init(uint16_t col);
		int nval;
#ifdef STL_VECTOR
		std::vector<int> x;
		std::vector<int> y;
#else
		int  x[PLOT_VECTOR_SIZE], y[PLOT_VECTOR_SIZE];
#endif
		//		float scax, scay;
		float x0, y0;
		int windowsH = 100, windowsW = 240, yGrafScr = 50;
		uint16_t color;
		boolean draw();
		boolean drawAxX(float y_pos, float deltx);
		void changeScaX(float dx);
		void scroll(float dx);
		boolean drawAxX(float y_pos, float deltx, byte ty);
		boolean drawAxy(int x_pos, float delty, byte ty);

	};
};
#define N500 1000         // maximum text buffer
#define N50 80           // maximum n. of lines 
 
class Glx_TWindows :public Print {
public:
	void charPos(byte nline, byte nchar,byte mode);
	void init(int x0, int y0, int x1, int y1,byte textH, byte mode);
	void redraw(int ii=0);
	void scroll(byte jc, byte jll);
	virtual size_t write(uint8_t);
	void throttle(int xpos, int ypos);
	void textColor(int color);

protected:
    int  _x0,_x1,_y1, yS,yDraw,nLines, xPos, yStart, TOP_FIXED_AREA, BOT_FIXED_AREA, TEXT_HEIGHT;
	int jcu = 0, ic = 0, jl = 0, stc[N50] = { 0 }; char buffer[N500]; bool scrolla = false;
	byte _fontH;
	byte index;
	char blank[100];
	bool Scroll;
	void scrollAddress(uint16_t _vsp);
	void setupScrollArea(uint16_t tfa, uint16_t bfa);
	int scroll_line();
};
#endif