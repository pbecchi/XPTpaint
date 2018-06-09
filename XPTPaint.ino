#include <Arduino.h>
#include <SPI.h>
#ifdef ESP8266
#include <Adafruit_ILI9341esp.h>
#include <XPT2046.h>

#else
//#define FS_NO_GLOBALS
#include <FS.h>
#include <TFT_eSPI.h>
#include <WiFi.h>

#endif
/*  ________________________TOUCH FUNCTIONS________________________
  uint8_t  getTouchRaw(uint16_t *x, uint16_t *y);
  uint16_t getTouchRawZ(void);
  uint8_t  getTouch(uint16_t *x, uint16_t *y, uint16_t threshold = 600);
  _______________________for touch calibration store and restore data______________________________
  void     calibrateTouch(uint16_t *data, uint32_t color_fg, uint32_t color_bg, uint8_t size);
  void     setTouch(uint16_t *data);
*/
#include <Preferences.h>
#include <Adafruit_GFX.h>
//#include "../../Arduino/OpenSprinkler/OpenSprinkler_Arduino_V_2_1_6/json_parse_program/Glx_SWindows.h"
#include "Glx_SWindows.h"

#include "startupRed.h"
// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15
#define N_curves 2
#ifdef ESP8266
 Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
 XPT2046 touch(/*cs=*/ 16, /*irq=*/ 0);
#else
TFT_eSPI tft = TFT_eSPI();
#define ILI9341_BLACK TFT_BLACK
#define ILI9341_BLUE   TFT_BLUE
#define ILI9341_WHITE  TFT_WHITE



#endif
Adafruit_GFX_Button button;
Glx_MWindowsClass MWin;
Glx_keyborad MyKeyb;
Glx_TWindows TWin;
Glx_GWindowsClass Gwin;
Glx_GWindowsClass::Graf myGraph[N_curves];
#define CALIBRATION_FILE "touch.dat"
Preferences pref;
void startTouch(bool reCal=false) {

	bool calDataOK=false;
	uint16_t calibrationData[5];

	tft.setRotation(2);
	tft.fillScreen((0xFFFF));

	tft.setCursor(20, 0, 2);
	tft.setTextColor(TFT_BLACK, TFT_WHITE);  tft.setTextSize(1);
	tft.println("calibration run");

#ifndef ESP32
	if (!SPIFFS.begin()) {

		Serial.println("formating file system");

		SPIFFS.format();
		SPIFFS.begin();
	}
	// check if calibration file exists
	if (SPIFFS.exists(CALIBRATION_FILE)) {

		File f = SPIFFS.open(CALIBRATION_FILE, "r");
		if (f) {
			if (f.readBytes((char *)calibrationData, 14) == 14)
				calDataOK = 1;
			f.close();
		}
	}
	if (calDataOK) {
		// calibration data valid
		tft.setTouch(calibrationData);
	} else {
		// data not valid. recalibrate
		tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 25);
		// store data
		File f = SPIFFS.open(CALIBRATION_FILE, "w");
		if (f) {
			f.write((const unsigned char *)calibrationData, 14);
			f.close();
		}
	}


#else
	pref.begin("LCDcal", false);
	if(pref.getBool("dataPresent")&&reCal==false)
	{
		pref.getBytes("data",(void *) calibrationData,sizeof(calibrationData));
		pref.end();
		tft.setTouch(calibrationData);

	}
	else{
		tft.calibrateTouch(calibrationData, TFT_WHITE, TFT_RED, 25);
		pref.putBytes("data", (void *)calibrationData,sizeof(calibrationData));
		pref.putBool("dataPresent",true);
		pref.end();
}
#endif


}

