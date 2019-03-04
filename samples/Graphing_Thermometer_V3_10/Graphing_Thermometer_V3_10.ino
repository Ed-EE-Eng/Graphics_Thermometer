//===========
//	20190110	E.Andrews	Rev 2.01	Fixed buffer over-run during acqusisition
//	20190121	E.Andrews	Rev	3.00	Add 4-Button Membrane Switch Option, Add RTC Option		
//	20190130	E.Andrews	Rev	3.01	Add 4-Button Membrane Switch Option, Add RTC Option
//										RTC Working, PB Switch Working, LONG TERM STORAGE not implemented
//	20190131	E.Andrews	Rev 3.02	Add TIME_OF_DAY (TOD) x-axis labeling of TREND graph
//	20190216	E.Andrews	Rev 3.03	General Code Cleanup
//										Remove unneeded Diag Print Statements
//										Fix Graphic Spill over when MARKER is at the very top or bottom of graphing area
//										Fix scaling but when Delta_T exceeds 50 DegF
//										Check and Text DegC option
//										Reassign I/O pins to agree with final article schematic
//	201902216	E.Andrews	Rev 3.04	Attempt to Fix Graphic Spill over Try #2 (when MARKER is at the very bottom of graphing area)
//										Fix scaling but when Delta_T exceeds 50 DegF
//										Check and Text DegC option
//										Reassign I/O pins to agree with final article schematic
//	20190222	E.Andrews	Rev 3.05	Plotting error bug fixes
//	20190224	E.Andrews	Rev 3.06	Fix Humidity full screen rewrite bug that occured right after a Time_Set operation
//	20190301	E.Andrews	Rev 3.10	General code clean up and first GitHub release
//===========

//Keep name & version updated HERE=======vvvv
#define ProgInfo  "Graphing_Thermometer_V3_10"
#define CompileDate __DATE__
#define CompileTime __TIME__

//=========== BEGIN DIAGNOSTIC/CONFIGURATION SWITCHES AND CONSTANTS =========v

//#define DiagInitialze_Tdata_WithTestData	//Used for testing during development
//#define DiagPrintEnabled					//Used for testing during development


//Include Date and Time functions using DS3231 connected via I2C and Wire Lib
#define RTCpresent
#include <RTClib.h>				//Download & install this lib from: https://github.com/adafruit/RTClib
RTC_DS3231 rtc;					//Create instance of Library called 'rtc'

#include <Adafruit_GFX.h>   	// Core graphics library
#include <MCUFRIEND_kbv.h>  	// Hardware-specific library to connect 480x320 3.5" LCD Display to GFX Library 
MCUFRIEND_kbv tft;				//Create an instance of MCUFRIEND_kbv called "tft"

#include <Bounce2.h>			//This is a general use switch debouncing library
								//https://github.com/thomasfredericks/Bounce2
//These fonts are part of the AdaFruit_GFX library...We are including just the ones needed for this App.								
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
//===Other fonts available but NOT USED in this program
//#include <Fonts/FreeSerif12pt7b.h>
//#include <Fonts/FreeSansBoldOblique24pt7b.h>
//#include <Fonts/FreeSansBold24pt7b.h>
//#include <Fonts/SanSerif_Monospc_48pt_ea.h>
//#include <Fonts/SanSerif_Monospc_60pt_ea.h>
//#include <Fonts/SanSerif_Monospc_96pt_ea.h>

//============ SPECIAL CUSTOM FONT CREATED FOR GRAPHINC_THERMOMETER PROGRAM =====v
//This is a Special font that was created for nice looking, 40_pt BIG DIGITAL DISPLAY
//This font-file must reside in the same directory as the mainline.ino file!
#include "droid_mono_40pt7b.h"	// NOTE: This font is just defined for ASCII 0x20 (‘SPACE’) -  0x40 (‘@’)
								// SPACE ! ” # $ % & ` ( ) * + , - / 0 1 2 3 4 5 6 7 8 9 : < = > ? @ 
//============ END SPECIAL CUSTOM FONT INCLUDE...=================================^

//============ Color Definitions for use w/AdaFruit_GFX & MCUFRIEND Libraries=====v
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410
#define	BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
// New color definitions.  thanks to Bodmer
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_DARKERGREY  0x4A49      /*  75,  75,  75 */		//This is a little 'darker' DARKGREY than Bodmer definition...
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFC9F
//=========End Color Definitions for use w/AdaFruit_GFX & MCUFRIEND Libraries=====^


//=========== CONFIGURATION/PREFERENCES SECTION =============================v
// 	SCREEN COLOR PREFERENCES 
const uint16_t GBL_Graph_BG = TFT_DARKERGREY;	//Sets 'grey' background color of grided-graph area
const uint16_t GBL_Indoor_Temp_FG = TFT_CYAN;	//Color for INDOOR digital readout
const uint16_t GBL_Outdoor_Temp_FG = TFT_GREEN;	//Color for OUTDOOR digital readout & Graph Line Color

//	TEMPERATURE UNITS PREFERENCE
//	Digital displays and Graphing defaults to showing DegF; 
//	if you want to show temperatures in DegC, set ShowDegC to "true"
#define ShowDegC false						//false = Show DegF, true = Show DegC

// TIME-DATE FORMAT PREFERENCES (See also "BuildDayDateTimeText(...) routine for additional explanations)
const uint8_t _12Hr = 0;		//Use 12-Hr format.				Example: Wed   Feb 28   10:47
const uint8_t _Secs = 1;		//Include SECONDS in display 	Example: Wed   Feb 30   10:47:23
const uint8_t _YYYY = 2;		//Include 4-digit YEAR.  		Example: Wed   Feb 28, 2019   10:47
const uint8_t _AMPM = 4;		//Include AM/PM indication. 	Example: Wed   Feb 30   10:47 AM
const uint8_t _24Hr = 8;		//Use 24-Hour format.			Example: Wed   Feb 30   14:47
								//Note: _24Hr will over-ride _12Hr and _AMPM flag values
//Set the time format code you want to see by 'summing-up' the various independent flag values
const uint8_t GBL_TimeFormatCode= _12Hr + _AMPM;	//Just sum up the format codes you want to see!

// SET MAX TREND TIME TO BE STORED INTO TREND-MEMORY RAM
// Normal configuration setting   Pick ONE and ONLY ONE!  (Usual value is 25.5 (this is number to use for a 24 Hr max trend time )
// Define total time to be captured in raw data buffer (GBL_MaxDataHours)
// DATA BASE sample time will be automatically achieve the MaxDataHours values set
// NOTE: Uncomment out only ONE of the following value pairs
const float GBL_MaxDataHours = 25.5;				//Set 1 Day  (Enough for  24 Hrs), NOTE: Data Base Sample Time = ~60 Sec
const int GBL_MaxDisplayHours = 24;

//const float GBL_MaxDataHours = 2* 25.5;			//Set 2 Days (Enough for 48 Hrs),  NOTE: Data Base Sample Time = ~120 Secs
//const int GBL_MaxDisplayHours = 48;

//const float GBL_MaxDataHours = 7* 25.5;			//Set 7 days (Enough for 168 Hrs), NOTE: Data Base Sample Time = ~7 min
//const int GBL_MaxDisplayHours = 168;														

//=========== END CONFIGURATION/PREFERENCES SECTION ==========================^


//====== BEGIN DHT11,21,22 SENSOR SETUP PIN ASSIGNMENTS (Temp & Humidity)=======
#include <DHT.h>
// Connections for DHT11,21,22 Sensors... 
//	Typ Wire Color	Usage		Notes (Wire color may vary!)
//	--------------	-----------	-------------------------------------------
//     RED (Vcc) 	3 - 5V		Connect to 3VDC for DUE, 5VDC Mega
//     YEL (Data) 	Data_Pin	Connect to Data Sensing Pin [See pin assignments below]
//     BLK (Gnd) 	GND			Connect to GND pin on processor


//PIN ASSIGNMENTS - Starting w/Ver 3.03	20190216
int In_DhtPin = 23;			//Indoor DHT Sensor Mega Pin Assignment
int Out_DhtPin = 25;		//Outdoor DHT Sensor Mega Pin Assignment
//		
// Uncomment just the type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE DHT21		// DHT 21 (AM2301)	
//#define DHTTYPE DHT22		// DHT 22  (AM2302), AM2321

//Define pin and sensor type
DHT In_DHT(In_DhtPin,DHTTYPE);		//create an instance for a DHT11 indoor sensor called In_DHT
DHT Out_DHT(Out_DhtPin,DHTTYPE);	//create an instance for a DHT11 indoor sensor called Out_DHT
//======= END DHTxx SENSOR SETUP ==================================================== 


//======= NEW 4-BUTTON DEFINITIONS (Starting w/Ver 3.03 20190216)
int const PB1pin=43;	//Use D37,39,41,43,45 when plugging 4-position membrane switch to "U-Turn-Connectors"
int const PB2pin=45;	//as ribbon connector from the button array is easily plugged onto these 5 adjacent pins 
int const PB3pin=39;	// Note: Port assignments depends on wiring of membrane switch
int const PB4pin=41;	//       and is NOT "Sequential" as you might guess. Use an ohm-meter
						//       to decipher & check your particular button array wiring.
int const PBcomPin=37;	//This pin is 'COMMON' for 4-position membrane button array
						//and must be set LOW in software for button sensing to work.
//======= END 4-BUTTON PIN DEFINITIONS

//.........DEFINE 4-BUTTON CONTROL PANEL - Push Buttons for clock-set & other functions
//	These buttons use <bounce2.h)> library
//  [Comment-out next four lines if no Push buttons are connected so that PB code will be omitted from build]
//								//	Button Array when mounted horizontally and viewed from REAR panel
#define IncrPB_Pin PB4pin		//    +-------+	+-------+ +-------+ +-------+ 
#define DecrPB_Pin PB3pin		//    | [PB1] | | [PB2] | | [PB3] | | [PB4] |
#define NxtPB_Pin PB2pin		//    | ENTER | |  NEXT | |  DECR | | INCR  | 
#define EnterPB_Pin PB1pin		//    +-------+ +-------+ +-------+ +-------+
								//	Button Array when mounted vertically and viewed from FONT panel
								//   +-------- ~ -------+ +-------+
								//   |					| | [PB4] |		For Clock Set
								//   |					| | INCR  |
								//   |					| +-------+
								//   |					| +-------+
								//   |		L C D 		| | [PB3] |		For Clock Set
								//   |					| | DECR  |
								//   |	S C R E E E N	| +-------+
								//   |					| +-------+
								//   |					| | [PB2] |		For Clock Set & step-trend-plot-period
								//   |					| | NEXT  |
								//   |					| +-------+
								//   |					| +-------+
								//   |					| | [PB1] |		For Clock Set
								//   |					| | ENTER |
								//   +-------- ~ -------+ +-------+

//.........Declare GLOBAL PUSH BUTTON variables
//	These are Global PUSH BUTTON variables that enable the button scanning routines to be called
//	from anywhere in the mainline or it's subroutines.  Note: While defined, values are NOT USED
// 	if PB_pin_numbers are not defined.

unsigned long PB_RepeatMs;
unsigned long PB_LastPushedMs;						//Define "LastPushedMs" TimeStamp

 
const long PB_RepeatInitialDelayMs=900;	//Initial Delay in milliseconds (900 means an initial delay of 0.9 Second before repeat begins)
const long PB_RepeatRatePeriodMs=330;	//Repeat Rate Period in milliseconds (330 means a repeat rate about 3 x per second)

int PB_Last_IncrBtn_value=HIGH, PB_IncrBtn_value=HIGH;
int PB_IncrBtn_UpEdge=LOW;
int PB_IncrBtn_DownEdge=LOW;

int PB_Last_DecrBtn_value=HIGH, PB_DecrBtn_value=HIGH; 
int PB_DecrBtn_UpEdge=LOW; 
int PB_DecrBtn_DownEdge=LOW;

int PB_Last_NxtBtn_value=HIGH, PB_NxtBtn_value=HIGH; 
int PB_NxtBtn_UpEdge=LOW; 
int PB_NxtBtn_DownEdge=LOW;

int PB_Last_EnterBtn_value=HIGH, PB_EnterBtn_value=HIGH; 
int PB_EnterBtn_UpEdge=LOW; 
int PB_EnterBtn_DownEdge=LOW;

//Initialize PushButton debounce routines ONLY IF Pins are defined
#if defined (IncrPB_Pin)
	// Instantiate a Bounce object
	Bounce debounce_IncrBtn = Bounce(); 
#endif
#if defined (DecrPB_Pin)
	// Instantiate a Bounce object
	Bounce debounce_DecrBtn = Bounce(); 
#endif
#if defined (NxtPB_Pin)
	// Instantiate a Bounce object
	Bounce debounce_NxtBtn = Bounce(); 
#endif
#if defined (EnterPB_Pin)
	// Instantiate a Bounce object
	Bounce debounce_EnterBtn = Bounce(); 
#endif
//.........END PUSH BUTTON Variables definitions

//======= Define General-Use Constants & Variables
//	Create Tdata - This is a circular buffer where raw OUTSIDE TEMPERATURE data is stored.  
//	Newest data is always placed at the GBL_Tdata[GBL_TdataNewPtr] of the array.
//	GBL_TdataNewPtr initializes at 0 and increments (+1) each time a new value is stored into the array.
//	GBL_TdataNewPtr wraps back to zero whenever GBL_TdataNewPtr>GBL_TdataSize.
//	Since this is a circular buffer, the oldest data will always be NewPtr+1. 
//	Set buffer size and declare of raw data buffer; Take care not to over run available memory!
//
//	Note: Each array is defined aa "short integer" (16 bits) and as such, each data element takes 
//	two bytes giving us a dynamic range of range of -32768 to +32767.  Data is stored in
// 'tenths of a DegC' while leads to a total temp range of -3276.8 to +3276.7 DegC 
//
const int GBL_TdataSize=1530;	//Since MEGA has only 8K of memory, normally set GBL_TdataSize = 1530 values (uses 3.06 Ram)
								//If we sample and store data every minute, 1530 = 1530 mins = 1530/60 = 25.5 Hours Total Storage used
						
short GBL_Tdata[GBL_TdataSize];	//Tdata is a 16 bit signed value that holds raw temp data in 'tenths' of a degree C.
								//Example: 275 = 27.5 DegC, -15 = -1.5 DegC
								//Usable range is +32768 To -32767 or 3,276.8 to -3,276.7 Deg C
								
short GBL_TdataNewPtr=0;		//This value always points to the Newest Data entered into the list 


const int NoData=9999;			//This CONSTANT value is loaded into the array during 'Setup' initialization &
								//whenever illegal or invalid temp readings are detected
								//NoData entries are then skipped during analysis & plotting, etc...
								
int GBL_CurDisplayHours=0;		//Used to label the graph...
//Calculate & define number of milliseconds data stores into Tdata[] Trend-memory
const float GBL_TotalDataMs=GBL_MaxDataHours*60*60*1000;	//Milliseconds = GBL_MaxDataHours X 60 Min/Hr X 60 Sec/Min X 1000 Ms/Sec
unsigned long GBL_TdataAcqMs = (GBL_TotalDataMs/float(GBL_TdataSize));	//Here is data sample rate (in ms)
//Sampling Timer
unsigned long LastSampleTime;

unsigned const long GBL_DHT_SamplePeriodMs = 2000;	//Fastest rate that we can read from DHT sensor
unsigned long GBL_DHT_LastSampleTime;				//Used to record ms() whenever we actually take a new sample from DHT sensor

bool GBL_UpdateDigitalDisplayNowFlag=true;			//Used to tell digital displays  when to update
													//Flag will trip whenever we sample the DHT sensors

//---------- GLOBAL GRAPHICS VARIABLES for communications across all graphics operations
int GBL_MaxLcdCoord_X,GBL_MaxLcdCoord_Y;	//Biggest XY coordinate supported by the display								

//---------- GLOBAL CURRENT/PRIOR TEST for display DAY-DATE-TIME display
String GBL_NewDayDateTimeText="";
String GBL_PriorDayDateTimeText="";
										
//---------- GLOBAL DATA ACQ & CURRENT-READINGS VARIABLES for use across all routines
//Plotting Frequency
bool NewPlotNeeded=true;		//Set=true whenever trend data changes and a new trend plot should be created
float GBL_TotPlotHrs;			//This value defines the total time to be shown in the Trend-Plot

//Integer version of the most current Outside Temp measurement (0.1 x degC)
int IntOutsideTemp=0;	//This is the value that will be stored into the Trend array

//Most current Outside/Inside floating point Temperature (DegC) & Humidity Readings
float F_Out_DegC,F_Out_Humidity,F_In_DegC,F_In_Humidity;

//Last known Out/In displayed value sent to the screen
//	Note: Temps can be DegC or DegF; if #ShowDegC = true, Temp readings are in DegC, else Temp readings in DegF
float F_LastDisplayedOutTemp,F_LastDisplayedOutHumidity,F_LastDisplayedInTemp,F_LastDisplayedInHumidity;


