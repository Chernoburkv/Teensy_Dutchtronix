/*Oscilloscope Clock 
  Mauro Pintus , Milano 2018/05/25
  GitHub Repository
  https://github.com/maurohh/ESP32_OscilloscopeClock
  Twitter Page
  https://twitter.com/PintusMauro
  Youtube Channel
  www.youtube.com/channel/UCZ93JYpVb9rEbg5cbcVG_WA/
  Old Web Site
  www.mauroh.com
  Credits:
  Andreas Spiess
  https://www.youtube.com/watch?v=DgaKlh081tU
  Andreas Spiess NTP Library
  https://github.com/SensorsIot/NTPtimeESP
  
  My project is based on this one:
  http://www.dutchtronix.com/ScopeClock.htm
  
  Thank you!!
******************************************************************************/

#include "DataTable.h"

#define BlankPin 2  // high blanks the display
#define RelPin 4  // Relay HV out
#define LedPin 13 // ledd



// Change this to set the initial Time
// Now is 10:08:37 (12h)
int h=10;   //Start Hour 
int m=8;    //Start Minutes
int s=37;   //Start Seconds

//Variables
int           lastx,lasty;
unsigned long currentMillis  = 0;
unsigned long previousMillis = 0;    
int           Timeout        = 20;
const    long interval       = 990; //milliseconds, you should twick this
                                    //to get a better accuracy




//*****************************************************************************
// Dot 
//*****************************************************************************

inline void Dot(int x, int y)
{
    if (lastx!=x){
      lastx=x;
      *(volatile uint16_t *)&(DAC0_DAT0L) = x;
    }
    
    if (lasty!=y){
      lasty=y;
      *(volatile uint16_t *)&(DAC1_DAT0L) = y;
    }
}


// End Dot 
//*****************************************************************************



//*****************************************************************************
// Line 
//*****************************************************************************
// Bresenham's Algorithm implementation optimized
// also known as a DDA - digital differential analyzer

void Line(byte x1, byte y1, byte x2, byte y2)
{
    int acc;
    // for speed, there are 8 DDA's, one for each octant
    if (y1 < y2) { // quadrant 1 or 2
        byte dy = y2 - y1;
        if (x1 < x2) { // quadrant 1
            byte dx = x2 - x1;
            if (dx > dy) { // < 45
                acc = (dx >> 1);
                for (; x1 <= x2; x1++) {
                    Dot(x1, y1);
                    acc -= dy;
                    if (acc < 0) {
                        y1++;
                        acc += dx;
                    }
                }
            }
            else {   // > 45
                acc = dy >> 1;
                for (; y1 <= y2; y1++) {
                    Dot(x1, y1);
                    acc -= dx;
                    if (acc < 0) {
                        x1++;
                        acc += dy;
                    }
                }
            }
        }
        else {  // quadrant 2
            byte dx = x1 - x2;
            if (dx > dy) { // < 45
                acc = dx >> 1;
                for (; x1 >= x2; x1--) {
                    Dot(x1, y1);
                    acc -= dy;
                    if (acc < 0) {
                        y1++;
                        acc += dx;
                    }
                }
            }
            else {  // > 45
                acc = dy >> 1;
                for (; y1 <= y2; y1++) {
                    Dot(x1, y1);
                    acc -= dx;
                    if (acc < 0) {
                        x1--;
                        acc += dy;
                    }
                }
            }        
        }
    }
    else { // quadrant 3 or 4
        byte dy = y1 - y2;
        if (x1 < x2) { // quadrant 4
            byte dx = x2 - x1;
            if (dx > dy) {  // < 45
                acc = dx >> 1;
                for (; x1 <= x2; x1++) {
                    Dot(x1, y1);
                    acc -= dy;
                    if (acc < 0) {
                        y1--;
                        acc += dx;
                    }
                }
            
            }
            else {  // > 45
                acc = dy >> 1;
                for (; y1 >= y2; y1--) { 
                    Dot(x1, y1);
                    acc -= dx;
                    if (acc < 0) {
                        x1++;
                        acc += dy;
                    }
                }

            }
        }
        else {  // quadrant 3
            byte dx = x1 - x2;
            if (dx > dy) { // < 45
                acc = dx >> 1;
                for (; x1 >= x2; x1--) {
                    Dot(x1, y1);
                    acc -= dy;
                    if (acc < 0) {
                        y1--;
                        acc += dx;
                    }
                }

            }
            else {  // > 45
                acc = dy >> 1;
                for (; y1 >= y2; y1--) {
                    Dot(x1, y1);
                    acc -= dx;
                    if (acc < 0) {
                        x1--;
                        acc += dy;
                    }
                }
            }
        }
    
    }
}

// End Line 
//*****************************************************************************

//*****************************************************************************
// PlotTable 
//*****************************************************************************

void PlotTable(byte *SubTable, int SubTableSize, int skip, int opt, int offset)
{
  int i=offset;
  while (i<SubTableSize){
    if (SubTable[i+2]==skip){
      i=i+3;
      if (opt==1) if (SubTable[i]==skip) i++;
    }
    Line(SubTable[i],SubTable[i+1],SubTable[i+2],SubTable[i+3]);  
    if (opt==2){
      Line(SubTable[i+2],SubTable[i+3],SubTable[i],SubTable[i+1]); 
    }
    i=i+2;
    if (SubTable[i+2]==0xFF) break;
  }
}

// End PlotTable 
//*****************************************************************************




//*****************************************************************************
// setup 
//*****************************************************************************

void setup() 
{
  
  
  SIM_SCGC2 |= (SIM_SCGC2_DAC0|SIM_SCGC2_DAC1);
  VREF_TRM = 0x60;
  VREF_SC = 0xE1;	
  delay(100);
  
  DAC0_C0 = (DAC_C0_DACEN);
  DAC1_C0 = (DAC_C0_DACEN);
  
  delay(100);
  pinMode(BlankPin, OUTPUT);   
  pinMode(LedPin, OUTPUT);
  pinMode(RelPin, OUTPUT);


  if (h > 12) h=h-12;
  h=(h*5)+m/12;
  
  delay(250);
  digitalWriteFast(LedPin, HIGH);
  delay(7000);
  digitalWriteFast(RelPin, HIGH);
  delay(250);
  digitalWriteFast(BlankPin, HIGH);


}

// End setup 
//*****************************************************************************



//*****************************************************************************
// loop 
//*****************************************************************************

void loop() {

  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    s++;
  }
  if (s==60) {
    s=0;
    m++;
    if ((m==12)||(m==24)||(m==36)||(m==48)) {
      h++;
    }
  }
  if (m==60) {
    m=0;
    h++;
  }
  if (h==60) {
    h=0;
  }

  //Optionals
  //PlotTable(DialDots,sizeof(DialDots),0x00,1,0);
  //PlotTable(TestData,sizeof(TestData),0x00,0,00); //Full
  //PlotTable(TestData,sizeof(TestData),0x00,0,11); //Without square

  int i;
  digitalWriteFast(BlankPin, HIGH);	
  PlotTable(DialData,sizeof(DialData),0x00,1,0);      //2 to back trace
  PlotTable(DialDigits12,sizeof(DialDigits12),0x00,1,0);//2 to back trace 
  PlotTable(HrPtrData, sizeof(HrPtrData), 0xFF,0,9*h);  // 9*h
  PlotTable(MinPtrData,sizeof(MinPtrData),0xFF,0,9*m);  // 9*m
  PlotTable(SecPtrData,sizeof(SecPtrData),0xFF,0,5*s);  // 5*s
  digitalWriteFast(BlankPin, LOW);
  delay_us(5600);	
}