#define MENU_H  30
#define GRAPH_P 55
#define GRAPH_H 150 
#define TEXT_H 40
#define TEXT_P 220
#define TEXT_S 8
#define KEYB_Y 300
#define KEYP_X 100
#define KEYP_Y 280
int XX = 100, YY = 100;
byte curr_m;
//___________________________functios for factory start up  menus______________________________________
void do_factory_setup() {            //calibrate touch aND
startTouch(true);
do_defaults(); }
bool WiFiMode = false;
void do_defaults() {     //list default configuration:HW,Language,Network;Spiffs;EEprom

	clear_window();
	Gwin.title("FACTORY DEFAULTS", TFT_WHITE, 2);
	byte y = 0, x = 50, y0 = 100; int ystep = 50, x2 = 280;
	tft.setTextFont(2);
	tft.drawString("Hw ", x, y0 + ystep*y); tft.drawString("OK ", x2, y0 + ystep*y++);
	tft.drawString("Network", x, y0 + ystep*y); tft.drawString("WiFi", x2, y0 + ystep*y++);
	tft.drawString("Clear", x, y0 + ystep*y); tft.drawString("No ", x2, y0 + ystep*y++);
	tft.setTextFont(1);
}
void do_network() {
	clear_window();
	Gwin.title("Network access hardware", TFT_WHITE, 2);
	tft.setCursor(50, 100);
	if (!WiFiMode)
		tft.printf("%16s    selected", "Ethernet");
	else
		tft.printf("%16s    selected", "WiFi");

}
void do_language_list() {};           //list available languages for selection
void do_ethernet() {                //list ethernet configuration: wifi access point
	WiFiMode = false;
	do_network();
}
static int pos = 0;
void do_clear_spiffs() {};            //clear format spiffs flash memory
void do_clear_EEprom() {};            //clear format EEprom flash memory 
void scroll_up() { TWin.redraw(++pos); }
void scroll_down() { TWin.redraw(--pos); }

void clear_window() {
	Gwin.init(0, 0, tft.width(), 450, ILI9341_BLACK);
	tft.fillRect(winXmin, winYmin, winXmax - winXmin, winYmax - winYmin, backgroundColor);
//	tft.setFreeFont(FF1);
}