//========== Plotting Coordinates and Constants ================v
// Last Updated for 480 x 320 display 
int const ScrnWidth=480;
int const ScrnHeight=320;
int const gXstart=4;									

int const gYstart=int(float(ScrnHeight)/3)+16;			//20190124 Update - Make room for RTC display
int const gGridSize=22;									
int const gV_GridSize=22;
int const gH_GridSize=24;

int const gXend= ScrnWidth-6;							//Size for 3 pixel boarder around all sides of display
int const gYend=ScrnHeight-6;							
 
//======= End General-Use Constants & Variable Definition=======^

//======= CLOCK-CALENDAR Constants =============================v
String const DayOfWeekText[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String const MonthOfYearText[12] = {"Jan", "Feb", "Mar","Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//======= END CLOCK-CALENDAR Constants =========================^

void setup(void)
{
    Serial.begin(115200);	//Only used for diag printouts....
	//Initialize DHT sensor read routines	
	In_DHT.begin();		//Start DHT Sensor 1, Indoor temp & humidity)
	Out_DHT.begin();	//Start DHT Sensor 2, Outdoor temp & humidity
	//------------------------------------------------------------
	//	Initialize PUSH BUTTONS CODE 
	//  (Include code ONLY WHEN PB_Pins are defined, other wise SKIP it!) 
	//------------------------------------------------------------	
	//
	// Define COMMON-BUTTON pin (if used) and drive it LOW
	// (These two lines of code are Not needed if you tie BUTTON COMMON wire to GROUND)
	pinMode (PBcomPin,OUTPUT);		//Common pin must be defined as output set LOW	
	digitalWrite(PBcomPin,LOW);

	// Setup INCR button with an internal pull-up :
	pinMode(IncrPB_Pin,INPUT_PULLUP);
	// After setting up the button, setup the Bounce instance :
	debounce_IncrBtn.attach(IncrPB_Pin);
	debounce_IncrBtn.interval(5); // interval in ms

	// Setup DECR button with an internal pull-up :
	pinMode(DecrPB_Pin,INPUT_PULLUP);
	// After setting up the button, setup the Bounce instance :
	debounce_DecrBtn.attach(DecrPB_Pin);
	debounce_DecrBtn.interval(5); // interval in ms

	// Setup NXT button with an internal pull-up :
	pinMode(NxtPB_Pin,INPUT_PULLUP);
	// After setting up the button, setup the Bounce instance :
	debounce_NxtBtn.attach(NxtPB_Pin);
	debounce_NxtBtn.interval(5); // interval in ms

	// Setup ENTER the button with an internal pull-up :
	pinMode(EnterPB_Pin,INPUT_PULLUP);
	// After setting up the button, setup the Bounce instance :
	debounce_EnterBtn.attach(EnterPB_Pin);
	debounce_EnterBtn.interval(5); // interval in ms
	//------------------------------------------------------------
	//	END PUSH BUTTON SETUP
	//------------------------------------------------------------	
	//

	ResetPushButtons();
	//================== LCD Startup Goodies
    uint16_t ID = tft.readID();
    if (ID == 0xD3) ID = 0x9481;
    tft.begin(ID);
    tft.setRotation(1);	//Set Portrait Mode
	GBL_MaxLcdCoord_X =tft.width()-1;	//retrieve screen X-dimension using GFX Lib
	GBL_MaxLcdCoord_Y =tft.height()-1;	//retrieve screen X-dimension using GFX Lib

	tft.fillScreen(BLACK);	//At start up, Erase Screen to BLACK, draw boarder, set text colors
	tft.drawRect(0, 0, GBL_MaxLcdCoord_X,GBL_MaxLcdCoord_Y, YELLOW);
	tft.setTextColor(GREEN);
	
	int tX=10;
	int tY=20;
	int tSize=1;
	tft.setCursor(tX,tY);
	tft.setFont(&FreeSans9pt7b);
	tft.print(ProgInfo);tft.print(" (");tft.print(CompileDate);tft.print(")");
	int LineSpace=getTextHeight(ProgInfo)+5;
	
	//===RTC Power up checks
	tY=tY+LineSpace;
	showmsgXY(tX+5,tY, tSize, &FreeSans9pt7b, "> Checking for RTC Module...");
	if (! rtc.begin()) {
		tY=tY+LineSpace;
		showmsgXY(tX+10,tY, tSize, &FreeSans9pt7b, "ERROR - Couldn't find RTC!");
		tY=tY+LineSpace;
		showmsgXY(tX+10,tY, tSize, &FreeSans9pt7b, "ERROR - PROGRAM HALTED!");
		Serial.println("Couldn't find RTC");
		while (1);	//ALL STOP!!!	PROGRAM STOPS if RTC not found!
	}else {
		tY=tY+LineSpace;
		showmsgXY(tX+5,tY, tSize, &FreeSans9pt7b, "> RTC FOUND...");
	}

	if (rtc.lostPower()) {
		tY=tY+LineSpace;
		showmsgXY(tX+10,tY, tSize, &FreeSans9pt7b, "ERROR - BATTERY FAILED, RTC Lost Power");
		tY=tY+LineSpace;
		showmsgXY(tX+10,tY, tSize, &FreeSans9pt7b, "      - TIME Must Be Set!");
		//Serial.println("RTC lost power - Operator MUST RESET Time & Date!");
		// following line sets the RTC to the date & time this sketch was compiled
		//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
		// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}else{
		tY=tY+LineSpace;
		showmsgXY(tX+5,tY, tSize, &FreeSans9pt7b, "> RTC BAT. OK - Verify Time/Date");


	}

	DateTime now = rtc.now();

	Serial.print("Current RTC DATE/TIME is: ");
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print('/');
	Serial.print(now.year(), DEC);
	
	Serial.print(" (");
	Serial.print(DayOfWeekText[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();
	//String DateTimeText="";
	GBL_NewDayDateTimeText=String(DayOfWeekText[now.dayOfTheWeek()])+" " 
		+MonthOfYearText[now.month()]+" " 
		+String(now.day(),DEC)+", "
		+String(now.year(),DEC);

	//Convert to 12-Hour AM/PM format
	int Hour_12=now.hour();	
	String AmPm = "AM";
	if (now.hour() ==0)	Hour_12=12;
	if (now.hour()>11) AmPm="PM";
	if (now.hour()>12) Hour_12=now.hour()-12;
	GBL_NewDayDateTimeText=GBL_NewDayDateTimeText +"  "
		+String (Hour_12,DEC)
		+":";
	if (now.minute()<10) GBL_NewDayDateTimeText=GBL_NewDayDateTimeText+"0";
	GBL_NewDayDateTimeText=GBL_NewDayDateTimeText
		+String(now.minute(),DEC)
		+":";
	if (now.second()<10) GBL_NewDayDateTimeText=GBL_NewDayDateTimeText+"0";
	GBL_NewDayDateTimeText=GBL_NewDayDateTimeText
		+String(now.second(),DEC)
		+" "
		+String(AmPm);
	
	tY=tY+LineSpace;
	tY=tY+LineSpace;
	Serial.println(GBL_NewDayDateTimeText);
	//showmsgXY(tX+5,tY, tSize, &FreeSans9pt7b, String(DateText));
	Serial.println();

	
	
	
	delay(3000);	//Display power-up data for about 5 secs then go on...
	Display_OVRVW_HELP_Screen(TFT_LIGHTGREY);
	delay (15000);

	//Initialize sample timers
	LastSampleTime=millis()-GBL_TdataAcqMs-5;	//Guarantee that we will start sampling first time through the loop
	GBL_DHT_LastSampleTime=millis()-GBL_DHT_SamplePeriodMs-5;	//Guarantee that we will start sampling first time through the loop

	//Initialize DigitDisplay Update FLAG
	GBL_UpdateDigitalDisplayNowFlag=true;	//Force a digital display update right away
	
	//Initialize plot trend window to one of the following 
	// Pick the startup trend display that you want. (uncomment ONE LINE, comment-out the rest)
	//GBL_TotPlotHrs = 4.25;	//This appears to operator as a trend time of 4 Hrs	
	GBL_TotPlotHrs = 8.5;		//This appears to operator as a trend time of 8 Hrs
	//GBL_TotPlotHrs = 17;		//This appears to operator as a trend time of 16 Hrs	
	//GBL_TotPlotHrs = 25.5;	//This appears to operator as a trend time of 24 Hrs

	//Initialize Raw Data Array with "NoData" Constant
	for (int i=0;i<GBL_TdataSize;i++){
	  GBL_Tdata[i]=NoData;	//random(600,700);
	}
	
	#ifdef DiagInitialze_Tdata_WithTestData	//This will preload a up/down ramp into GBL_Tdata[] for test purposes
		 //Load test pattern into array
		short TestPattern=183;		//18.3 DegF = 60 Deg F
		short TpMax=TestPattern+10;			
		short TpMin=TestPattern-10;			
		float TpChange=.01;
		short TpDir=1;
		for (int i=GBL_TdataSize-1;i>-1;i--){
			GBL_Tdata[i]=TestPattern;
			TestPattern=TestPattern+TpChange*TpDir;	//This will make a ramp pattern in the buffer...
			if (TestPattern>TpMax||TestPattern<TpMin) {
				TpDir=-TpDir;	//Confine pattern to TpMax<data<TpMin
				TpMax=TpMax+10;
				TpMin=TpMin-10;
			}
			TpChange=TpChange+.01;
		}
		
		//DiagPrint(" **** Just finished loading test data set into array...GBL_Tdata[",GBL_TdataSize,"]");DiagPrint(" = ",GBL_Tdata[0]);
		//DiagPrintln(" which equals ",DegC_to_DegF(float(GBL_Tdata[GBL_TdataSize-1])/10.)," Deg_F");
	#endif
	

	tft.fillScreen(BLACK);				//Startup by erasing LCD screen
	tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, BLUE);	//Just for fun, plot a one pizel wide BLUE boarder to screen
	
}

void loop(void)
{
	//Update push button status
	ReadPushButtons();		
	
	//==== The DHT sensors can only be read once every 2 seconds ==============================
	//	This block of code checks the milliseconds timer to see if at least 
	//	2 seconds have elapsed since the last reading.  If yes, take a sample.
	if (millis()-GBL_DHT_LastSampleTime>GBL_DHT_SamplePeriodMs){
		//  > GBL_DHT_SamplePeriodMS have elapsed...Get new readings
		F_Out_DegC = Out_DHT.readTemperature();
		F_Out_Humidity = Out_DHT.readHumidity();
		F_In_DegC = In_DHT.readTemperature();
		F_In_Humidity = In_DHT.readHumidity();
		//Uncomment the following 4 lines so see a diagnostic printout to IDE Serial Monitor...
		// DiagPrint("SAMPLING - - F_OutTemp: ",F_Out_DegC," DegC");
		// DiagPrint(",F_OutHumid: ",F_Out_Humidity," %");
		// DiagPrint(", F_InTemp: ",F_In_DegC," DegC");
		// DiagPrintln(",F_InHumid: ",F_In_Humidity," %");		
		GBL_DHT_LastSampleTime=millis();		//Update GBL_DHT_LastSampleTime value
		GBL_UpdateDigitalDisplayNowFlag=true;	//Set flag that to Update digital displays
		//=========== Done Sampling DHT11 Temp and Humidity
	}
	//==== END DHT Sensor Read =================================================================
	
	//Convert current readings from Float to 'Integer-Tenths' Values
	IntOutsideTemp=int(F_Out_DegC*10.0);	//Scale floating point from DHT sensor to an integer value in 10ths of a degree
											//This makes it ready to put into the Trend Storage array if needed
	
	//==== BEGIN DATA LOGGING BLOCK ============================================================
	//Check timer to see if it's time to add the current temp measurement into the trend array
	if (millis()-LastSampleTime>GBL_TdataAcqMs){	//Is it time to put new data into the array?
		//-- YES
		LastSampleTime = millis();	//Update timer for next sample
		//Range check the data to be sure it is within a valid range...Must be  > -40.0 AND < 55.0 Deg_C AND != NoData
		//	...If not, skip this data point and reset timer for another try on next pass through the loop
		if (IntOutsideTemp>-400 && IntOutsideTemp<550 && IntOutsideTemp!=NoData){	
			//Data is in-range...Now Update Pointers and add a new data sample into the trend-data-array
			GBL_TdataNewPtr++;	
			if (GBL_TdataNewPtr>GBL_TdataSize-1) GBL_TdataNewPtr=0; 	//increment pointer and wrap it back to zero if needed
																		//20190110: Changed limit to GLB_TdataSize-1 to prevent buffer over-runas probable over-ran!
			GBL_Tdata[GBL_TdataNewPtr]=IntOutsideTemp;	//Store the sample into the trend array
			NewPlotNeeded=true;		//We've added new data; Replot Trend Graph to the display screen		
		}
		else {
			//Data is Out-of-range;  Skip current sample and Set timer value to try again on next pass through the loop
			LastSampleTime=millis()-GBL_TdataAcqMs-5; 	//Bad data...Force an immediate resample!
		}
		#ifdef DiagPrintEnabled	//Diagnostic code...Only dump this message to Serial Monitor when Diag... flag is defined 
			DiagPrint(" >>>>> Added value=",GBL_Tdata[GBL_TdataNewPtr],"");DiagPrint(", GBL_TdataNewPtr=",GBL_TdataNewPtr,"");
		#endif
	}
	//==== END DATA LOGGING BLOCK ==============================================================
	
	//==== BEGIN SET CLOCK BLOCK ===============================================================
	if (PB_EnterBtn_DownEdge==HIGH) {		//Did Oper push the INCR button
		ResetPushButtons();
		SetClockWithPushButtons();			//Go to clock set routine
		//Now, clear and request a redraw of the whole screen!
		tft.fillScreen(BLACK);				//Restart the display
		tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, BLUE);	
		F_LastDisplayedOutTemp=150.5;		//Force a Temp Update
		F_LastDisplayedInTemp=150.5;
		F_LastDisplayedInHumidity=-99.1;		//Force a Humidity Update
		F_LastDisplayedOutHumidity=-99.1;	
		GBL_PriorDayDateTimeText="";		//Force Date-Time Update
		NewPlotNeeded=true;					//Force a new TREND PLOT Update
		ReadPushButtons();
	}
	//==== END SET CLOCK BLOCK = ==============================================================
	
	//==== BEGIN OPER PUSH HELP BUTTON ================================================	
	if (PB_NxtBtn_DownEdge==HIGH){
		PB_NxtBtn_DownEdge=LOW;
		ResetPushButtons();
		ReadPushButtons();
	
		//Plot_OvrView_Buttons(TFT_BLUE,TFT_WHITE);
		Display_OVRVW_HELP_Screen(TFT_LIGHTGREY);
		delay(5000);
		//Now, clear and request a redraw of the whole screen!
		tft.fillScreen(BLACK);				//Restart the display
		tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, BLUE);	
		F_LastDisplayedOutTemp=150.5;		//Force a Temp Update
		F_LastDisplayedInTemp=150.5;
		F_LastDisplayedInHumidity=-99.1;		//Force a Humidity Update
		F_LastDisplayedOutHumidity=-99.1;	
		GBL_PriorDayDateTimeText="";		//Force Date-Time Update
		NewPlotNeeded=true;					//Force a new TREND PLOT Update
		ReadPushButtons();
	}
	//==== BEGIN OPER TREND PERIOD CHANGE BLOCK ================================================	
	if (PB_IncrBtn_DownEdge==HIGH){
		ChangeTrendInterval(1);		//Step Trend forward by one setting
		PB_IncrBtn_DownEdge=LOW;
		ResetPushButtons();
		ReadPushButtons();
	}
	if (PB_DecrBtn_DownEdge==HIGH){
		ChangeTrendInterval(-1);		//Step Trend Down by one setting
		PB_DecrBtn_DownEdge=LOW;
		ResetPushButtons();
		ReadPushButtons();
	}
	//==== END OPER TREND PERIOD CHANGE BLOCK =================================================
	
	//======================UPDATE TEMP & HUMIDITY DIGITAL READINGS ===========================	
	int tX=10;	int tY=50;		//Variables used to position Big Digital Displays
	float F_DisplayOutdoorTemp=0.,F_DisplayIndoorTemp=0.;	//Variables to hold display values	
	if (GBL_UpdateDigitalDisplayNowFlag){	//...UpdateDisplayNow flag is set every time a new sample is taken
		//===== Print Digital INDOOR TEMPERATURE
		if (ShowDegC) {	//Are we to display in Deg C or Deg F?
			//Deg C mode - Use raw sensor data AS-IS (No conversion needed)
			F_DisplayOutdoorTemp=F_Out_DegC;
			F_DisplayIndoorTemp=F_In_DegC;
		}	
		else {
			//DegF mode - Convert raw sensor data to Deg_F
			F_DisplayOutdoorTemp=DegC_to_DegF(F_Out_DegC);
			F_DisplayIndoorTemp=DegC_to_DegF(F_In_DegC);
		}
		//===== Update OUTDOOR TEMPERATURE Digital Display	
		tX=10;tY=65;	//Set "OUTDOORS" screen coordinate
		tX=10;tY=65+24;	//Set "OUTDOORS" screen coordinate		
		UpdateTempDisplay(tX,tY,GBL_Outdoor_Temp_FG,F_DisplayOutdoorTemp,F_LastDisplayedOutTemp);
		//===== Update OUTDOOR HUMIDITY Digital Display right below the temp reading
		UpdateHumidityDisplay(tX,tY+20, GBL_Outdoor_Temp_FG, "OUTDOOR HUMID ",F_Out_Humidity, " %", F_LastDisplayedOutHumidity);
		
		//===== Update INDOOR TEMPERATURE Digital Display	
		tX=250;tY=65;	//Set "INDOORS" screen coordinate
		tX=250;tY=65+24;	//Set "INDOORS" screen coordinate
		UpdateTempDisplay(tX,tY,GBL_Indoor_Temp_FG,F_DisplayIndoorTemp,F_LastDisplayedInTemp);
		//===== Print Digital INDOOR HUMIDITY right below the temp reading
		UpdateHumidityDisplay(tX,tY+20, GBL_Indoor_Temp_FG, "INDOOR HUMID ",F_In_Humidity," %", F_LastDisplayedInHumidity);
		

		//===== Update Day-Date-Time Display
		tX=15;tY=24;	//define screen coordinate where Day-Date-Time will be placed; refine X-pos a little later on
		
		BuildDayDateTimeText(GBL_NewDayDateTimeText,GBL_TimeFormatCode);
		//Calculate tX to center text in middle of screen
		tX=(ScrnWidth/2)-getTextWidth(GBL_NewDayDateTimeText+"WWWW")/2;	//Pad by 3 extra characters to nudge left
		//Send "New" text to UpdateDayDate... routine; Routine will only update changed text as needed
		//If time text hasn't changed since last time it was called, UpdateDayDate... routine will do nothing.

		UpdateDayDateTimeDisplay(tX,tY, TFT_DARKCYAN, GBL_NewDayDateTimeText, GBL_PriorDayDateTimeText);

		
		//We're done with all digital display updates 
		GBL_UpdateDigitalDisplayNowFlag=false;	//Reset ...UpdateFlag
	}
	//======================END DIGITAL DISPLAY UPDATES =======================================
		
	//======================UPDATE TREND PLOT (If needed, that is) ============================	
	if (NewPlotNeeded){ //Do we need to repaint the TREND PLOT?
		//YES - Update the PlotTodTrendChart. 
		//Note: Plotting location and size are global constants (set at top of program)

		PlotTodTrendChart(gXstart,gYstart,gXend,gYend,gH_GridSize,gV_GridSize,GBL_TdataAcqMs,GBL_TotPlotHrs*60*60*1000);

		
		NewPlotNeeded=false;	//Reset PlotNeeded Flag
	}
	//======================END TREND PLOT UPDATE ==============================================
	
}	//*******************************END MAINLINE LOOP ******************************************

void showmsgXY(int x, int y, int sz, const GFXfont *f, char const* msg){
	//	This routine  and sets text location, size, font, and message in one call.
	//	Text will appear in last foreground color that was set.
	//
	//	PARAMETERS:
	//		int (x,y) 			Location where text will print
	//		int sz				Specifies font size multiplier (ie: 2 = 2X font size). If omitted
	//							font size = 1 is used.
	//		chr const msg		Text message to print
	//
	//	RETURNS: NOTHING
	//
	//	01-04-2019	E.Andrews	Rev 0.0	Adopted from MCUFRIEND_kbv example program
	//
	int16_t x1, y1;
    uint16_t wid, ht;
    //tft.drawFastHLine(0, y, tft.width(), WHITE);
	//tft.drawFastHLine(0, y, x, WHITE);
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextSize(sz);
    tft.print(msg);
    //delay(1000);
}
void setFontStyle(const GFXfont *font,int fontSize, uint16_t FgColor, uint16_t BgColor){
	//	This routine sets text font, size, foreground color, & background color in one call.
	//	Note: Background color only applies to sysfont NOT other Custom fonts.
	//
	//	PARAMETERS:
	//		GFXfont font		User can specify a new font.  if omitted, font is unchanged.
	//		int fontSize		Specifies font size multiplier (ie: 2 = 2X font size). If omitted
	//							font size = 1 is used.
	//		int_16t FgColor		Text foreground color. 
	//		int_16t BgColor		Text background color (Only active for sysfont)
	//
	//	RETURNS: NOTHING
	//
	//	01-04-2019	E.Andrews	Rev 0.0	First cut
	//	
	tft.setFont(font);
	tft.setTextSize(fontSize);
	tft.setTextColor(FgColor,BgColor);
}


int getTextWidth(String Text){
	//	This routine returns the # pixels WIDE the text string plot will be on the srceen
	//	using the currently active font.
	//
	//	PARAMETERS:
	//		String Text			The ASCII string to be analyzed.
	//
	//	RETURNS: 
	//		int Width			# of pixels that string will take on the screen
	//
	//	01-04-2019	E.Andrews	Rev 0.0	First cut
	//	
	int16_t newX1=0,newY1=0,newW=0,newH=0;
	tft.getTextBounds(Text,0,100,&newX1,&newY1,&newW,&newH);
	return newW;
}
int getTextHeight(String Text){
	//	This routine returns the # pixels HEIGHT the text string plot will be on the srceen
	//	using the currently active font.
	//
	//	PARAMETERS:
	//		String Text			The ASCII string to be analyzed.
	//
	//	RETURNS: 
	//		int Height			# of pixels that string will take on the screen
	//
	//	01-04-2019	E.Andrews	Rev 0.0	First cut
	//	
	int16_t newX1=0,newY1=0,newW=0,newH=0;
	tft.getTextBounds(Text,0,100,&newX1,&newY1,&newW,&newH);
	return newH;	
}
void updateText(int iX, int iY, String newText, String priorText,uint16_t textFG,uint16_t textBG){
	//	This routine prints a new text string to the screen.  It was tested and intended for use with
	// 	"custom fonts" and as such will write BOTH the foreground as well as background font colors.
	//	This prevents the characters-piled-on-top-of-each-other, an effect that normally occurs when 
	//	the updating text using 'custom fonts'.
	//
	//	Note: Rev 0.0 was not fully tested using SYSTEM fonts!
	//
	//		Example:	String NewText[] = "This is NEW text";
	//					String OldText[] = "This was OLD text";
	//		NOTE: You MUST add the REFERENCE operator ('&') when calling this routine as shown below!
	//
	//					updateText(100,120,NewText,&OldText,BLUE,GREY)	//Note OldText passed by REFERENCE ('&')
	//																	//Upon return from this call, OldText = NewText
	//
	//	PARAMETERS:
	//		int iX,iY			Integer coordinate of lower LH corner of the first character location
	//							NOTE: When using the sysfont, this will be coordinate of the Upper LH corner!
	//		String newText		Contains the NEW-TEXT to be printed.
	//	opt	String priorText	Contains the PRIOR-TEXT present. Routine will erase the priorText area 
	//							with the background color to prevent the characters-piled-on-top-
	//							of-each-other effect. If omitted, only newText area will be erased.
	//		uint16_t textFG		Foreground text color
	//		uint16_t textBG		Background text color
	//
	//	RETURNS: 		 		priorText = newText upon return.
	//
	//	01-04-2019	E.Andrews	Rev 0.0	First cut
	//
	int16_t newX1=0,newY1=0,newW=0,newH=0;
	int16_t priorX1=0,priorY1=0,priorW=0,priorH=0;
	int16_t eraseW=0,eraseH=0;
	int newNchars,priorNchars;
	//Check to see if newText = priorText.  If they are the same just return without doing anything.
	if (newText != priorText){
		//newValue does NOT EQUAL priorValue...some change is needed

		newNchars=String(newText).length();		//Get length of newText string
		priorNchars=String(priorText).length();	//Get length of priorText string
		if(newNchars!=priorNchars){  	//Compare character lengths...
			//Number of characters is different between prior and new.
			//Do a total rewrite!  Erase the largest of the two character string zones...
			tft.setCursor(iX,iY);
			tft.getTextBounds(priorText,iX,iY,&priorX1,&priorY1,&priorW,&priorH);
			tft.getTextBounds(newText,iX,iY,&newX1,&newY1,&newW,&newH);
			if(newW>priorW) eraseW=newW; else eraseW=priorW;
			if(newH>priorH) eraseH=newH; else eraseH=priorH;
			tft.fillRect(iX-1,iY+2,eraseW+2,-(eraseH+1),textBG);	//Erase text zone to background color
			tft.setTextColor(textFG,textBG);
			tft.print(newText);										//Do a fresh print of the new text
		}
		else {
			//Number of chars is the SAME between prior and new...
			//Now, step through and compare each character, skipping update for those that are the same
			//Once you detect a difference, rewrite that character and all that follow.
			bool ReprintAll=false;
			int i=0;
					
			while (i<newNchars){
				String SnglChar[10]="";
				if(newText[i]!=priorText[i]){
					ReprintAll=true;
				}

				if(newText[i]!=priorText[i] || ReprintAll) {
					//If one character change was detected, treat all of the remaining characters as needing a background erase.
					SnglChar[0] = priorText[i];priorX1=0;priorY1=0;priorW=0;priorH=0;
					tft.getTextBounds(SnglChar[0],iX,iY,&priorX1,&priorY1,&priorW,&priorH);
					tft.fillRect(priorX1,priorY1,priorW,priorH,textBG);	//erase backaground for this character

				}
				tft.setTextColor(textFG,textBG);
				tft.setCursor(iX,iY);	
				tft.print(newText[i]);					//print new "replacement" character
														//This bumps the cursor location as needed
				iX=tft.getCursorX();
				iY=tft.getCursorY();
				i++;
			}
		}
	}
	priorText=newText;
}
 void SetClockWithPushButtons(){
	//  This routine plots a SET TIME AND  DATE screen to display and
	//	and guides the operator through the set process using the 4-push button
	//	control panel.  A four button control panel is needed.
	//
	//	Button Array when mounted vertically and viewed from REAR PANEL of enclosure
	//	--------------------------------------------------------
	//			PB1			PB2			PB3			PB4
	//		+-------+	+-------+	+-------+	+-------+
	//		|(Enter)|	|  (>)	|	|  (-)	|	|  (+)	|
	//		| ACCEPT|	| NEXT	|	|  DECR	|	| INCR	|
	//		+-------+	+-------+	+-------+	+-------+
	//
	//	Button Array when mounted vertically and viewed from FRONT panel of enclosure
	//    +-------+	   MAIN DISPLAY FUNCTIONS		|    CLOCK SET FUNCTIONS
	//    | [PB4] |		Increase Trend Period		|		Increment value
	//    | INCR  |									|
	//    +-------+									|
	//    +-------+									|
	//    | [PB3] |		Decrease Trend Period		|		Decrement value
	//    | DECR  |									|
	//    +-------+									|
	//    +-------+									|
	//    | [PB2] |		Display HELP Screen			|		Step to NEXT value
	//    | NEXT  |		(Closes after a few secs)	|
	//    +-------+									|
	//    +-------+									|
	//    | [PB1] |		Enter CLOCK SET Mode		|		Confirm changes &
	//    | ENTER |		 							|		Exit CLOCK SET Mode
	//    +-------+									|		(Auto Exit after a few 
	//												|		secs of no-button pushes)

	//	Note: This routine erases and then uses the whole LCD screen.
	//	It is up to the programmer to restore previous screens and 
	//	operations upon return from this routine.
	//
	//	Passed Parameters	NONE
	//
	//	Returns: NOTHING
	//
	//	20190127 Ver 0.0	E.Andrews	Initial cut
	//						Note: We stay in this routine until oper has completed set process 
	//						or a idle-button time-out has occurred
	//
	//Clear Screen and Start post the SET CLOCK SCREEN display.
	//
	
	//Get current time and date info...
	int ST_hour, ST_minute, ST_second, ST_month, ST_day, ST_year;
	DateTime now = rtc.now();
	ST_hour=now.hour();
	ST_minute=now.minute();
	ST_second=now.second();
	ST_month=now.month();
	ST_day=now.day();
	ST_year=now.year();
	bool ClockSetACCEPT=false;
	ClockSetACCEPT = GetTimeDateUpdatesFromOper(ST_hour,ST_minute,ST_second,ST_month, ST_day,ST_year);
	//Select message font and reset size to '1'
	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	if (ClockSetACCEPT){
		//SUCCESS MESSAGE.........................
		// Function rtc.adjust() is used to set the RTC with an explicit date & time.
		//	USAGE: rtc.adjust(DateTime(yyyy,nn,dd,hh,mm,ss));
		//	EXAMPLE: to set January 21, 2014 at 3am you would call:
		// 		rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
		rtc.adjust(DateTime(ST_year,ST_month,ST_day,ST_hour,ST_minute,ST_second));
		tft.fillRect(30,30,ScrnWidth-60,ScrnHeight-60,TFT_DARKGREEN);
		String const SuccessText="Set-Clock Completed!";
		tft.setCursor (ScrnWidth/2-getTextWidth(SuccessText)/2,ScrnHeight/2-50);
		tft.setTextColor(TFT_YELLOW);
		tft.print(SuccessText);
	}
	else{
		//CANCEL MESSAGE.......................
		tft.fillRect(30,30,ScrnWidth-60,ScrnHeight-60,TFT_RED);
		String const CancelText="Set-Clock CANCELED by Operator!";
		tft.setCursor (ScrnWidth/2-getTextWidth(CancelText)/2,ScrnHeight/2-50);
		tft.setTextColor(TFT_YELLOW);
		tft.print(CancelText);
	}
	String CurrentTimeDate;
	CurrentTimeDate="Current Time:";
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(ScrnWidth/2-getTextWidth(CurrentTimeDate)/2,ScrnHeight/2);
	tft.print(CurrentTimeDate);	
	BuildDayDateTimeText(CurrentTimeDate,GBL_TimeFormatCode);
	tft.setCursor(ScrnWidth/2-getTextWidth(CurrentTimeDate)/2,ScrnHeight/2+50);
	tft.print(CurrentTimeDate);	
	delay(5000);	//Display Set Complete message for 5 seconds
 }
 
void Display_OVRVW_HELP_Screen(int FG_TextColor){
	//	Routine to display "Help Screen".  This uses
	//	a fixed font.
	//
	//	Passed Parameters	
	//		int FG_TextColor	Foreground of instruction text.
	//
	//	Returns: NOTHING
	//
	//	20190303 Ver 0.0 		 	E.Andrews	1st pass effort
	//	
	int16_t ST_textX = 20;	//Set time Screen Position Pointer variables
	int16_t ST_textY = 70;
	const int16_t ST_textSize=1;
	//const int16_t ST_NormTextColor=TFT_LIGHTGREY;
	//const int16_t ST_HighlightTextColor=TFT_YELLOW;
	//Erase screen and plot a nice screen boarder
	tft.fillScreen(BLACK);				//Startup by erasing LCD screen
	tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, YELLOW);	//Just for fun, plot a one pixel wide BLUE boarder to screen
	//Screen Title
	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(ST_textSize);
	tft.setTextColor(FG_TextColor);	
	String const TitleText = "HELP SCREEN";
	tft.setCursor(240-getTextWidth(TitleText)/2,40);
	tft.print(TitleText);
	//Print clock set instructions...
	tft.setFont(&FreeSans9pt7b);
	tft.setCursor(ST_textX,ST_textY);
	tft.print ("1. Touch +(F1) to Increase TREND period.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("2. Touch - (F2) to Decrease TREND period.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("2. Touch NEXT (F3) to see this HELP screen.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("3. Touch ENTER (F4) to set TIME & DATE.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("-> HELP SCREEN will close in a few seconds.");
	
	//Print Program Info and Compile Date at bottom of screen "just for fun"
	int tX=20;
	int tY=GBL_MaxLcdCoord_Y-10;
	tft.setFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(TFT_DARKERGREY,TFT_BLACK);
	tft.setCursor(tX,tY);
	tft.print(ProgInfo);tft.print(" (");tft.print(CompileDate);tft.print(")");
	Plot_OvrView_Buttons(TFT_BLUE,TFT_WHITE);
 }
 void Display_SET_TIME_HELP_Screen(int FG_TextColor){
	//	Routine to display "Set Time and Date" INSTRUCTIONS screen.  This uses
	//	a fixed font.
	//
	//	Passed Parameters	
	//		int FG_TextColor	Foreground of instruction text.
	//
	//	Returns: NOTHING
	//
	//	20190121 Ver 0.0 		 	E.Andrews	1st pass effort (adapted from XYscope project)
	//	
	int16_t ST_textX = 20;	//Set time Screen Position Pointer variables
	int16_t ST_textY = 70;
	const int16_t ST_textSize=1;
	//const int16_t ST_NormTextColor=TFT_LIGHTGREY;
	//const int16_t ST_HighlightTextColor=TFT_YELLOW;
	//Erase screen and plot a nice screen boarder
	//tft.fillScreen(BLACK);				//Startup by erasing LCD screen
	//tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, YELLOW);	//Just for fun, plot a one pixel wide BLUE boarder to screen
	//Screen Title
	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(ST_textSize);
	tft.setTextColor(FG_TextColor);	
	String const TitleText = "SET DATE-TIME";
	tft.setCursor(240-getTextWidth(TitleText)/2,40);
	tft.print(TitleText);
	//Print clock set instructions...
	tft.setFont(&FreeSans9pt7b);
	tft.setCursor(ST_textX,ST_textY);
	tft.print ("1.Touch UP/DOWN (F1/F2) to incr/decr value.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("2.Touch NEXT (F3) to move to next item.");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("3.Select ACCEPT (or CANCEL) and touch");
	ST_textY+=25; tft.setCursor(ST_textX,ST_textY);
	tft.print ("  ENTER (F4) when done.");
	
	//Print Program Info and Compile Date at bottom of screen "just for fun"
	int tX=20;
	int tY=GBL_MaxLcdCoord_Y-10;
	tft.setFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(TFT_DARKERGREY,TFT_BLACK);
	tft.setCursor(tX,tY);
	tft.print(ProgInfo);tft.print(" (");tft.print(CompileDate);tft.print(")");
	Plot_ClockSet_Buttons(TFT_BLUE,TFT_WHITE);
 }
void Plot_OvrView_Buttons(int FG_Color,int BG_Color){
	//	Routine to Paint a vertical button guide on to screen
	//	This uses a fixed font ('FreeSans12pt7b') to label the buttons
	//
	//	Passed Parameters	
	//		int FG_Color	Filled Button Color
	//		int BG_Color	Button Text Color
	//
	//	Returns: NOTHING
	//
	//	20190217 Ver 0.0 		 	E.Andrews	1st pass effort
	//	
	const int Ht=70;
	const int Wdth=Ht;
	const int Radius=10;
	const int ButSpace = 10;
	int xPos =480-Wdth-ButSpace, yPos=ButSpace/2;	
	
	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BG_Color);
	
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+30,yPos+Ht/3+2);
	tft.print ("+");
	tft.setCursor(xPos+3,yPos+3*Ht/4+2);	
	tft.print ("Trend");
	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+30,yPos+Ht/3+2);
	tft.print ("-");
	tft.setCursor(xPos+5,yPos+3*Ht/4+2);
	tft.print ("Trend");
	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+10,yPos+Ht/2+5);
	tft.print ("Help");

	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+15,yPos+Ht/3+2);
	tft.print ("Set");
	tft.setCursor(xPos+7,yPos+3*Ht/4+2);
	tft.print ("Clock");	
}
	
void Plot_ClockSet_Buttons(int FG_Color,int BG_Color){
	//	Routine to Paint a vertical button guide on to screen
	//	This uses a fixed font ('FreeSans12pt7b') to label the buttons
	//
	//	Passed Parameters	
	//		int FG_Color	Filled Button Color
	//		int BG_Color	Button Text Color
	//
	//	Returns: NOTHING
	//
	//	20190217 Ver 0.0 		 	E.Andrews	1st pass effort
	//	
	const int Ht=70;
	const int Wdth=Ht;
	const int Radius=10;
	const int ButSpace = 10;
	int xPos =480-Wdth-ButSpace, yPos=ButSpace/2;	
	
	tft.setFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BG_Color);
	
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+30,yPos+Ht/2+2);
	tft.print ("+");
	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+30,yPos+Ht/2+2);
	tft.print ("-");
	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+10,yPos+Ht/2+5);
	tft.print ("Next");
	
	yPos+=Ht+ButSpace;
	tft.fillRoundRect(xPos,yPos,Ht,Wdth,Radius,FG_Color);
	tft.setCursor(xPos+5,yPos+Ht/2+5);
	tft.print ("Enter");	
}	
short GetDigitCountOfAnInteger(int Value){
	
	int Ndigits=1;			//Min Digit Count value
	if (Value<0) Ndigits++;	//Add a digit space to accommodate a negative sign
	Value=abs(Value);		//Evaluate all numbers as though they are positive
	while (Value/10>0){
		Ndigits++;
		Value=Value/10;
	}
	return Ndigits;	
}
bool GetNumericInputFromOper(int iX, int iY, int &Value,int MaxValue,int MinValue,int FG_TextColor,int BG_TextColor,int HL_TextColor ){
	//	Routine to get Number from Oper using 4-position Keypad.  Routine will "time-out" and
	//	return as 'false' if oper fails to touch ENTER key within 30 secs. Routine returns
	//	'true' if oper completes data entry.  Updated value will return in 'Value' by reference.
	//
	//	NOTE: This routine only operates properly when the current font is MONOSPACED!
	//	
	//	Passed Parameters	
	//		int iX, iY				Screen Coordinate of numeric entry
	//		int Value [By Ref]		Starting/Returned value to be changed (if out of range, MinValue will be used)
	//		int MinValue, MaxVale	Upper/Lower Bounds of input
	//		int FG_TextColor		Non-highlight text color
	//		int BG_TextColor		Background Text Color
	//		int HL_TextColor		Highlight Text Color
	//
	//	Returns: 	false			Data Entry Time Out, "enter button" never pressed, no data entered
	//				true			Data entry completed, 'Value' now has been (or may have been!) updated by Oper
	//
	//	20190121 Ver 0.0 		 	E.Andrews	1st pass effort (adapted from XYscope project)
	//	
	bool ReturnStatus=false;
	int InitialValue=Value;		//grab starting value and remember it
	if (InitialValue>MaxValue)InitialValue=MaxValue;
	if (InitialValue<MinValue)InitialValue=MinValue;
	
	//Get char width and Height of a single character for the currently active font
	short charWidth = getTextWidth("W");	//Note: "W" is a wide character and enables the routine to 'sort-of-work' with Proportional Text
	short charHt=getTextHeight("W");
	//figure out how many character spaces is needed for this entry
	int Ndigits = 0;	
	Ndigits=GetDigitCountOfAnInteger(MaxValue);
	if (GetDigitCountOfAnInteger(MinValue)>Ndigits) Ndigits=GetDigitCountOfAnInteger(MinValue);
	//Ndigits now holds the number of characters we need to express the biggest number we will ever display
	//Erase screen in active zone to support Ndigits...
	tft.fillRect(iX,iY,charWidth*Ndigits,-charHt,BG_TextColor);
	//Now display the current value, right justified and Highlighted!
	tft.setCursor(iX + (Ndigits-GetDigitCountOfAnInteger(InitialValue)),iY);
	tft.setTextColor(HL_TextColor,BG_TextColor);	//Print value in Highlighted color
	tft.print(InitialValue,DEC);
	
	bool StayInLoop=true;
	bool ChangeFlag=true;
	//Now enter a loop to accept and modify the value with every key Incr/Decr keypress...End when oper touches ENTER or we timeOUT
	const long MaxIdleTime_ms=20000;	//Set Max Timeout value to 30 sec
	unsigned long TimeOfLastButtonPress = millis();
	
	while (StayInLoop && (millis()-TimeOfLastButtonPress < MaxIdleTime_ms) ) { 	//Stay in loop until NEXT or ENTER button pushed or idle-timeout expires
	//while (StayInLoop){ //Stay in loop until NEXT or ENTER button pushed or idle-timeout expires
		//check all push buttons
		ReadPushButtons();
		
		if (PB_IncrBtn_DownEdge==HIGH){ //Increase Button Pushed
			PB_IncrBtn_DownEdge=LOW;	//Reset the edge detector
			TimeOfLastButtonPress = millis();	//Reset idle-Timeout logic
			InitialValue++;
			if (InitialValue>MaxValue) InitialValue=MinValue;	//Allow Roll-over
			if (InitialValue<MinValue) InitialValue=MaxValue;	//Allow Roll-over
			ChangeFlag=true;
		}
		if (PB_DecrBtn_DownEdge==HIGH){ //Decrease Button Pushed
			PB_DecrBtn_DownEdge=LOW;	//Reset the edge detector
			TimeOfLastButtonPress = millis();	//Reset idle-Timeout logic
			InitialValue--;
			if (InitialValue>MaxValue) InitialValue=MinValue;	//Allow Roll-over
			if (InitialValue<MinValue) InitialValue=MaxValue;	//Allow Roll-over
			ChangeFlag=true;
		}
		if (PB_NxtBtn_DownEdge==HIGH){	//Next Button Pushed
			PB_NxtBtn_DownEdge=LOW;		//Reset the edge detector
			ReturnStatus=true;
			StayInLoop=false;			//All done, exit routine
		}
		if (PB_EnterBtn_DownEdge==HIGH){
			PB_EnterBtn_DownEdge=LOW;	//Reset the edge detector
			ReturnStatus=true;
			StayInLoop=false;			//All done, exit routine		
		}
		
		if (ChangeFlag){
			//A change in value has been detected; Erase screen in active zone (charWidth * Ndigits)
			tft.fillRect(iX-2,iY+2,charWidth*Ndigits+6,-(charHt+4),TFT_RED);//BG_TextColor);
			//Now write the updated value (right justified and Highlighted) to the screen
			tft.setCursor(iX + (Ndigits-GetDigitCountOfAnInteger(InitialValue)),iY);
			tft.setTextColor(HL_TextColor,BG_TextColor);	//Print value in Highlighted color
			for (int i=0;i<(Ndigits-GetDigitCountOfAnInteger(InitialValue));i++) tft.print("0");	//Print leading Zeros...
			tft.print(InitialValue,DEC);
			Value = InitialValue;
			ChangeFlag=false;	//reset change flag
		}
		
	}
	tft.fillRect(iX-2,iY+2,charWidth*Ndigits+6,-(charHt+4),BG_TextColor);	//Blank out/Erase Highlighted text area we used...
	tft.setCursor(iX,iY);	//Restore cursor to init position
	ResetPushButtons();
	return ReturnStatus;	//Tell the caller status upon return (false = timeout, true = normal, Oper touched NEXT or ENTER)

}
bool GetTimeDateUpdatesFromOper(int &ST_hour, int &ST_minute, int &ST_second, int &ST_month, int &ST_day,int &ST_year){	
	//	Routine to display and accept operator updates to "Set Time and Date" . A STATE machine steps the Oper through
	//	each time and date variable and the presents and ACCEPT or CANCEL option. 
	//	
	//	Note: This routine MUST have a 4-station push-button switch (INC, DEC, NEXT, ENTER) to which 
	//	to witch the operator makes entries and changes.
	//
	//
	//	Passed Parameters	
	//		ST_hour		Integer, 0-23 Hours (values 0-11 are AM, values 12-23 are PM)
	//		ST_minute	Integer, 0-59 Minutes
	//		ST_second	Integer, 0-59 Seconds
	//		ST_month	Integer, 1-12 Month
	//		ST_day		Integer, 1-31 Day of Month	
	//		ST_year		Integer, 00-99 Year
	//
	//		Caller should initialize the above variables to CURRENT TIME-DATE values as the display will then start 
	//		with those values.
	//
	//	Returns By Reference: 
	//		All of the above ST_... values are PASSED BY REFERENCE, modified by this routine, and therfore updated and 'returned' with changes made by the operator.
	//
	//	Return by Function: 	Bool true or false
	//		Returns true if Oper has keyed in ACCEPT, returns false if Oper has keyed in CANCEL OR routine has TIMED-OUT
	//
	//	20190121	Ver 0.0	E.Andrews	1st pass effort (adapted from XYscope project)
	//	20190130	Ver 1.0	E.Andrews	Refactored and added state machine to make this a single call process
	//	
	//Now enter a loop to accept and modify the value with every key Incr/Decr keypress...End when oper touches ENTER or we timeOUT
	const long MaxIdleTime_ms=20000;	//Set Max Timeout value to 30 sec
	unsigned long TimeOfLastButtonPress = millis();
	
	tft.fillScreen(BLACK);				//Startup by erasing LCD screen
	tft.drawRect(0, 0, GBL_MaxLcdCoord_X+1,GBL_MaxLcdCoord_Y+1, YELLOW);	//Just for fun, plot a one pizel wide BLUE boarder to screen
	tft.drawRect(1, 1, GBL_MaxLcdCoord_X-1,GBL_MaxLcdCoord_Y-1, YELLOW);		//Make boarder a little wider...
	Display_SET_TIME_HELP_Screen(TFT_LIGHTGREY);
	//Wait a few secs for control panel to settle down
	delay(2000);
	ResetPushButtons;
	
	//	ST_State controls the parameter capture process...
	//	0=Hr, 1=Min, 2=Second 3=Mnth, 4=Day, 5=Yr, 6=ACCEPT/CANCEL.  All other values will just generate the UNHIGHLIGHTED the display	
	int ST_State = -1;	//initialize ST_State to -1 (State '-1' just paints the current values to the screen from which changes will commence)
	bool ReturnStatus = false;

	const int16_t ST_textSize=1;
	const int16_t ST_NormTextColor=TFT_LIGHTGREY;
	const int16_t ST_HighlightTextColor=TFT_YELLOW;

	int16_t const startY=190;
	
	int16_t ST_textX = 0;		//Set time Screen Position Pointer variables
	int16_t ST_textY = startY;
	
	//tft.setFont(&FreeSans12pt7b);	//12 Pt Font	
	tft.setFont(&FreeMono24pt7b);	//use 24 Pt MONOSPACED font to make manuvering easier
	tft.setCursor(ST_textX, ST_textY);	
	int CharWide = getTextWidth("X");	//Get the height and width of this monospaced char set
	int CharHt=getTextHeight("X");
	
	//Time Format is 8 characters wide (ie: HH:MM:SS")
	//Center time in SCREEN
	ST_textX = ScrnWidth/2-5*CharWide;
	tft.setCursor(ST_textX, ST_textY);
	
	
	while (ST_State < 6) {	
		ST_textX = ScrnWidth/2-5*CharWide;	//initialize to starting X-Y location
		ST_textY = startY;					
		tft.setCursor(ST_textX, ST_textY);
		
		
		if(ST_State==0) {	//=========================Get new HOUR input from Oper==========================
			//Get Opr Input for New Hour...
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_hour,23,0,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);
		}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		//Print Hour
		if (ST_hour<10) {
			tft.print("0");
		}
		tft.print(String(ST_hour,DEC));
		tft.setTextColor(ST_NormTextColor);
		tft.print (":");
		
		
		ST_textX+= 3*CharWide;
		tft.setCursor(ST_textX, ST_textY);
		
		if(ST_State==1) {	//=========================Get new MINUTE input from Oper==========================
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_minute,59,0,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);
			}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		//Print Minute
		if (ST_minute<10) {
			tft.print("0");
		}
		tft.print(String(ST_minute,DEC));
		tft.print (":");
		
		
		ST_textX+= 3*CharWide;
		tft.setCursor(ST_textX, ST_textY);
		if(ST_State==2) {	//=========================Get new SECONDS input from Oper==========================
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_second,59,0,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);
		}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		//Print Seconds
		if (ST_second<10) {
			tft.print("0");
		}
		tft.print(String(ST_second,DEC));
		
		//Reset coord for next line of info...
		ST_textY+=(CharHt+20);
		
		//Date Format is 10 characters wide (ie: MM/DD/YYYY")
		ST_textX=ScrnWidth/2-6*CharWide;
		tft.setCursor(ST_textX, ST_textY);
		
		if(ST_State==3) {	//=========================Get new MONTH input from Oper==========================
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_month,12,1,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);
			//ST_Hilight_Flag=true;
			//tft.setTextColor(ST_HighlightTextColor);
		}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		//Print Month
		if (ST_month<10) {
			tft.print("0");
			tft.print (String(ST_month,DEC));
			tft.setTextColor(ST_NormTextColor);
			tft.print ("/");
		}
		
		ST_textX+= 3*CharWide;
		tft.setCursor(ST_textX, ST_textY);
		if(ST_State==4) {	//=========================Get new Day-of-Month (aka:ST_day) input from Oper==========================
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_day,31,1,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);		
		}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		if (ST_day<10) {
			tft.print("0");
		}
		tft.print(String(ST_day,DEC));
		tft.setTextColor(ST_NormTextColor);
		tft.print ("/");
		
		ST_textX+= 3*CharWide;
		tft.setCursor(ST_textX, ST_textY);
		if(ST_State==5) {	//=========================Get new YEAR input from Oper==========================
			TimeOfLastButtonPress = millis();
			bool OprStatus = GetNumericInputFromOper(ST_textX,ST_textY, ST_year,2099,2018,ST_NormTextColor,TFT_BLACK,ST_HighlightTextColor);
			tft.setTextColor(ST_NormTextColor);		
			}
		else{
			tft.setTextColor(ST_NormTextColor);
		}
		//Print Year
		tft.print (String(ST_year,DEC));

		//Reposition for 3rd line of text
		ST_textX= 20;
		ST_textY+=(CharHt+20);
		tft.setCursor(ST_textX, ST_textY);
		
		if (ST_State<0||ST_State>7){	//Just print ACCEPT CANCEL text on screen without highlight.
			tft.setTextColor(ST_NormTextColor);
			tft.print("ACCEPT");	//Add a couple of spaces to separate from CANCEL option	
			ST_textX+= 8*CharWide;	//Move X 9 places ("ACCEPT"+ 2 spaces)
			tft.setCursor(ST_textX, ST_textY);
			tft.print("CANCEL");
		}
		if (millis()-TimeOfLastButtonPress > MaxIdleTime_ms) ST_State=8;	//We're all Done...A time out
		ST_State++;
	}	//Done with time-date input...Let's see if Oper wants to ACCEPT the changes...
	
	ST_State=6;	//Force into ACCEPT/CANCEL State
	short LastState=5;
	bool WaitForOperAcceptOrCancel=true;
	if (ST_State>7){
		ReturnStatus=false;	//We've timed out;
		bool WaitForOperAcceptOrCancel=false;
	}
	
	while (WaitForOperAcceptOrCancel && ST_State<8&& (millis()-TimeOfLastButtonPress<MaxIdleTime_ms)){
		if(ST_State==6 && LastState!=6) {
			ST_textX= 20;
			tft.setCursor(ST_textX, ST_textY);
			tft.fillRect(ST_textX-2,ST_textY+2,getTextWidth("ACCEPT")+6,-(CharHt+4),TFT_DARKGREEN);
			tft.setTextColor(ST_HighlightTextColor);
			tft.print("ACCEPT");

			ST_textX+= 8*CharWide;	//Move X 9 places ("ACCEPT"+ 2 spaces)
			tft.setCursor(ST_textX, ST_textY);
			tft.setTextColor(ST_NormTextColor);
			tft.fillRect(ST_textX-2,ST_textY+2,getTextWidth("CANCEL")+6,-(CharHt+4),TFT_BLACK);
			tft.print("CANCEL");
			LastState=6;
		}
		if(ST_State==6) {
			//Wait for Oper Input
			ReadPushButtons();	//Only NEXT and ENTER is acceptable input
			if (PB_IncrBtn_DownEdge==HIGH) {
				ST_State=7;					//Go Forward
				PB_IncrBtn_DownEdge=LOW;	//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_DecrBtn_DownEdge==HIGH) {
				ST_State=7;					//Go Forward
				PB_DecrBtn_DownEdge=LOW;	//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_NxtBtn_DownEdge==HIGH) {
				ST_State=7;					//Go Forward
				PB_NxtBtn_DownEdge=LOW;	//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_EnterBtn_DownEdge==HIGH){
				WaitForOperAcceptOrCancel=false;	//All Done!
				ReturnStatus = true;				//Oper selected ACCEPT	
				PB_EnterBtn_DownEdge=LOW;			//reset button edge flag
				TimeOfLastButtonPress = millis();
				ST_State=8;							//'State 8' = ALL DONE!
			}
		}
		if(ST_State==7 && LastState!=7) {
			ST_textX= 20;
			tft.setCursor(ST_textX, ST_textY);
			tft.setTextColor(ST_NormTextColor);
			tft.fillRect(ST_textX-2,ST_textY+2,getTextWidth("ACCEPT")+6,-(CharHt+4),TFT_BLACK);
			tft.print("ACCEPT");
			ST_textX+= 8*CharWide;	//Move X 8 places ("ACCEPT"+ 3 spaces)
			tft.setCursor(ST_textX, ST_textY);
			tft.setTextColor(ST_HighlightTextColor);
			tft.fillRect(ST_textX-2,ST_textY+2,getTextWidth("CANCEL")+6,-(CharHt+4),TFT_RED);
			tft.print("CANCEL");
			LastState=7;
		}
		if(ST_State==7){
			//Wait for Oper Input	
			ReadPushButtons();	//Only NEXT and ENTER is acceptable input
			if (PB_IncrBtn_DownEdge==HIGH) {
				ST_State=6;					//Go Backward
				PB_IncrBtn_DownEdge=LOW;	//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_DecrBtn_DownEdge==HIGH) {
				ST_State=6;					//Go Forward
				PB_DecrBtn_DownEdge=LOW;	//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_NxtBtn_DownEdge==HIGH) {
				ST_State=6;					//Go back!
				PB_NxtBtn_DownEdge=LOW;		//reset button edge flag
				TimeOfLastButtonPress = millis();
			}
			if (PB_EnterBtn_DownEdge==HIGH){
				WaitForOperAcceptOrCancel=false;	//All Done
				PB_EnterBtn_DownEdge=LOW;			//reset button edge flag
				TimeOfLastButtonPress = millis();
				ReturnStatus=false;					//Oper selected CANCEL			
				ST_State=8;							//'State 8' = ALL DONE!
			}		
		}
	}
	tft.setTextColor(ST_NormTextColor);
	Serial.print (ST_State);
	if (ReturnStatus) Serial.println ("   - Hey, Oper ACCEPTED Clock-Set changes!"); else Serial.println ("  Hey, Oper CANCELLED Clock-Set changes!");
	return ReturnStatus;	//true means Oper ACCEPTED changes, false means Oper CANCELLED changes

} 