#define Serial TWin
void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
	Serial.printf("Listing directory: %s\n", dirname);

	File root = fs.open(dirname);
	if (!root) {
		Serial.println("Failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		} else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}
#define Serial Serial
bool start = true;
void do_startup() {
	tft.fillRect(0, 0, 320, 320, TFT_WHITE);
	drawBmp("/startupRed.bmp", 26, 43);	//drawIcon(startupRed,26,43,267,214);
	if (start) {
		start = false;
		TWin.init(0, 300, tft.width(), 450, TEXT_S, 0); //located at the bottom!!!!
		TWin.printf("Start Up......\r\n 9 Jun 2018 15:00:00 Location: Italy \r\n\r\n");
		TWin.println(" Start SPIFFS:");
		listDir(SPIFFS, "/", 0);


	} else TWin.redraw();
}
void select_wifi(){
	Gwin.init(0, 0, tft.width(), 450, TFT_BLACK);
	Gwin.title("WiFi Select", TFT_WHITE, 0);
	byte nmax = WiFi.scanNetworks();

	String WIFIlist[10], psw[10];
	pref.begin("wifi",false);
	for (byte i = 0; i < nmax; i++) {
		WIFIlist[i] = WiFi.SSID(i);
		psw[i]=pref.getString(WIFIlist[i].c_str());
		Serial.println(WIFIlist[i]);
		Serial.println(psw[i]);
		Serial.println(psw[i].length());
	}
	Glx_List WiFilist = Gwin.list(12);

	WiFilist.begin(0, 50, 450);

	//WiFilist.makelist(StrAdd, nn, false);   //type=-2 os.options list
	/*tft.setCursor(20, 100);
	tft.printf("SSID=%16s    psw=%16s", "uno", "unopass");
	tft.setCursor(20, 150);
	tft.printf("SSID=%16s    psw=%16s", "due", "duepass");
	*/
	//WiFilist.BorCol = TFT_BLACK;       //no rectangles
	//WiFilist.text_color = TFT_WHITE; 

	WiFilist.makelist(WIFIlist, NULL, NULL, psw, nmax, 1, nmax<10 ? false : true);
	Serial.printf("Bord COLOR=%d ", WiFilist.BorCol);
	WiFilist.drawList(nmax, 0);
	// touch loop do ()while(); not return; check menu check scrolllist;
	uint16_t x, y; int ind = 0; int nn = 255;
	do {
#ifdef ESP8266
		if (touch.isTouching()) {
			touch.getPosition(x, y);
#else
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
#endif
	//		ind = MWin.getPressed(x, y);
	//		WiFilist.scrollList(x, y);
			// on menu edit  30??
			nn=WiFilist.selectList(x,y);
			if (nn != 255) {
				WiFilist.drawItem(WiFilist.x0, WiFilist.yy[nn], nmax, true);
				while (touchIsTouching()) {};
				break;
			}
		}
		} while (nn== 255);
		Serial.println(nn); Serial.println(WIFIlist[nn]);
		TWin.redraw();
		TWin.print("Connecting to ;");TWin.println( WIFIlist[nn]);
		WiFi.begin(WIFIlist[nn].c_str(), psw[nn].c_str());

		byte icount = 0;
		while (WiFi.status() != WL_CONNECTED&&icount++<255) {
			delay(500);
			if (icount % 30 == 0)TWin.println();
			TWin.print(".");
		}
		TWin.println();
		//  WiFi.config(IPAddress(192, 168, 1, 31), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));


		TWin.print("IP number assigned by DHCP is ");
		TWin.println(WiFi.localIP());
}
//_____________________________________________________________________________________________
void setup() {
  delay(1000);
  
  Serial.begin(115200);
 // SPI.setFrequency(ESP_SPI_FREQ);

  tft.begin();
#ifdef ESP32
  
  startTouch();

#else
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
  touch.setCalibration(176, 256, 1739, 1791);//209, 1759, 1775, 273);
#endif
  tft.setRotation(2);      //                 portrait vertical
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  // Replace these for your screen module calibrations
  //_________________________________________________________________________SPIFFS_________________________________________________________________
  if (!SPIFFS.begin()) {
	  tft.println("SPIFFS initialisation failed!");
	  while (1) yield(); // Stay here twiddling thumbs waiting
  }
  tft.println("\r\nSPIFFS initialised.");
  delay(2000);
 // listDir(SPIFFS, "/", 0);
 // tft.fillRect(0, 0, 320, 300, TFT_WHITE);
//  drawBmp("/startupRed.bmp", 26, 43);
 // delay(5000);

  //tft.setRotation(1); //----land scape mode (text scrolling not available)
  //-----------------------------init menus--------------------------------------
  MWin.init(0, tft.height()-MENU_H, tft.width(), tft.height());
  #define FILL_M(i,j,s,y) MWin.menu[i].menuName[j]=s;MWin.menu[i].menuIndex[j]=y;MWin.menu[i].nbutton++;  
#define FILL_F(i,s,y,z) strcpy(MWin.menu[i].menuName[MWin.menu[i].nbutton],s);MWin.menu[i].menuIndex[MWin.menu[i].nbutton]=y;MWin.menu[i].handlerF[MWin.menu[i].nbutton++]=z;  
  FILL_F(0, "Setup", 5, do_factory_setup)
	  FILL_F(4, "HWconf", 0, do_defaults)
	  FILL_F(4, "Language", 0, do_language_list)
	  FILL_F(4, "Network", 6, do_network)
	  FILL_F(5, "Ethernet", 0, do_ethernet)
	  FILL_F(5, "WiFi", 7, do_wifi_list)
	  FILL_F(6, "rescan", 0, do_wifi_list)
	  FILL_F(6, "psw", 0, NULL)// edit_wifi_list)
	  FILL_F(6, "return", 6, do_network)
	  FILL_F(5, "return", 5, do_defaults)
	  //FILL_F(4,  "clear SPIFFS", 0, do_clear_spiffs)
	  //FILL_F(4,  "clear EEPROM", 0, do_clear_EEprom)
	  FILL_F(4, "return", 1, NULL)
	  FILL_F(0, "keyb.", 2, do_keyboard)
  FILL_F(0, "run", 3, NULL)
	  //		  FILL_F(2,  "keypad", 0, do_keypad)
	  FILL_F(2, "run_list", 0, do_sta_list)
	  FILL_F(2, "stop all ", 0, NULL)
	  FILL_F(2, "<", 0, time_dec)
	  FILL_F(2, ">", 0, time_inc)
	  FILL_F(2, "ret", 1, NULL)
  FILL_F(0, "startup", 8, do_startup)
	  FILL_F(7, "WiFi", 0, select_wifi)
	  FILL_F(7, "setTime", 0, do_get_time)
	  FILL_F(7, "options", 4, do_options_list)
	  FILL_F(7, "^", 0, scroll_up);
      FILL_F(7, "v", 0, scroll_down);

	  FILL_F(3, "edit", 0, NULL)
		 FILL_F(3, "return", 8, do_startup)
	  FILL_F(1, "^", 0, scroll_up);
      FILL_F(1, "v", 0, scroll_down);
	  FILL_F(1, "exit", 1, NULL);


/*		  FILL_M(1, 0, "<-", 0)
		  FILL_M(1, 1, "->", 0)
		  FILL_M(1, 2, "X+", 0)
		  FILL_M(1, 3, "X-", 0)
		  FILL_M(1, 4, "ret", 1)
		  */
	  MWin.menu[0].init();
	  MWin.menu[0].draw();
//  Serial.println(MWin.menu[1].menuName[3]);
//  Serial.println(MWin.menu[1].nbutton);
   //------------------------------init Graphics-------------------------------------
	  myGraph[0].x.resize(50);
	  myGraph[0].y.resize(50);
	  myGraph[1].x.resize(50);
	  myGraph[1].y.resize(50);

	  for (byte j = 0; j < 50; j++)
   {
	   
	   myGraph[0].x[j] = j*10. ;
	   myGraph[0].y[j] = 1000*sin(j*10. / 57);
	   myGraph[1].x[j] = j*10. ;
	   myGraph[1].y[j] = 1000*cos(j*10. / 57)+1;
	   Serial.printf(" x= %d  sin()= %d   cos()= %d \r\n", j * 10, myGraph[0].y[j], myGraph[1].y[j]);
   }

   myGraph[0].nval = 50;
   myGraph[1].nval = 50;
   //_________________________________shows fonts
   const GFXfont* ff[] = {FF1, FF2, FF3, FF4, FF5, FF6, FF7, FF8, FF9,FF10,FF1,FF12,FF13,FF14,FF15,FF16,FF17,FF18,FF19,FF20,FF21};
   for (byte i = 0; i < 10; i++) {
	   tft.setTextFont(i);
	   String tex = "Font n.";
	   tft.setFreeFont(ff[i]);
	   tft.drawString(tex + i, 10,10+ i * 40);
	   tft.setFreeFont(ff[i+10]);
	   tft.drawString(tex + i, 180, 10 + i * 40);

   }
   
   tft.setTextFont(NULL);
   
//  --------------------------------init KEYBOARD-------------------------------
//   TWin.init(0, TEXT_P, tft.width(), TEXT_P + TEXT_H, TEXT_S, 0); //located at the bottom!!!!


}

void do_graph(){
	
		Gwin.init( 0, GRAPH_P, tft.width(), GRAPH_P + GRAPH_H, ILI9341_WHITE);
		myGraph[0].init(ILI9341_BLACK);
		myGraph[1].init(ILI9341_BLUE);
		for (byte i = 0; i < N_curves; i++) {
			myGraph[i].draw();
			myGraph[i].drawAxX(0, 0.1, TIME_SCA);  // X axis with proposed min step to be adjusted by code
		}
		
	}

static boolean keyb_on;
Glx_List list = Gwin.list(16);

void do_keyboard() {
	TWin.redraw();
	uint16_t x, y, ind;
	MyKeyb._ypos = KEYB_Y;
	char c;
	MyKeyb.init(0);
	if (!keyb_on) {
		keyb_on = true;

		do {
			if (tft.getTouch(&x, &y)) {
				x = tft.width() - x;
				y = tft.height() - y;
				c = MyKeyb.isPressed(x, y);
				ind = MWin.getPressed(x, y);
				if (c > 0) {
					Serial.println((byte)c);
					//	if (c != '<') {
					tft.setTextColor(TFT_WHITE, TFT_BLACK);
					TWin.write(c);
					if (c == 13)TWin.write('\n');
					//	} else {
					//		i--;
					//		tft.fillRect(0, _ypos - 30, tft.width(), 10, TFT_WHITE);
					//	}
					//	retChar[i] = 0;
						//	Serial.println(retChar);
					//	tft.setTextColor(TFT_BLACK);

		//				tft.drawString(retChar, 20, _ypos - 30);
				}
			}
		} while  (ind%10!=1);
	}else
	 {
		MyKeyb.end();
		keyb_on = false;
	}
}
void do_keypad() {
	Glx_keypad mykeypad;
	mykeypad.init(KEYP_X, KEYP_Y, 0);
	TWin.print(mykeypad.input());
	

}
//Glx_GWindowsClass::Glx_List list(); 
void time_inc() { int inc = (list.vmax - list.vmin)/2; list.vmin+=inc; list.vmax+=inc; list.drawList(8,4); }
void time_dec() { int inc = (list.vmax - list.vmin)/2; list.vmin-=inc; list.vmax-=inc; list.drawList(8,4); }
void do_sta_list() {
	Gwin.init( 0, 0, tft.width(), 450, TFT_BLACK);
	Gwin.title("Stations", TFT_WHITE, 0);
	
	list.begin(0, 50, 450);
	String str[16] = {
		"valve n.1",
		"valve n.2",
		"valve n.3",
		"valve n.4",
		"valve n.5",
		"valve n.6",
		"valve n.7",
		"valve n.8",
		"valve n.9",
		"valve n.10",
		"valve n.11",
		"valve n.12",
		"valve n.13",
		"valve n.14",
		"valve n.15",
		"valve n.16"
		 };
	int value0[16] = { 60,120,180,240,260,360,390,480,500,580,620,640,700,800,830,900 },
		value[16] = { 80,150,190,280,270,380,420,500,560,590,630,680,790,810,850,920 };

	list.makelist(str, value0, value, NULL, 16, 0,true);   //type=0 progress bar // type=1 1 par 2 par 3 par
	list.drawList( 8,4);
	list.editList(2);
}
#define N_OPTIONS 45
int option[50] = { 1,2,3,4,5,6,7,8,9,10,11,12,13 };
String string_options(int i) {
	String str = "number ";
	return str + option[i];
}
/*
void edit_list(list list )
{

	// touch loop do ()while(); not return; check menu check scrolllist;
	uint16_t x, y; int ind;
	do {
#ifdef ESP8266
		if (touch.isTouching()) {
			touch.getPosition(x, y);
#else
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
#endif
			ind = MWin.getPressed(x, y);
			list.scrollList(x, y);
			// on menu edit  30??
			if (ind%10 == 1)	list.editList(opt);
		}
	} while (ind%10 != 2);

	// on return exit 31??
}*/
void do_options_list() {
	//menu??         EDIT      RETUR
	uint16_t x, y; int ind=0;
	Gwin.init(0, 0, tft.width(), 450,TFT_BLACK);

	Gwin.title("Option List", TFT_RED, 0);

	Glx_List list = Gwin.list(45);
	list.begin(0, 50, 450);
	char* op_prompts[] = {
		"Firmware version",
		"Time zone (GMT):",
		"Enable NTP sync?",
		"Enable DHCP?    ",
		"Static.ip1:     ",
		"Static.ip2:     ",
		"Static.ip3:     ",
		"Static.ip4:     ",
		"Gateway.ip1:    ",
		"Gateway.ip2:    ",
		"Gateway.ip3:    ",
		"Gateway.ip4:    ",
		"HTTP Port:      ",
		"----------------",
		"Hardware version",
		"# of exp. board:",
		"----------------",
		"Stn. delay (sec)",
		"Master 1 (Mas1):",
		"Mas1  on adjust:",
		"Mas1 off adjust:",
		"Sensor type:    ",
		"Normally open?  ",
		"Watering level: ",
		"Device enabled? ",
		"Ignore password?",
		"Device ID:      ",
		"LCD contrast:   ",
		"LCD brightness: ",
		"LCD dimming:    ",
		"DC boost time:  ",
		"Weather algo.:  ",
		"NTP server.ip1: ",
		"NTP server.ip2: ",
		"NTP server.ip3: ",
		"NTP server.ip4: ",
		"Enable logging? ",
		"Master 2 (Mas2):",
		"Mas2  on adjust:",
		"Mas2 off adjust:",
		"Firmware minor: ",
		"Pulse rate:     "
		"----------------"
		"As remote ext.? "
		"DNS server.ip1: "
		"DNS server.ip2: "
		"DNS server.ip3: "
		"DNS server.ip4: "
		"Special Refresh?"
		"IFTTT Enable: " };


	int val[42] = { 1,2,3,4,5,6,7,8,9 };
	int max[42] = { 1,20,30,40,50,60,70,80,90 };
	list.makelist(op_prompts, val, max, NULL, 42, 2, true);//type=2 os.options list and intereger
	list.drawList(12, 0);

	do {
#ifdef ESP8266
		if (touch.isTouching()) {
			touch.getPosition(x, y);
#else
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
#endif
			Serial.println(y);
			ind = MWin.getPressed(x, y);
			list.scrollList(x, y);
			// on menu edit  30??
			if (ind % 10 == 1)	list.editList(1);
		}
		} while (ind % 10 != 2);
		TWin.redraw();
		TWin.printf("Modified Option[%d]=%d\r\n", list.selected,list.list[list.selected].start);
	
}
String psw[10];
String WIFIlist[10];
String StrAdd(int i) {
	return WIFIlist[i] + psw[i];
}
void do_wifi_list() {               //list  wifi access point and if password is available 
	clear_window();
	pref.begin("wifi", false);
	WiFiMode = true;
	Gwin.init(0, 0, tft.width(), 450, TFT_BLACK);
	Gwin.title("WiFi AP", TFT_WHITE, 0);
	byte nn = WiFi.scanNetworks();

	for (byte i = 0; i < nn; i++) {
		WIFIlist[i] = WiFi.SSID(i);
		psw[i] = pref.getString(WIFIlist[i].c_str());
	}
	Glx_List WiFilist = Gwin.list(12);

	WiFilist.begin(0,  50, 450);

	//WiFilist.makelist(StrAdd, nn, false);   //type=-2 os.options list
											/*tft.setCursor(20, 100);
											tft.printf("SSID=%16s    psw=%16s", "uno", "unopass");
											tft.setCursor(20, 150);
											tft.printf("SSID=%16s    psw=%16s", "due", "duepass");
											*/
	//WiFilist.BorCol = TFT_BLACK;       //no rectangles
	//WiFilist.text_color = TFT_WHITE; 

	WiFilist.makelist(WIFIlist, NULL, NULL, psw, nn, 1, nn<10?false:true);
	Serial.printf("Bord COLOR=%d ",WiFilist.BorCol);
	WiFilist.drawList(10, 0);
	// touch loop do ()while(); not return; check menu check scrolllist;
	uint16_t x, y; int ind;
	do {
#ifdef ESP8266
		if (touch.isTouching()) {
			touch.getPosition(x, y);
#else
		if (tft.getTouch(&x, &y)) {
			x = tft.width() - x;
			y = tft.height() - y;
#endif
			ind = MWin.getPressed(x, y);
			WiFilist.scrollList(x, y);
			// on menu edit  30??
			if (ind % 10 == 2) {
				WiFilist.editList(3);	
				Serial.println(WiFilist.selected);	
				Serial.printf("WiFi %s", WIFIlist[WiFilist.selected].c_str()); 
				WiFilist.list[WiFilist.selected].value[WiFilist.list[WiFilist.selected].value.length() - 2] = 0;
				Serial.println(WiFilist.list[WiFilist.selected].value);
				pref.putString(WIFIlist[WiFilist.selected].c_str(), WiFilist.list[WiFilist.selected].value);
			}
		}
	} while (ind % 10 != 3);
	
		/*	// on menu edit  30??
			if (ind == 62) {
				bool done = false;
				byte n0 = 0;
				byte _n = nn;
				do {

					if (tft.getTouch(&x, &y)) {
						x = tft.width() - x;
						y = tft.height() - y;
						byte n = WiFilist.selectList(x, y);
						Serial.println(n);
						if (n < 255) {
							WiFilist.drawItem(list.x0, list.yy[n], n, true);
							while (touchIsTouching()) {};
							Serial.println(n);

							Glx_keyborad keyb;
							keyb._ypos = 250;
							keyb.init(0);
							if (keyb.getChar())psw[n] = keyb.retChar;
							keyb.end();
							done = true;
							tft.fillRect(0, keyb._ypos, tft.width(), Ymax - keyb._ypos, TFT_BLACK);
							// str_funct f user function to generate new strings
							for (byte i = 0; i < _n; i++)WiFilist.list[i].name = StrAdd(i);

							WiFilist.drawList(_n, 0);
							//drawItem(x0, yy[n], n);
							break;

						}

						WiFilist.scrollList(x, y);
					}

				} while (!done);
			}
		}
		} while (ind != 63);
		*/
		// on return exit 31??
	}
void do_get_time() { 
	Glx_keypad mykeypad;
	TWin.print("Set Time to: ");
	mykeypad.init(100, 300, 0);
	
     mykeypad.inputTime(now());

	Serial.println(mykeypad.c);
	TWin.redraw();
	TWin.println(mykeypad.c);
	char* cc = "   ";
	cc = strtok(mykeypad.c, ":");
	byte h = atoi(cc);
	cc = strtok(NULL, ":");
	byte m = atoi(cc);
	cc = strtok(NULL, " ");
	byte s = atoi(cc);
	cc = strtok(NULL, "-");
	byte d = atoi(cc);
	cc = strtok(NULL, "-");
	byte mo = atoi(cc);
	cc = strtok(NULL, "\0");
	int y = atoi(cc);
	//void    setTime(int hr,int min,int sec,int day, int month, int yr);
	setTime(h, m, s, d, mo, y);
	
}

void edit_wifi_list() {            //add password for selected access point

	clear_window();
	Gwin.title("Access point List", TFT_WHITE, 2);
	tft.setCursor(50, 100);
	tft.printf("SSID=%16s    psw=%16s", "uno");
	tft.setCursor(50, 150);
	tft.printf("SSID=%16s    psw=%16s", "due");
}
char touch_control()
{
	char c;
	
	uint16_t x, y;
#ifdef ESP8266
	if (touch.isTouching()) {
		touch.getPosition(x, y);
	
#else
	if(tft.getTouch(&x, &y)){
		x = tft.width() - x;
		y = tft.height() - y;
#endif
	
		int ind = MWin.getPressed(x, y);
		if (ind > 0) {
			TWin.println(ind);//select menu function case of
			switch (ind) {

			case 2:
				if (!keyb_on){
					MyKeyb._ypos = KEYB_Y;
					MyKeyb.init( 0); keyb_on = true;
                  if(MyKeyb.getChar())
					   TWin.print(MyKeyb.retChar);
					break;
				}
				else {
					MyKeyb.end(); keyb_on = false;
				
					break;
				}
			case 1:{  
				Gwin.init( 0, GRAPH_P, tft.width(), GRAPH_P + GRAPH_H, ILI9341_WHITE);
				myGraph[0].init(ILI9341_BLACK);
				myGraph[1].init(ILI9341_BLUE);
				for (byte i = 0; i < N_curves; i++) {
					myGraph[i].draw();
					myGraph[i].drawAxX(0, 0.1, TIME_SCA);  // X axis with proposed min step to be adjusted by code
				}
				break;
			}
			case 11: {
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].scroll(10.);
				for (byte i = 0; i < N_curves; i++) {
					myGraph[i].draw();
					myGraph[i].drawAxX(0, 1.,0);

				}
				break;
			}
					 //<
			case 12: {                 //>
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].scroll(-10.);
				for (byte i = 0; i < N_curves; i++)
				{
					myGraph[i].draw();
					myGraph[i].drawAxX(0, 1.,0);
				}
				break; }
			case 13: {


				//+
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].changeScaX(1.2);
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].draw();

				break; }

			case 14: {
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].changeScaX(0.8);
				for (byte i = 0; i < N_curves; i++)
					myGraph[i].draw();

				break; }//-
			case 21: {
				Glx_keypad mykeypad;
				TWin.print(mykeypad.init(KEYP_X, KEYP_Y, 0));
				break;

			}
			case 22: { 
				Glx_List list(10);
				list.begin(10, 10, 200);
				String str[5] = { "uno","due","tre","quattro","cinque" };
				int value[5] = { 1,2,3,4,5 }, value0[5] = { 0,1,2,3,2 },valum[5] = { 5,5,5,5,5 };
				
				list.makelist( str, value0, value, NULL, 5, 0,false);
				list.drawList(0, 5);
				list.editList(2);
				break;

			}
			}
		}
		 c = MyKeyb.isPressed(x, y);
		if (c > 0)TWin.write(c);
	}
	return c;
}