void BuildDayDateTimeText(String &NewDayDateTimeText,uint8_t Format){
	//	This routine creates a formated text string to show Day Date & Time
	//	It starts by getting the current date-time from the RTC and then 
	//	make a text string like the following:	Sat   Feb 26,2019   2:17 PM
	//	
	//	PARAMETERS
	//		String 	NewDayDateTimeText	Text string passed BY REFERENCE
	//		Short 	Format	Sets the Date-Time format to be generated as follows:
	//				General Format: Ddd  Mmm dd,yyyy hh:mm:ss AM/PM
	//			0 = Base, 12 Hour  format.  Example: "Wed   Feb 30   10:47 PM"	
	//			1 =	Include SECONDS
	//			2 = Include YEAR (4 Digits)
	//			4 = Include AM/PM (Only if applies if a 12 Hr Format selected, otherwise, ignored)
	//			8 = 24 Hour Format, Exclude AM/PM.  (Note: AM/PM switch is ignored) 
	//		  -----	
	//		   ADD	 Add up the above values to arrive at the desired format number
	//	
	//		Note: If format is NOT SPECIFIED, display will default to option 0, Example: Sat Feb 26 2:17 PM

	//	RETURNS
	//		Upon return, NewDayDateTimeText will contain the formatted string
	//
	//	01-26-2019	E.Andrews	Rev 0.0	First cut
	//	01-30-2019	E.Andrews	Rev 0.1	Add multiple format options
	//	03-01-2019	E.Andrews	Rev 0.2	Bug fix for some format options	
	
	//===== Update Day-Date-Time Display
	DateTime now = rtc.now();	//Get current time from RTC module
	//Get ready for possible 12-hour format...
	int Hour_12=now.hour();	
	String AmPm = "AM";
	if (now.hour() ==0)	Hour_12=12;
	if (now.hour()>11) AmPm="PM";
	if (now.hour()>12) Hour_12=now.hour()-12;
	//Now build the DayDateTime string that we will display
	NewDayDateTimeText=String(DayOfWeekText[now.dayOfTheWeek()])+"      " 
		+MonthOfYearText[now.month()-1]
		+" " 
		+String(now.day(),DEC);
	if((Format & 2) ==2){	//Print date with 4 digit year
		NewDayDateTimeText=NewDayDateTimeText +", "	+String(now.year(),DEC);
	}
	NewDayDateTimeText=NewDayDateTimeText +"    ";
	if ((Format & 8) ==8){
		//Use 24 Hour Format
		if (now.hour() <10){
			NewDayDateTimeText=NewDayDateTimeText + "  0" +String (now.hour(),DEC)+":";	//Add leading Zero
		} else{
			NewDayDateTimeText=NewDayDateTimeText + "  "  +String (now.hour(),DEC)+":";
		}
	} else{	
		//Use 12 Hour format
		NewDayDateTimeText=NewDayDateTimeText +"  " +String (Hour_12,DEC)+":";
	}
	//Add MINUTES
	//Do we need a leading Zero on Minutes?
	if (now.minute()<10) {
		NewDayDateTimeText=NewDayDateTimeText+"0";
	}
	NewDayDateTimeText=NewDayDateTimeText+String(now.minute(),DEC);
	if (Format & 1 ==1){ 	
		// Add SECONDS
		NewDayDateTimeText=NewDayDateTimeText+":";
		if (now.second()<10) NewDayDateTimeText=NewDayDateTimeText+"0";
		NewDayDateTimeText=NewDayDateTimeText+String(now.second(),DEC);//Add leading zero if needed
	}
	
	if ((Format & 12)==4){	//Add AMPM if if in 12 Hr Mode and if "4" is set
		NewDayDateTimeText=NewDayDateTimeText			
		+" "
		+String(AmPm);	
	}

}
void UpdateDayDateTimeDisplay(int iX,int iY, uint16_t Color, String newValue, String &priorValue){
	//	This routine prints a Text String to the screen, ie: "Wed Sept 23,2019 12:23pm".
	//	Font to be used is hard-coded as: "FreeSans12pt7b".  
	//
	//	Routine is compares newValue with priorValue and is optimized to only print CHANGED characters.
	//	This makes the display less prone to flicker, especially when printing changes.
	//
	//	PARAMETERS:
	//		int iX,iY			Integer coordinate of lower LH corner of display screen location
	//		int Color			Integer value of Color to Plot
	//		string newValue		New Text string to be displayed
	//		string priorValue	Prior Text string passed BY_REFERENCE that was last displayed  
	//
	//	UPON RETURN: 			priorValue = newValue
	//
	//	01-24-2019	E.Andrews	Rev 0.0	First cut
	//
	int16_t iX1=0,iY1=0,iW=10,iH=0;	//Working variables for this routine
	tft.setFont(&FreeSans12pt7b);	//This is the fixed font we will use...User can change if desired here!
	tft.setTextColor(Color);
	tft.setCursor(iX,iY);
	tft.setTextSize(1);	
	String newText=newValue;			//TODO get rid of this extra variable
	String priorText = priorValue;		//TODO get rid of this extra variable
	if (newValue != priorValue){
		//newValue does NOT EQUAL priorValue...some change is needed
		int newNchars;
		newNchars=newText.length();		//Get length of newText string
		int priorNchars;
		priorNchars=priorText.length();	//Get length of priorText string
		if(newNchars!=priorNchars){  	//Compare character lengths...
			//Number of characters is different between prior and new.
			//Do a total rewrite!
			tft.getTextBounds(priorText,iX,iY,&iX1,&iY1,&iW,&iH);
			tft.fillRect(iX1-20,iY1,iW+40,iH,BLACK);	//Increase erase zone width in case new iX has moved a little bit...
			tft.print(newText);	//Do a fresh print of the new text
		}
		else {
			//Number of chars is the SAME between prior and new...Most likely just the Hours and Minutes have changed
			//Now, step through and compare each character, skipping update for those that are the same
			//Once you detect a difference, rewrite that character and all that follow.
			bool ReprintAll=false;
			int i=0;
	
			while (i<newNchars){
				String SnglChar[10]="";
 
				if(newText[i]!=priorText[i]){
					ReprintAll=true;
				}
				if(newText[i]!=priorText[i] || ReprintAll) {
					SnglChar[1] = priorText[i];iX1=0;iY1=0;iW=0;iH=0;
					tft.getTextBounds(SnglChar[1],iX,iY,&iX1,&iY1,&iW,&iH);
					tft.fillRect(iX1,iY1,iW,iH,BLACK);	//erase prior character

				}
		
				tft.setCursor(iX,iY);	
				tft.print(newText[i]);					//print new "replacement" character
				iX=tft.getCursorX();
				i++;
			}
		}
	}
	priorValue=newValue;	
}
void UpdateTempDisplay(int iX,int iY, uint16_t FgColor, float newValue, float &priorValue){
	UpdateTempDisplay(iX,iY, FgColor, TFT_BLACK, newValue, priorValue);
}
void UpdateTempDisplay(int iX,int iY, uint16_t FgColor, uint16_t BgColor, float newValue, float &priorValue) {
	//	This routine prints a FLOATING POINT temperature values to the screen
	//	as a digital value with one decimal point of precision.  Font to be used
	//	is hard-coded as: "droid_mono_40pt7b".  This works best as a digital temperature
	//	display when fed with values in the range of -99.9nnn to 120.0nnn	which will actually
	//	display as -99.9 to 120.0 (one decimal point of precision!)
	//
	//	Routine is compares newValue with priorValue and is optimized to only print CHANGED digits.
	//	This makes the display less prone to flicker, especially when printing large 'droid-mono_...' digits.
	//
	//	PARAMETERS:
	//		int iX,iY			Integer coordinate of lower LH corner of display screen location
	//		int FgColor			Integer value of the foreground color of text to plot
	//		int BgColor			OPTIONAL: Integer value of the background color of text to plot
	//							If BgColor is NOT SPECIFIED, routine will use BgColor=TFT_BLACK
	//		float newValue		Value to be displayed (in 'Degrees')
	//		float priorValue	Value passed BY_REFERENCE that was last displayed  
	//
	//	UPON RETURNS: 			priorValue = newValue
	//
	//	12-29-2018	E.Andrews	Rev 0.0	First cut
	//	01-25-2019	E.Andrews	Rev 1.0	Added ability to specify an optional background color
	//
	int16_t iX1=0,iY1=0,iW=10,iH=0;	//Working variables for this routine
	tft.setFont(&droid_mono_40pt7b);
	tft.setTextColor(FgColor);
	tft.setCursor(iX,iY);
	tft.setTextSize(1);	
	String newText=String(newValue,1);			//Create text version of newValue (to 1 decimal point)
	String priorText = String (priorValue,1);	//Create text version of priorValue (to 1 decimal point)
	if (newValue != priorValue){
		//newValue does NOT EQUAL priorValue...some change is needed
		int newNchars;
		newNchars=newText.length();		//Get length of newText string
		int priorNchars;
		priorNchars=priorText.length();	//Get length of priorText string
		if(newNchars!=priorNchars){  	//Compare character lengths...
			//Number of characters is different between prior and new.
			//Do a total rewrite!
			tft.getTextBounds(priorText,iX,iY,&iX1,&iY1,&iW,&iH);
			tft.fillRect(iX1,iY1,iW,iH,BgColor);	//erase prior character
			tft.print(newText);	//Do a fresh print of the new text
		}
		else {
			//Number of chars is the SAME between prior and new...
			//Now, step through and compare each character, skipping update for those that are the same
			//Once you detect a difference, rewrite that character and all that follow.
			bool ReprintAll=false;
			int i=0;
	
			while (i<newNchars){
				String SnglChar[10]="";
 
				if(newText[i]!=priorText[i]){
					ReprintAll=true;
				}
				if(newText[i]!=priorText[i] || ReprintAll) {
					SnglChar[1] = priorText[i];iX1=0;iY1=0;iW=0;iH=0;
					tft.getTextBounds(SnglChar[1],iX,iY,&iX1,&iY1,&iW,&iH);
					tft.fillRect(iX1,iY1,iW,iH,BgColor);	//erase prior character

				}
		
				tft.setCursor(iX,iY);	
				tft.print(newText[i]);					//print new "replacement" character
				iX=tft.getCursorX();
				i++;
			}
		}
	}
	priorValue=newValue;
}
void UpdateHumidityDisplay(int iX,int iY, int FgColor, String PreTxt, float newValue, String PostTxt, float &priorValue){
	UpdateHumidityDisplay(iX,iY,FgColor, TFT_BLACK, PreTxt,newValue,PostTxt,priorValue);
}
void UpdateHumidityDisplay(int iX,int iY, int FgColor, int BgColor, String PreTxt, float newValue, String PostTxt, float &priorValue) {
	//	This routine prints a floating point Humidity value to the screen
	//	as a digital value with one decimal point of precision.  Furthermore, it supports a 
	//	'PreTxt' text string which will precede the numeric value followed by a 'postTxt' text 
	//	string which will follow the numeric value.  This routine compares newValue with priorValue 
	//	and to minimize display flicker, is optimized to only print CHANGED characters.
	//
	//	Note: This routine is is hard-coded to use: "FreeSans9pt7b".  This works best as a digital 
	//	display whose PreTxt and PostTxt elements do not vary.  This routine assumes the background
	//	colore is BLACK.  Lastly, while this routine should work well over a very large range of
	//	data values, it has only been tested with values in the range of 0.0 to 99.9.
	//
	//		Example: 	The value 12.315 will appear as 12.3
	//					The value 98.725 will appear as 98.7
	//
	//	PARAMETERS:
	//		int iX,iY			Integer coordinate of lower LH corner of display screen location
	//		int FgColor			Integer value of Color to Plot
	//		int BgColor			OPTIONAL Background color.  If not spec'd, will be TFT_BLACK will be used
	//		String PreText		Text string that precedes floating point value (ie: "HUMIDITY ")
	//		float newValue		Value to be displayed (Note, value will display with 1 decimal point or precision)
	//		String PostText		Text string that follows floating point value (ie: "%")
	//		float priorValue	Value passed by REFERENCE,that was last displayed. 
	//
	//	RETURNS: 				priorValue = newValue upon return.
	//
	//	12-29-2018	E.Andrews	Rev 0.0	First cut
	//	01-25-2019	E.Andrews	Rev 1.0	Added ability to specify an optional background color
	//	03-03-2019	E.Andrews	Rev 1.1	Text-Background-Erase Bug fix
	//
	tft.setFont(&FreeSans9pt7b);
	tft.setTextColor(FgColor);
	tft.setTextSize(1);
	tft.setCursor(iX,iY);
	String newText=PreTxt+String(newValue,1)+PostTxt;				//Create text version of newValue (to 1 decimal point)
	String priorText = PreTxt + String (priorValue,1) + PostTxt;	//Create text version of priorValue (to 1 decimal point)
	if (newValue != priorValue){
		//newValue does NOT EQUAL priorValue...some change is needed
		int newTextWidth = getTextWidth(newText);
		int priorTextWidth = getTextWidth(priorText);
		
		int newNchars=newText.length();		//Get length of newText string
		int priorNchars;
		//priorNchars=priorText.length();	//Get length of priorText string
		if(newTextWidth!=priorTextWidth){  	//Compare character lengths...
			//Number of characters is different between prior and new --- Do a total rewrite!
			//Erase the larger of the two areas...
			tft.fillRect(iX-5,iY+2,max(newTextWidth,priorTextWidth),-15,BgColor);	//Erase all previous numbers
			tft.print(newText);	//Do a fresh print of the new text
		}
		else {
			//Number of chars is the SAME between prior and new...
			//Now, step through and compare each character, skipping update for those that are the same
			//Once you detect a difference, rewrite that character and all that follow.
			bool ReprintAll=false;
			int i=0;
			iX=tft.getCursorX();
			iY=tft.getCursorY();
			while (i<newNchars){
				if(newText[i]!=priorText[i]){
					ReprintAll=true;
				}
				if(newText[i]!=priorText[i] || ReprintAll) tft.fillRect(iX,iY+2,max(getTextWidth(String(priorText[i])),getTextWidth(String(newText[i])))+2,-15,BgColor);	//erase prior character
				tft.setCursor(iX,iY);
				tft.print(newText[i]);					//print new "replacement" character
				iX=tft.getCursorX();
				i++;
			}
		}
	}
	priorValue=newValue;
}

void PlotTodTrendChart(int xStart,int yStart, int xEnd, int yEnd,int Horiz_GridSize,int Vert_GridSize,float DataAcqPeriodMs, float TotalPlotMs){
	// 	Routine to plot GBL_Tdata[] TREND chart to screen including 1) Graph Title, 2) Vertical labeling (TEMP),
	//	and 3) Horizontal labeling (Hours).  Note, GBL_Tdata[] contains signed DegC X .1 values.
	//
	//	int xStart,yStart,xEnd,yEnd
	//							Total Graph area dimensions are defined as (xStart,yStart,,xEnd,yEnd)
	//							Actual plot area = Graph_Dimensions - space req'd for Axis_Labels
	//	int Horz_GridSize 		Spacing (in pixels) between adjacent Y-Grid lines
	//	int Vert_GridSize 		Spacing (in pixels) between adjacent X-Grid lines
	//	float DataAcqPeriodMs 	Time (in milliseconds) between adjacent samples in Tdata array
	//	float TotalPlotMs		Total Time (in milliseconds) to be appear in trend display plot
	//							Note: Point-to-Point duration (in milliseconds) is TotalPlotMs/NumPointsToPlot
	//
	//	Note: 	Programmer should should properly sync TotPlotTimeHrs to Vert_GridSize (# of Vert Graph Lines & labels)
	//			or odd,irregular, or incorrect Horizontal graph labels will occur
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	20181201	E.Andrews	Rev 1.0	Significant rework to incorporate circular-buffer concept into design
	//									Changed calling parameters so allow changeable total plot times
	//	20181230	E.Andrews	Rev 2.0	Convert from UFT lib to MCUFRIEND_kbv/Adafruit_GFX library
	//	20190124	E.Andrews	Rev 2.1	Change call to provide independent H-GridSize & V-GridSize parameters
	//
	#if ShowDegC
		const bool DisplayDegF = false;	//True means display in DegF; False means display in DegC
	#else
		const bool DisplayDegF=true;	
	#endif
	//========== Variables and Constants we use to plot data to the screen =================
	//	Note: GBL_Tdata[] array always 'stays in DegC'; Only the axis labels appear in 'DegF' when DegF mode is set
	int PriorX=NoData;
	int PriorY=NoData;
	
	//Define Just the Trend Graph Plot Area & Boundary Check Limits
	//int xGraphStart=xStart+32;					//Accommodate room for Vert-Axis Labels on Left Side of grid
	//int xGraphStart=xStart+52;					//Accommodate room for Vert-Axis Labels on Left Side of grid
	int xGraphStart=xEnd-int(18*float(Horiz_GridSize));		
	int xGraphStart2=xEnd-int(17.5*float(Horiz_GridSize));
	int yGraphTop=yStart;
	int xGraphSize=xEnd-(xGraphStart);
	int xGraphSize2=xEnd-(xGraphStart2);
	int yGraphSize= yEnd-yGraphTop-22;		//Accommodate room for Horz-Axis Labels on bottom of grid
	int yGraphBottom=yGraphTop+yGraphSize;	//y-TopLimit is the same as yStart; y-Bottom Limit = Bottom of graph - (Character Ht + 3)
	int xGraphLeft=xGraphStart2;	
	
	int xGraphRight=xGraphStart+xGraphSize;		
	int NumPointsToPlot=xGraphRight-xGraphLeft;	//NumPointsToPlot on the graph
	float MilliSecPerPixel = TotalPlotMs/NumPointsToPlot;		//Used for Horz Scaling
	float PlotTimeVsDataTimeRatio = (TotalPlotMs/NumPointsToPlot)/DataAcqPeriodMs;			//Calc PlotTimeRatio
	
	int X,Y;	//Coordinates of points to plot
	
	//Define Trend_variables to hold starting coordinates and Temp Min/Max values
	int TxStart,TyStart;	//General purpose "X,Y Text-Starting-Coordinates" for use within this routine
	float Tmax,Tmin,Tstep;	//Reserve and Initialize variable to hold the Max & Min Temperature we are currently plotting (Deg C)
							//Reserve Variable to hold Deg_C/Grid line calculation scale factor
	
	int numV_lines = xGraphSize/Horiz_GridSize;
	float vGridSpacing = float(xGraphSize)/float(numV_lines);
	int numH_lines = yGraphSize/Vert_GridSize;
	float hGridSpacing = float(yGraphSize)/float(numH_lines);

	//Define variables needed during data scanning to find max, min, and average.
	int ScanMax=NoData;
	int ScanMin=NoData;
	float Yrange=20.;
	float Yavg=ScanMax;
	int TdataPlotPtr;
	//===== Scan TrendData array to find min and max data values for the points we will be showing	
	for (int i=0;i<NumPointsToPlot;i++){
		TdataPlotPtr=GetDataPointer(i,PlotTimeVsDataTimeRatio);	//Get the new data pointer
		if (TdataPlotPtr>0){	//Error check...Did we get a valid pointer value returned?	
			if (GBL_Tdata[TdataPlotPtr] != NoData){	//Only look at valid data entries
				if (ScanMax==NoData) {				//If this is the first data point we've found, set ScanMax and ScanMin to that value
					ScanMax=GBL_Tdata[TdataPlotPtr];
					ScanMin=ScanMax;
				}
				if (GBL_Tdata[TdataPlotPtr]>ScanMax) ScanMax=GBL_Tdata[TdataPlotPtr];		//Update ScanMax and ScanMin values as needed
				if (GBL_Tdata[TdataPlotPtr]<ScanMin) ScanMin = GBL_Tdata[TdataPlotPtr];	
			}
		}
	}
	Yavg= float(ScanMax + ScanMin)/2.;	//Calc the average value
	#ifdef DiagPrintEnabled
		// DiagPrint(" SSSSS Done scale-Scan, First TdataPlotPtr=",GetDataPointer(0,PlotTimeVsDataTimeRatio),",");
		// DiagPrint("Last TdataPlotPtr=",TdataPlotPtr,",");
		// DiagPrint(" MAX=",ScanMax,",");
		// DiagPrint(" MIN=",ScanMin,",");
		// DiagPrint(" Range=",ScanMax-ScanMin,"");
		// DiagPrintln(" Avg=",Yavg,"");
	#endif
	//
	//=====BEGIN AUTO SCALING ALGORITHM
	//
	// Here is where we select the correct vertical scale based on Largest and Smalled temps found in data scan
	// This is where we determine the vertical plot-scaling we will be using
	switch (ScanMax-ScanMin){	
		case 0 ... 55:		//Range = 0-5.5 DegC P-P, 8.0 DegF
			Yrange=55.;
			Tmax= float(float(Yavg+float(Yrange)/2.)+.5);
			Tstep = Yrange/numH_lines;	
			Tmin = Tmax - Yrange;
			break;
		case 56 ... 100:	//Range = 5.6-10.0 DegC P-P, 18.0 DegF
			Yrange=100.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		case 101 ... 150:	//Range = 10.1-15.0 DegC P-P, 27.0 DegF
			Yrange=150.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		case 151 ... 200:	//Range = 15.1-20.0 DegC P-P, 36.0 DegF
			Yrange=200.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		case 201 ... 250:	//Range = 20.1-25.0 DegC P-P, 45.0 DegF
			Yrange=250.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		case 251 ... 300:	//Range = 25.1-30.0 DegC P-P, 54.0 DegF
			Yrange=1050.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		case 301 ... 350:	//Range = 30.1-35.0 DegC P-P, 63.0 DegF
			Yrange=350.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;			
		case 351 ... 400:	//Range = 40.0 DegC, 72.0 DegF
			Yrange=350.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;			
		case 401 ... 450:	//Range = 45.0 DegC, 81.0 DegF
			Yrange=350.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;			
		case 451 ... 500:	//Range = 50.0 DegC, 90.0 DegF
			Yrange=350.;
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
		default:
			Yrange=700.;	//Range = 70.0 DegC, 126.0 DegF (Crazy low gain!)
			Tmax= ((Yavg+Yrange/2)/5)*5;
			Tstep = Yrange/numH_lines;
			Tmin = Tmax - Yrange;
			break;
	}
	
	#ifdef DiagPrintEnabled
		DiagPrint("       Post Scaling-scan, MAX=",Tmax,",");DiagPrint(" MIN=",Tmin,"");DiagPrint(" Range=",Tmax-Tmin,"");DiagPrintln(" Avg=",Yavg,"");
		//	DiagPrint("Max=",ScanMax,", ");
		//	DiagPrint("Min=",ScanMin,", ");
		//	DiagPrint("Yrange=",Yrange,", ");
		//	DiagPrintln("numH_lines=",numH_lines," ");
	#endif
	
	//Set axis font and then make the plot area background to grey...
	//TODO // myGLCD.setFont(SanSerif_8x14);
	tft.setFont();	//Select default font just in case... TODO Remove this backup code

	//Paint Graphics Plot area background Color
	
	tft.fillRect(xGraphStart2,yGraphTop,xGraphSize2,yGraphSize,GBL_Graph_BG);

	int H_Adjust=2;
	const int GridLineColor = BLACK;
	for (int i=0;i<numH_lines+1;i++) {
		int Vert_Axis_Lbl_Wdth=28;
		tft.drawFastHLine(xGraphStart2,yGraphTop+i*hGridSpacing,xGraphSize2,GridLineColor);
		if (i % 2==0){	//Make every other line a slightly wider. Is "i" an EVEN number?
			//Yes i==EVEN, draw a single-wide line
			tft.drawFastHLine(xGraphStart2,yGraphTop+i*hGridSpacing+1,xGraphSize2,GridLineColor);
		}
	}
	//===== DRAW VERTICAL GRID
	H_Adjust=2;	//Minor fixed hori-offset determined empirically
	for (int i=0;i<numV_lines+1;i++) {	//Draw Single-Wide Grid Line
		tft.drawFastVLine(xGraphStart+i*vGridSpacing,yGraphTop,yGraphSize,GridLineColor);
		if (i % 2==0){	//Make every other line a slightly wider.  Is "i" an EVEN number?  
			//Yes i==EVEN, draw a second line to make it double-wide line
			tft.drawFastVLine(xGraphStart+i*vGridSpacing+1,yGraphTop,yGraphSize,GridLineColor);
		}
	}
	//====== PRINT VERTICAL AXIS LABELS
	float Grid_Ymax=-10000.;
	float Grid_Ymin= 10000.;
	tft.setTextColor(WHITE,GREY);	//Foreground,Background text color
	tft.setFont(&FreeSans9pt7b);	

	
	for (int i=0;i<numH_lines+1;i++) {

		TxStart=xStart+1;

		
		float GridLabel=(Tmax-i*Tstep)/10.; 	//Grid Label value (still in DegC)
		
		//Print new label.  Note: GBL_Tdata[] array always 'stays in DegC'.
		//Only the axis labels are converted to appear in 'DegF' when "DegF-mode" is set
		int DisplayedValue = int(GridLabel+.5);		//Use this for DegC display
		if (DisplayDegF) { 					//Use this one for DegF display
			DisplayedValue = int(DegC_to_DegF(GridLabel) );
		}
		
		//Determine X-starting coordinate of label

		switch(DisplayedValue){
			case 100 ... 999:	//Three Digit Display
				TxStart=xStart+1;
			break;
			case 10 ... 99:		//Two Digit Display
				TxStart=xStart+1+8;
			break;
			case 0 ... 9:		//One Digit Display
				TxStart=xStart+1+16;
			break;
			case -9 ... -1:		//Negative One Digit Display
				TxStart=xStart+1+8;
			break;
			case -99 ... -10:	//Negative Two Digit Display
				TxStart=xStart+1;
			break;
			default:
			break;
		}
		TyStart=yGraphTop+6;
		TxStart=xStart+1;	//This eliminates the centering-stuff in the above switch()...
		tft.setCursor(TxStart,TyStart+i*hGridSpacing);
		//String NewText = String(DisplayedValue);
		String NewText;
		if(ShowDegC) NewText= String(GridLabel,1);			//DegC to 1 decimal point of precision...
		else NewText= String(DegC_to_DegF(GridLabel),1);	//DegF to 1 decimal point of precision...
		
		tft.fillRect(2,TyStart+i*hGridSpacing+2,xGraphStart2-2,-(getTextHeight(NewText)+2),TFT_BLACK);
		tft.print(NewText);
		//String OldText = "999.9";	//Init OLD TEXT
		//updateText(TxStart,TyStart+i*hGridSpacing-2, NewText, OldText,WHITE,BLACK);
		//==Works//tft.fillRect(TxStart,TyStart+i*hGridSpacing-2,20,-20,BLACK);	//Erase char area first
		//==Works//tft.print(DisplayedValue);	//Print grid value...
		#ifdef DiagPrintEnabled	
			// DiagPrint("   i=",i,", ");
			// DiagPrint("Float_GridLabel=",(float(Tmax)/10.-i*Tstep/10. +.5),", ");
			// DiagPrint("Float_DisplayedValue",DegC_to_DegF(GridLabel),", ");
			// DiagPrintln("Int DisplayedValue=",DisplayedValue," ");
		#endif
		//Find Graph Max and Min values (in DegC)
		if (GridLabel>Grid_Ymax) Grid_Ymax=GridLabel;
		if (GridLabel<Grid_Ymin) Grid_Ymin=GridLabel;
	
	}
	//Capture and scale Y Min/Max Grid values so we can plot things properly
	Grid_Ymin=Grid_Ymin*10;	
	Grid_Ymax=Grid_Ymax*10;	//This is needed because incoming data is in 10ths of a degree C
	#ifdef DiagPrintEnabled	
		// DiagPrint("   Grid_Ymin=",Grid_Ymin,", ");
		// DiagPrintln("Grid_Ymax=",Grid_Ymax," ");
	#endif	
	
	//====== PRINT NEW HORIZONTAL AXIS GRAPH LABELS
	int H_GridLabel=1;
	float H_GridLabel_F=0.0;
	float H_GridPeriodHrs_F=GBL_TotPlotHrs/(numV_lines-1);
	DateTime now = rtc.now();
	int TOD_StartHour=now.hour()+1;
	if (TOD_StartHour>23) TOD_StartHour=0;	//Range limit value to stay in 0-23 range
	if (TOD_StartHour<0) TOD_StartHour=23;	//Range limit value to stay in 0-23 range
	int LargestHourValue=0;
	//ERASE horizontal axis labels region to BLACK
	//tft.fillRect(TxStart+30,yEnd+3,xEnd-TxStart+29,-17,TFT_BLACK);
	tft.fillRect(xGraphLeft,ScrnHeight-2,xGraphSize2,-(ScrnHeight-yGraphBottom-2),TFT_BLACK);
	
		for (int i=0;i<numV_lines;i=i+2) {	//Label every other line with a numeric value
		TxStart=xEnd-9;
		H_GridLabel=H_GridLabel-1; 
		H_GridLabel_F=i*H_GridPeriodHrs_F;
		float remainder = int(10.0*H_GridLabel_F)%10;
		#ifdef DiagPrintEnabled
			//DiagPrint("   ...i=",i); DiagPrint(", H_GridLabel_Int=",H_GridLabel); DiagPrint(", H_GridLabel_Float=",H_GridLabel_F); DiagPrintln(", Remainder = ",remainder);
		#endif
		if (remainder <.001){
			//---Construct Axis Label String using RTC values
			int AxisTodNum=TOD_StartHour-int(H_GridLabel_F);
			while (AxisTodNum>23){
				AxisTodNum = 24-AxisTodNum;		//Range limit value to stay in 0-23 Hour range
			}
			while (AxisTodNum<0){
				AxisTodNum=AxisTodNum+24;		//Range limit value to stay in 0-23 Hour range
			}
			//Convert to Text in 12-hour AM/Pm format
			String AxisTodText;
			switch (AxisTodNum){
				case 0:
					if (i==0) AxisTodText="12"; else AxisTodText="12a";
					break;
				case 1 ... 11:
					if (i==0) AxisTodText=String(AxisTodNum); else AxisTodText=String(AxisTodNum)+"a";
					break;
				case 12:
					if (i==0) AxisTodText="12"; else AxisTodText="12p";
					break;
				case 13 ... 23:
					if (i==0) AxisTodText=String(AxisTodNum-12); else AxisTodText=String(AxisTodNum-12)+"p";
					break;
				default:
					AxisTodText="UKN";
					break;		
			}
			LargestHourValue=int(abs(H_GridLabel_F));
			if (i==0){	//Patch to move first label (i==0) left from right edge of graph
				tft.setCursor(xGraphRight-(i*vGridSpacing)-getTextWidth(AxisTodText)-3,yEnd-1);
			}
			else {
				tft.setCursor(xGraphRight-(i*vGridSpacing)-getTextWidth(AxisTodText)/2,yEnd-1);
			}
			tft.print(AxisTodText);
			//tft.print(TOD_Hour-H_GridLabel_F,0);
			#ifdef DiagPrintEnabled
			//DiagPrint("   >>>Printing This One, Flt_Value= ",H_GridLabel_F); DiagPrintln(", Int_Value= ",int(H_GridLabel_F));
			#endif
		}
	}
	tft.setTextColor(WHITE,GREY);	//Foreground,Background text color
	tft.setFont(&FreeSans9pt7b);
	tft.setTextSize(1);	//Make is 1X sized
	tft.setCursor(xStart+1,yEnd-1);
	//tft.print("Time>");
	
	
	//Build up Graph Label Text String.... Format like this example-> 24 HOUR TREND  Min:26.2  Max:31.7 
	GBL_CurDisplayHours=LargestHourValue;	//Remember this value so other routines can dectect a change
	String TrendText=String(LargestHourValue,DEC);
	
	TrendText+=" HOUR TREND  Min:";
	TrendText+=String(DegC_to_DegF(float(ScanMin)/10.),1);	//Add Min Temp
	TrendText+="  Max:";
	TrendText+=String(DegC_to_DegF(float(ScanMax)/10.),1);	//Add Max Temp
	TrendText+=" ";
	//Now more or less center the text inside of the grid area; This is an approximation based on mono-spaced characters...
	//Get Width of the printed text string,,,
	int TrendTextWidth = getTextWidth(TrendText);
	//Center Text in GRAPH region
	int X_TrendText= xGraphLeft+xGraphSize2/2-TrendTextWidth/2;	//Calculate Starting X-coord to center the text string (about 31 Characters)
	X_TrendText = constrain(X_TrendText,xGraphLeft,xGraphRight-TrendTextWidth); //Make sure we're in-bounds
	tft.setFont(&FreeSans9pt7b);
	tft.setTextSize(1);	//Make is 1X sized

	int16_t X_erase,Y_erase,dX_erase,dY_erase;
	tft.getTextBounds(TrendText,X_TrendText,yStart+20, &X_erase,&Y_erase,&dX_erase,&dY_erase);	//Clear background where label will be to get rid of grid lines
	uint16_t const B_Spc=3;	//B_Spc (in pixels) = extra boarder space around text to be sure enough background has been cleared
	tft.fillRect(X_erase-B_Spc,Y_erase-B_Spc,dX_erase+2*B_Spc,dY_erase+2*B_Spc,GBL_Graph_BG);	//Make text background GREY
	tft.setCursor(X_TrendText,yStart+20);
	tft.setTextColor(WHITE);
	tft.print(TrendText);	//Print graph label
	
	//--- Plot a dashed line at the freezing point ONLY IF Freezing point is On-Screen at this time
	int const FreezeDegC=0;
	//Y=map(FreezeDegC,Grid_Ymin,Grid_Ymax,yStart+(numH_lines-1)*GridSize,yStart);	//Note: (0 DegC) = Freezing point!
	Y=map(FreezeDegC,Grid_Ymin,Grid_Ymax,yGraphTop+yGraphSize,yGraphTop);	//Map Y points, Max Temp Range is from -25.0 to +115.0 DegF

	//--- See if Y-Freeze Point is on the graph or not.  If YES then plot a dashed line at 0 DegC / 32 DegF
	if ((Y<yGraphBottom+1) && (Y>yStart-1))	{	//If in range, then plot a dashed line!
		int FrzLineOffset=vGridSpacing/4;
		for (int i=xGraphLeft;i<xGraphRight-vGridSpacing;i=i+vGridSpacing){
			tft.drawLine(i+FrzLineOffset,Y,i+FrzLineOffset+vGridSpacing/2,Y,CYAN);	//Draw dashed line whose dash-length is 1/2 as wide as a vertical grid
		}
	}
	//TODO Remove this Block of Diagnostics Code
	#ifdef DiagPrintEnabled		//Diagnostics print out stuff....
	  // DiagPrintln("=== ACQ PARMAS   GBL_MaxDataHours = ",GBL_MaxDataHours," hrs");
	  // DiagPrintln("=== ACQ PARAMS      GBL_TotalDataMs = ",GBL_TotalDataMs," ms");
	  // DiagPrintln("=== ACQ PARAMS        GBL_TdataSize = ",GBL_TdataSize," Num Of Array Elements");
	  // DiagPrintln("=== ACQ PARAMS         GBL_TdataAcqMs = ",GBL_TdataAcqMs," ms");
	  // Serial.println ("");
	  // Serial.println ("");
	  // DiagPrintln("===PLOT PARAMS       GBL_TotPlotHrs = ", TotalPlotMs/1000/60/60," hrs");
	  // DiagPrintln("===PLOT PARAMS        TotPlotMs = ", TotalPlotMs," ms");
	  // DiagPrintln("===PLOT PARAMS  NumPointsToPlot = ", NumPointsToPlot," Num Of Array Elements");
	  // DiagPrintln("===PLOT PARAMS     PlotPeriodMs = ",TotalPlotMs/NumPointsToPlot," ms");
	  // DiagPrintln("===PLOT PARAMS PlotVsData Ratio = ",PlotTimeVsDataTimeRatio," ");
	  // Serial.println ("");
	#endif	
	int CurMarkerX=NoData;
	int CurMarkerY=NoData;
	
	//========== PLOT ACTUAL TREND DATA POINTS TO SCREEN
	for (int i=0;i<NumPointsToPlot;i++){
		
		TdataPlotPtr=GetDataPointer(i,PlotTimeVsDataTimeRatio);
		
		if(TdataPlotPtr>-1){		
			if (GBL_Tdata[TdataPlotPtr]!=NoData){		//Fetch data -- Skip  "NoData" values
				//We have some data to plot...Do it!
				float MilliSecOffset=float(60-now.minute())*60.*1000.;
				int xPixelOffset=int(MilliSecOffset/MilliSecPerPixel);
				//X=map(i,0,NumPointsToPlot-1,xGraphRight,xGraphLeft+1);								//Map X points, right to left
				X=map(i,0,NumPointsToPlot-1,xGraphRight-xPixelOffset,xGraphLeft+1-xPixelOffset);		//Map X points, right to left
				Y=map(GBL_Tdata[TdataPlotPtr],Grid_Ymin,Grid_Ymax,yGraphTop+yGraphSize,yGraphTop);	//Map Y points, Max Temp Range is from -25.0 to +115.0 DegF
				//Find, limit, & plot OUT-OF-RANGE points RED
				int PointColor=GBL_Outdoor_Temp_FG;
				// last minute check to keep Y-points in-bounds 
				Y=constrain (Y,yGraphTop,yGraphBottom-1);		//Final check to keep XY point inside the define graph area

				if (X>xGraphLeft+1 && X<xGraphRight+1) {	//This is LEFT/RIGHT Hard Limiting to be sure we don't plot out of bounds
															//Skip plotting point if it's out of range
															//Can not use CONSTRAIN here because it would cause a pile-up plot at edges!
					if (PriorX==NoData){	//If we don't have a priorX, then just plot a POINT
						tft.fillRect(X,Y,2,2,PointColor);		//print 2X2 box for the point
					}
					else{
						tft.drawLine(PriorX,PriorY,X,Y,PointColor);		//Plot connecting line between the points
						tft.drawLine(PriorX+1,PriorY,X+1,Y,PointColor);	//Thicken line in X
						tft.drawLine(PriorX,PriorY+1,X,Y+1,PointColor);	//Thicken line in Y				
					}
					PriorX=X;	//Remember this coordinate so connecting line is draw between the PRIOR point and the NEXT point
					PriorY=Y;
				}
				
				if(i==0){	//Draw marker triangle around the very first point (Newest Point)
					CurMarkerX=X;
					CurMarkerY=Y;
					CurMarkerY=constrain(CurMarkerY,yGraphTop+3,yGraphBottom -4);	//Make sure marker symbols never over-run graph area
					CurMarkerX=constrain(CurMarkerX,xGraphLeft+3,xGraphRight-1);	
					//Put a small triangle on the CURRENT temperature data point to distinguish it on the trend plot
					tft.fillTriangle(CurMarkerX-3,CurMarkerY,CurMarkerX+1,CurMarkerY-4,CurMarkerX+1,CurMarkerY+4,WHITE);
				}
			}
			else {
				PriorX=NoData;
				PriorY=NoData;	

			}
		} 
		else{
			#ifdef DiagPrintEnabled
			//DiagPrintln(" === Help Rosebud! GetDataPointer(",i,") returned -1!");
			#endif
		}
	}

	//========== End Screen Plotting	
}
int GetDataPointer(int PlotIndex,float ScaleFactor){
	//Routine to translate the incoming index value to a pointer into the Trend-Data array, Tdata().
	//	PARAMETERS
	//	  int PlotIndex			This is the X-location on the screen to be plotted (usually somewhere in the 0-319 or 0-480 range)
	//	  float ScaleFactor 	The ratio of PlotSamplePeriod/DataSamplePeriod (aka: a scale-factor)
	//
	//	RETURNS: 	Integer DataIndex value that points into Tdata() corresponding to the passed PlotIndex value.
	//				Returns -1 if error was detected calculating the return value
	//
	//	Error Return = -1 	This occurs if the calculated return value is too big (> GBL_TdataSize) or too small (<0).
	//
	int ReturnValue=-1;
	float Offset_From_Start_Of_Array=float(PlotIndex)*ScaleFactor-.5;	//Calculate the Offset_From_Start_Of_Array
	ReturnValue=GBL_TdataNewPtr-int(Offset_From_Start_Of_Array);		//Create the return array index value & convert back to an Integer Pointer to the nearest array element
	
	if (ReturnValue<0){								//If calculated ReturnValue is bigger than the end of the GBL_TdataSize,
		ReturnValue=GBL_TdataSize+ReturnValue;		//wrap it back around in a 'circular buffer' fashion	
		if (ReturnValue>GBL_TdataSize || ReturnValue <0) ReturnValue=-1;	//Last minute return value range check
	}																		//Return a negative number if something went wrong!
	return ReturnValue;
}
void ChangeTrendInterval(){
	ChangeTrendInterval(0);	//Call with button Num = 0 (an invalid button num!)
}
void ReadPushButtons() {
	//	Routine to read 4 push buttons and update state variables for each
	//
	//	Passed Parameters	NONE, but requires the following Global variables
	//
	//	Returns Via Global Variables: 
	//		PB_IncrBtn_UpEdge		"Increment Button" variables
	//		PB_IncrBtn_DownEdge
	//		PB_IncrBtn_value
	//		
	//		PB_DecrBtn_UpEdge		"Decrement Button" variables
	//		PB_DecrBtn_DownEdge
	//		PB_DecrBtn_value
	
	//		PB_NxtBtn_UpEdge		"Increment Button" variables
	//		PB_NxtBtn_DownEdge
	//		PB_NxtBtn_value
	//
	//		PB_EnterBtn_UpEdge		"Enter Button" variables
	//		PB_EnterBtn_DownEdge
	//		PB_EnterBtn_value
	//
	//	20180928 Ver 0.0	E.Andrews	First cut - Make button update a subroutine that can be called from anywhere)
	//	20180930 Ver 0.1	E.Andrews	Expanded from 2 to 4 buttons. Moved button REPEAT code to beginning of routine
	//									to improve initial-button-push responsiveness.
	//	20181018 Ver 0.2	E.Andrews	Updated auto-repeat code to avoid "millis() rollover issues"
	//
	
	// BEGIN Push Button  repeat logic vvvvvvvvvvvvvv
	//	Note: Only PB1 (Incr) and PB2 (Decr) are repeating keys

 	if (PB_IncrBtn_DownEdge==HIGH) {					//PB1: If we are just seeing the down edge, reset repeat counter and reset DOWN_Edge variable
		PB_RepeatMs=PB_RepeatRatePeriodMs;				//Use RepeatRatePeriodMs for next auto-repeat period
		PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
		PB_IncrBtn_DownEdge=LOW;
	}	
	if (PB_IncrBtn_value==LOW && millis()-PB_LastPushedMs>PB_RepeatMs ) {
		PB_IncrBtn_DownEdge=HIGH;						//Repeat the button press
		PB_RepeatMs=PB_RepeatRatePeriodMs;				//Use RepeatRatePeriodMs for next auto-repeat period
		PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
	}
	if (PB_DecrBtn_DownEdge==HIGH){		//PB2: If we are just seeing the down edge, reset repeat counter and reset DOWN_Edge variable
		PB_RepeatMs=PB_RepeatRatePeriodMs;				//Use RepeatRatePeriodMs for next auto-repeat period
		PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
		PB_DecrBtn_DownEdge=LOW;
	}		
	if (PB_DecrBtn_value==LOW && millis()-PB_LastPushedMs>PB_RepeatMs ) {
		PB_DecrBtn_DownEdge=HIGH;						//Repeat the button press
		PB_RepeatMs=PB_RepeatRatePeriodMs;				//Use RepeatRatePeriodMs for next auto-repeat period
		PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
	}
	// END Push Button repeat logic ^^^^^^^^^^^^^^^^ 
	
	// Update the Bounce instances :
	#if defined (IncrPB_Pin)	//Update Increment Button state...
		debounce_IncrBtn.update();
		PB_IncrBtn_value = debounce_IncrBtn.read();	
		
		if (PB_IncrBtn_value==HIGH && PB_Last_IncrBtn_value==HIGH) {
			//No button push is in process...
			//PB_IncrBtn_DownEdge=LOW;	//Comment out to let application code reset these values
			//PB_IncrBtn_UpEdge=LOW;
		}		
		if (PB_IncrBtn_value==LOW && PB_Last_IncrBtn_value==HIGH) {
			//Detect a DOWN Edge...(Button just being depressed)
			PB_IncrBtn_DownEdge=HIGH;
			PB_IncrBtn_UpEdge=LOW;
			PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
			PB_RepeatMs=PB_RepeatInitialDelayMs;			//Use InitialRepeatDelayMs for next auto-repeat period
			//Serial.println ("==== PB1 DOWN Edge");
		}
		if (PB_IncrBtn_value==HIGH && PB_Last_IncrBtn_value==LOW) {
			//Detect an UP Edge...(Button being released)
			PB_IncrBtn_DownEdge=LOW;
			PB_IncrBtn_UpEdge=HIGH;
			//Serial.println ("==== PB1 UP Edge");
		}		
		PB_Last_IncrBtn_value=PB_IncrBtn_value;
	#endif
	
	#if defined (DecrPB_Pin)	
		debounce_DecrBtn.update();		//Update Decrement Button state...
		PB_DecrBtn_value = debounce_DecrBtn.read();	
		
		if (PB_DecrBtn_value==HIGH && PB_Last_DecrBtn_value==HIGH) {
			//No button push is in process...
			//PB_DecrBtn_DownEdge=LOW;	//Comment out to let application code reset these values
			//PB_DecrBtn_UpEdge=LOW;
		}		
		if (PB_DecrBtn_value==LOW && PB_Last_DecrBtn_value==HIGH) {
			//Detect a DOWN Edge...(Button just being depressed)
			PB_DecrBtn_DownEdge=HIGH;
			PB_DecrBtn_UpEdge=LOW;
			PB_LastPushedMs = millis();						//Update LastPushed TimeStamp
			PB_RepeatMs=PB_RepeatInitialDelayMs;			//Use InitialRepeatDelayMs for next auto-repeat period
			//Serial.println ("==== PB2 DOWN Edge");
		}
		if (PB_DecrBtn_value==HIGH && PB_Last_DecrBtn_value==LOW) {
			//Detect an UP Edge...(Button being released)
			PB_DecrBtn_DownEdge=LOW;
			PB_DecrBtn_UpEdge=HIGH;
			//Serial.println ("==== PB2 UP Edge");
		}		
		PB_Last_DecrBtn_value=PB_DecrBtn_value;

	#endif
	
	#if defined (NxtPB_Pin)	
		debounce_NxtBtn.update();
		PB_NxtBtn_value = debounce_NxtBtn.read();	
		
		if (PB_NxtBtn_value==HIGH && PB_Last_NxtBtn_value==HIGH) {
			//No button push is in process...
			//PB_NxtBtn_DownEdge=LOW;	//Comment out to let application code reset these values
			//PB_NxtBtn_UpEdge=LOW;
		}		
		if (PB_NxtBtn_value==LOW && PB_Last_NxtBtn_value==HIGH) {
			//Detect a DOWN Edge...(Button being depressed)
			PB_NxtBtn_DownEdge=HIGH;
			PB_NxtBtn_UpEdge=LOW;
			PB_RepeatMs=millis()+PB_RepeatInitialDelayMs;
			//Serial.println ("==== PB2 DOWN Edge");
		}
		if (PB_NxtBtn_value==HIGH && PB_Last_NxtBtn_value==LOW) {
			//Detect an UP Edge...(Button being released)
			PB_NxtBtn_DownEdge=LOW;
			PB_NxtBtn_UpEdge=HIGH;
			//Serial.println ("==== PB2 UP Edge");
		}		
		PB_Last_NxtBtn_value=PB_NxtBtn_value;

	#endif
	
	#if defined (EnterPB_Pin)	
		debounce_EnterBtn.update();
		PB_EnterBtn_value = debounce_EnterBtn.read();	
		
		if (PB_EnterBtn_value==HIGH && PB_Last_EnterBtn_value==HIGH) {
			//No button push is in process...
			//PB_EnterBtn_DownEdge=LOW;	//Comment out to let application code reset these values
			//PB_EnterBtn_UpEdge=LOW;
		}		
		if (PB_EnterBtn_value==LOW && PB_Last_EnterBtn_value==HIGH) {
			//Detect a DOWN Edge...(Button being depressed)
			PB_EnterBtn_DownEdge=HIGH;
			PB_EnterBtn_UpEdge=LOW;
			PB_RepeatMs=millis()+PB_RepeatInitialDelayMs;
			//Serial.println ("==== PB2 DOWN Edge");
		}
		if (PB_EnterBtn_value==HIGH && PB_Last_EnterBtn_value==LOW) {
			//Detect an UP Edge...(Button being released)
			PB_EnterBtn_DownEdge=LOW;
			PB_EnterBtn_UpEdge=HIGH;
			//Serial.println ("==== PB2 UP Edge");
		}		
		PB_Last_EnterBtn_value=PB_EnterBtn_value;

	#endif

}
void ResetPushButtons(){
	//	Routine to reset/initialize global push button variables.
	//
	//	Passed Parameters	NONE
	//
	//	Returns Via Global Variables:  RESET values for all buttons
	//
	//	20181005 Ver 0.0	E.Andrews	First cut - (Make button update a subroutine that can be called from anywhere)
	//	20181005 Ver 0.1	E.Andrews	Fixed reset logic state
	//
	
	//TODO vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv ---code not yet active!!!
		
		const bool ResetState=LOW;
		
		PB_IncrBtn_DownEdge=ResetState;
		PB_IncrBtn_UpEdge=ResetState;
		PB_Last_IncrBtn_value=PB_IncrBtn_value=LOW;
		
		PB_DecrBtn_DownEdge=ResetState;	
		PB_DecrBtn_UpEdge=ResetState;
		PB_Last_DecrBtn_value=PB_DecrBtn_value=LOW;
		
		PB_NxtBtn_DownEdge=ResetState;
		PB_NxtBtn_UpEdge=ResetState;
		PB_Last_NxtBtn_value=PB_NxtBtn_value=LOW;
		
		PB_EnterBtn_DownEdge=ResetState;
		PB_EnterBtn_UpEdge=ResetState;
		PB_Last_EnterBtn_value=PB_EnterBtn_value=LOW;
		
	//<TODO ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^--code not yet active!!!
}
void ChangeTrendInterval(int UpDwnChange){
	//Routine to change the Trend Plot interval in a 4-8-16-24-hour circular rotation.
	//
	//	PARAMETERS
	//	UpDwnChange		When >0, Trend time will INCREMENT
	//					When =0, Nochange in Trend Time
	//					When <0, Trend time will DECREMENT
	//						
	//
	//	RETURNS: 	NOTHING
	//
	//	20190124	E.Andrews	Rev 0.0	Move Trend-Change out of mainline into a subroutine
	//	20190301	E.Andrews	Rev 0.1	Added long term options (2-Days, 7_Days) if data is actually available
	//									Note: Max plot duration options will never exceed configuration value GBL_MaxDataHours setting
	//	20190302	E.Andrews	Rev 1.0	Changed behaviour to support INCREASE or DECREASE of Trend Time
	//	
		int DisplayHrs=0;	//This variable is used to provide Op Feedback to button press
		//Every-time the CHANGE-TREND-TIME button is pressed, the plot hours will change in a 
		// 4-8-16-1_day,2_days,7_Days sequence.  It wraps back to '4' when at GBL_MaxDataHours is EXCEEDED
		
		if(UpDwnChange>0){	//INCREASE BUTTON PUSHED
			switch (int(GBL_TotPlotHrs)){
				case 168 ... 200:			//Was 7 days, Stop Incrementing!
					GBL_TotPlotHrs = 7*25.5;
					DisplayHrs=168;
				break;
				case 0 ... 7:				//Was 4, switch to 8
					GBL_TotPlotHrs = 8.5;
					DisplayHrs=8;
				break;
				case 8 ... 16:				//Was 8, switch to 16
					GBL_TotPlotHrs = 17.0;
					DisplayHrs=16;
				break;
				case 17 ... 23:				//Was 16, switch to 24
					GBL_TotPlotHrs = 25.5;
					DisplayHrs=24;
				break;
				case 24 ... 27:				//Was 24, switch to 48
					GBL_TotPlotHrs = 51.0;
					DisplayHrs=48;
				break;		
				case 28 ... 52: 			//Was 48, switch to 7 days (168 Hours)
					GBL_TotPlotHrs = 7*25.5;
					DisplayHrs=168;
				break;			
				default:					//Out of Bounds... default to 4 Hrs
					GBL_TotPlotHrs = 4.25;
					DisplayHrs=4;
				break;
			}
		}
		if(UpDwnChange<0){	//DECREASE BUTTON PUSHED
			switch (int(GBL_TotPlotHrs)){
				case 168 ... 200:			//Was 7 days, switch to 48 hrs
					GBL_TotPlotHrs = 51.0;
					DisplayHrs=48;				
					//GBL_TotPlotHrs = 4.25;
					//DisplayHrs=4;
				break;
				case 0 ... 7:				//Was 4 Hrs, Stay at 4 Hrs)
					GBL_TotPlotHrs = 4.25;
					DisplayHrs=4;
				break;
				case 8 ... 16:				//Was 8 Hrs, switch to 4 Hrs
					GBL_TotPlotHrs = 4.25;
					DisplayHrs=4;
					//GBL_TotPlotHrs = 17.0;
					//DisplayHrs=16;
				break;
				case 17 ... 23:				//Was 16 Hrs, switch to 8 Hrs
					GBL_TotPlotHrs = 8.5;
					DisplayHrs=8;
					//GBL_TotPlotHrs = 25.5;
					//DisplayHrs=24;
				break;
				case 24 ... 27:				//Was 24 Hrs, switch to 16 Hrs
					GBL_TotPlotHrs = 17.0;
					DisplayHrs=16;				
					//GBL_TotPlotHrs = 51.0;
					//DisplayHrs=48;
				break;		
				case 28 ... 52: 			//Was 48 Hrs, switch to 24 Hrs
					GBL_TotPlotHrs = 25.5;
					DisplayHrs=24;
					//GBL_TotPlotHrs = 7*25.5;
					//DisplayHrs=168;
				break;			
				default:					//Out of Bounds... default to 4 Hrs
					GBL_TotPlotHrs = 4.25;
					DisplayHrs=4;
				break;
			}
		}
		
		//Range limit the upper most value...
		if (GBL_TotPlotHrs>GBL_MaxDataHours){
				GBL_TotPlotHrs = GBL_MaxDataHours;
				DisplayHrs=GBL_MaxDisplayHours;
		}

		#ifdef DiagPrintEnabled
			//DiagPrint(", New GBL_TotPlotHrs=",GBL_TotPlotHrs);
		#endif
		if(GBL_CurDisplayHours!=DisplayHrs){
			//Give Oper feedback to button press with GREEN square and CHANGE Message
			tft.fillRect(gXstart+55, gYstart+18,405,150,TFT_DARKGREEN);
			//myGLCD.setColor(255, 0, 0);
			//myGLCD.fillRoundRect (gXstart+35, gYstart+18, gXend-5, gYend-20);
			#ifdef DiagPrintEnabled
				DiagPrint(" -- Button PRESSED, waiting....",NULL);
			#endif 
			tft.setTextColor(YELLOW,GREEN);
			//tft.setFont(&FreeSansBold24pt7b);
			tft.setFont(&FreeSans12pt7b);
			tft.setTextSize(1);
			tft.setCursor(gXstart+40,gYstart+75);
			String NewTrendMsg = "    Trend-Time Changed To "+String(DisplayHrs) + " Hours";
			tft.print(NewTrendMsg);
		}
		else{	//No change made...Give NO CHANGE message
			//Give Oper feedback to button press with RED square and CHANGE Message
			tft.fillRect(gXstart+55, gYstart+18,405,150,RED);
			//myGLCD.setColor(255, 0, 0);
			//myGLCD.fillRoundRect (gXstart+35, gYstart+18, gXend-5, gYend-20);
			#ifdef DiagPrintEnabled
				DiagPrint(" -- Button PRESSED, waiting....",NULL);
			#endif 
			tft.setTextColor(YELLOW,RED);
			//tft.setFont(&FreeSansBold24pt7b);
			tft.setFont(&FreeSans12pt7b);
			tft.setTextSize(1);
			tft.setCursor(gXstart+40,gYstart+55);
			String NewTrendMsg = "     ---- AT LIMIT ---- NOT CHANGED ----";
			tft.print(NewTrendMsg);		
			tft.setCursor(gXstart+40,gYstart+95);
			NewTrendMsg= "     Trend Unchanged at "+String(DisplayHrs) + " Hours";
			tft.print(NewTrendMsg);	
		}
		delay (2000);	//Wait 2 seconds, then go on
		GBL_CurDisplayHours=DisplayHrs;	//This isn't really needed as the PlotTodTrendChart routine will update the global value...
		
		NewPlotNeeded=true;			//request a new graphics plot right away		
}
float DegC_to_DegF(float DegC){	//Convert Deg_C to Deg_F
	//	Converts DegC input to DegF output.
	//
	//	INPUTS: Float, Deg_C value
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	return (1.8*DegC) + 32.;
}
float DegF_to_DegC(float DegF){	//Convert Deg_F to Deg_C

	//	Converts DegF input to DegC output.
	//
	//	INPUTS: Float, Deg_F value
	//
	//	RETURNS: Float, Deg_C value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	return (DegF - 32.)*(5./9.);
}