void loop() {
	MWin.handleMenus();
//	curr_m = MWin.curr_m;
//	Serial.println(curr_m);
	//char c = touch_control();
	delay(20);
}
/*
	static boolean keyb_on;
	uint16_t x, y;
	if (touch.isTouching())
	{
		touch.getPosition(x, y);
		int ind = MWin.getPressed(x, y);
		if (ind > 0) {
			TWin.println(ind);//select menu function case of
			switch (ind) {

			case 2:
				if (!keyb_on) {
					MyKeyb.init(10, 50, 0); keyb_on = true;
					break;
				}
				 else {
					   MyKeyb.end(); keyb_on = false;
					   break;
				   }
			case 11: {
				for (byte i = 0; i <  N_curves; i++)
					myGraph[i].scroll( 10.);
				for (byte i = 0; i <  N_curves; i++) {
					myGraph[i].draw();
					myGraph[i].drawAxX(0, 1.,0);

				}
				break;
			}
			//<
		case 12: {                 //>
			for (byte i = 0; i <  N_curves; i++)
				myGraph[i].scroll(-10.);
			for (byte i = 0; i <  N_curves; i++)
			{
				myGraph[i].draw();
				myGraph[i].drawAxX(0, 1.,0);
			}
			break; }
		case 13: {

			
			//+
			for (byte i = 0; i <  N_curves; i++)
				myGraph[i].changeScaX(1.2);
			for (byte i = 0; i <  N_curves; i++)
				myGraph[i].draw();
			
			break; }
		
		case 14: {
			for (byte i = 0; i < N_curves; i++)
				myGraph[i].changeScaX(0.8);
			for (byte i = 0; i <  N_curves; i++)
				myGraph[i].draw();

			break; }//-
			}
		}
		char c = MyKeyb.isPressed(x, y);
		if (c > 0)TWin.write(c);
	}

	*/