//====================== DIAGNOSTIC SERIAL PRINT FORMAT ROUTINES==============================v
//
//	These routines were used during development to produce test prints to the Serial Port.
//	These routines are NOT used during normal run-time operation of the Graphing Thermometer program.
//	As such, these routines may be DELETED if desired.

void DiagPrintln(char const* Txt, int value){
	//	A single call to support the diagnostic printout of an integer numeric value with a string prefix (Txt) and string suffix (units).
	//	This routine will issue a CRLF upon completion of the print.
	//
	//	INTPUTS
	//	char const Txt		Prefix string
	//	int value			Numeric value to be printed
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	DiagPrintln(Txt, value, "");
}
void DiagPrintln(char const* Txt, int value, char const* units){
	//	A single call to support the diagnostic printout of an integer numeric value with a string prefix (Txt) and string suffix (units).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	int value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt); Serial.print(value); Serial.println(units);
}	
void DiagPrintln(char const* Txt, unsigned long value){
	//	A single call to support the diagnostic printout of an 'long' numeric value with a string prefix (Txt).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	long value			Numeric value to be printed
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	DiagPrintln(Txt, value, "");
}
void DiagPrintln(char const* Txt, unsigned long value, char const* units){
	//	A single call to support the diagnostic printout of an 'long' numeric value with a string prefix (Txt) and a string suffix (units).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	long value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt); Serial.print(value); Serial.println(units);
}
void DiagPrintln(char const* Txt, float value){
	//	A single call to support the diagnostic printout of an 'float' numeric value with a string prefix (Txt).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	float value			Numeric value to be printed

	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	DiagPrintln(Txt,value, "");	
}
void DiagPrintln(char const* Txt, float value, char const* units){
	//	A single call to support the diagnostic printout of an 'float' numeric value with a string prefix (Txt) and a string suffix (units).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	float value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt); Serial.print(value); Serial.println(units);
}	
void DiagPrintln(char const* Txt1, String& Txt2){
	//	A single call to support the diagnostic printout of two text strings.
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt1		Prefix text
	//	String Txt2			String text
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt1);Serial.println(Txt2);
}
void DiagPrintln(char const* Txt1, String& Txt2,char const* Txt3){
	//	A single call to support the diagnostic printout of a text string Txt2 with a string prefix (Txt1) and suffix (Txt3).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt1		Prefix text
	//	String Txt2			String text
	//	char const Txt3		Suffix text
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt1);Serial.print(Txt2);Serial.println(Txt3);	
}
void DiagPrint(char const* Txt, int value){
	//	A single call to support the diagnostic printout of an integer numeric value with a string prefix (Txt).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	int value			Numeric value to be printed
	//
	//	RETURNS: 	NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	DiagPrint(Txt, value,"");
}
void DiagPrint(char const* Txt, int value, char const* units){
	//	A single call to support the diagnostic printout of an integer numeric value with a string prefix (Txt) and string suffix (units).
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	int value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.print(Txt); Serial.print(value); Serial.print(units);
}
void DiagPrint(char const* Txt, unsigned long value){
	//	A single call to support the diagnostic printout of an 'long' numeric value with a string prefix (Txt).
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	long value			Numeric value to be printed
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	DiagPrint(Txt, value,"");
}
void DiagPrint(char const* Txt, unsigned long value, char const* units){
	//	A single call to support the diagnostic printout of an 'long' numeric value with a string prefix (Txt) and a string suffix (units).
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	long value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.print(Txt); Serial.print(value); Serial.print(units);
}
void DiagPrint(char const* Txt, float value){
	//	A single call to support the diagnostic printout of an 'float' numeric value with a string prefix (Txt).
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	float value			Numeric value to be printed

	//
	//	RETURNS: Float, Deg_F value
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	DiagPrint(Txt, value, "");
}
void DiagPrint(char const* Txt, float value, char const* units){
	//	A single call to support the diagnostic printout of an 'float' numeric value with a string prefix (Txt) and a string suffix (units).
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	float value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.print(Txt); Serial.print(value); Serial.print(units);
}
void DiagPrint(char const* Txt, double value){
	//	A single call to support the diagnostic printout of an 'double' numeric value with a string prefix (Txt).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	double value		Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	DiagPrint(Txt,value, "");
}
void DiagPrint(char const* Txt, double value, char const* units){
	//	A single call to support the diagnostic printout of an 'double' numeric value with a string prefix (Txt) and a string suffix (units).
	//	This routine will issue a CRLF upon completion of the print.
	//	
	//	INTPUTS
	//	char const Txt		Prefix string
	//	double value			Numeric value to be printed
	//	char const units	Suffix string
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.print(Txt); Serial.print(value); Serial.print(units);
}
void DiagPrint(char const* Txt1, String& Txt2){
	//	A single call to support the diagnostic printout of two text strings.
	//	
	//	INTPUTS
	//	char const Txt1		Prefix text
	//	String Txt2			String text
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//
	Serial.print(Txt1);Serial.print(Txt2);
}
void DiagPrint(char const* Txt1, String& Txt2,char const* Txt3){
	//	A single call to support the diagnostic printout of a text string Txt2 with a string prefix (Txt1) and suffix (Txt3).
	//	
	//	INTPUTS
	//	char const Txt1		Prefix text
	//	String Txt2			String text
	//	char const Txt3		Suffix text
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.print(Txt1);Serial.print(Txt2);Serial.print(Txt3);	
}
void DiagPrintln(){	// Just outputs a line feed...
	//	A single call to support the diagnostic printout of a CRLF.
	//	
	//	INTPUTS: NONE
	//
	//	RETURNS: NOTHING
	//
	//	20180925	E.Andrews	Rev 0.0	First Cut
	//	
	Serial.println();
}
//====================== END DIAGNOSTIC SERIAL PRINT FORMAT ROUTINES=========================^