#include <xc.h>
#include <string.h>
#include <stdint.h>
#include <float.h>

#include "Registers.h"

#include "defs.h"
#include "NTSD1XH103FPB30.h"


#define TERM_DESCRIPTION "NTSD1XH103FPB30"
#define NO_OF_TERMISTERS 5
#define R_1 5080

extern Register Registers[MEMORY_SIZE];

ThermTable NTS_XH103[] =
{
   { -40, 195652, -2621440}, //   0
   { -35, 148171, -2293760}, //   1
   { -30, 113347, -1966080}, //   2
   { -25,  87559, -1638400}, //   3
   { -20,  68237, -1310720}, //   4
   { -15,  53650,  -983040}, //   5
   { -10,  42506,  -655360}, //   6
   { -5,   33892,  -327680}, //   7
   {  0,   27219,        0}, //   8
   {  5,   22021,   327680}, //   9
   { 10,   17926,   655360}, //  10
   { 15,   14674,   983040}, //  11
   { 20,   12081,  1310720}, //  12
   { 25,   10000,  1638400}, //  13
   { 30,    8315,  1966080}, //  14
   { 35,    6948,  2293760}, //  15
   { 40,    5834,  2621440}, //  16
   { 45,    4917,  2949120}, //  17
   { 50,    4161,  3276800}, //  18
   { 55,    3535,  3604480}, //  19
   { 60,    3014,  3932160}, //  20
   { 65,    2586,  4259840}, //  21
   { 70,    2228,  4587520}, //  22
   { 75,    1925,  4915200}, //  23
   { 80,    1669,  5242880}, //  24
   { 85,    1452,  5570560}, //  25
   { 90,    1268,  5898240}, //  26
   { 95,    1110,  6225920}, //  27
   {100,     974,  6553600}, //  28
   {105,     858,  6881280}, //  29
   {110,     758,  7208960}, //  30
   {115,     671,  7536640}, //  31
   {120,     596,  7864320}  //  32
};

const uint32_t MaxCounts=0x010000000; // 4096 * 65536
const uint32_t R1=((uint32_t)R_1 << 16); // R1 * 65536)

void UpDateTemp(void *);

static struct _Thermister Therms[NO_OF_TERMISTERS];
static int FindIndex(uint32_t);

static Boolean _therm_Alloc(int *);
static void Clear(Thermister *);
static uint32_t Slope;
static uint32_t Resistence;
static Frational Scratch;

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Device * NewTherm(Register * Counts, Register * Temp)
{
   int Index=0;
   Thermister * aTherm=0;
   Device * This=0;
   Device aDev=
   {
     .DeviceType=THERMISTER,
     .Discription=TERM_DESCRIPTION,
     .UpDate=UpDateTemp,
     .Read=0,
     .Write=0,
     .IsReady=0,
     .IOCTL=0
   };

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
   if(_therm_Alloc(&Index))
   {
      aTherm=&Therms[Index];
      Clear(aTherm);
      aTherm->Counts = Counts;
      aTherm->Temperature = Temp;
      aTherm->Used=True;
      aTherm->Id=Index+1;
   }
   aDev.Driver = aTherm;
   This = NewDeviceCpy(&aDev);
   return This;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
void UpDateTemp(void * Obj)
{
   Thermister * Thm = (Thermister *) Obj;   
   uint32_t Cnts = Thm->Counts->Value;
   int Index;
   
   if (Cnts > 0)
   {
      Resistence = MaxCounts / Cnts;
      Resistence -= 0x00010000;
      Resistence *= R_1;
      Resistence >>= 16;
      
      Registers[FAN_COUNTS1].Value=Resistence;
      Registers[FAN_COUNTS1].Changed=True;
      
      Index = FindIndex(Resistence);
      
      Registers[FAN_COUNTS2].Value=Index;
      Registers[FAN_COUNTS2].Changed=True;
      
      if(Index < 34)
      {  
         // Slope = 5.0/(NTS_XH103[Index - 1].Resistence - NTS_XH103[Index].Resistence);
         Slope = 327680/(NTS_XH103[Index - 1].Resistence - NTS_XH103[Index].Resistence);
         //Scrtch1 = (Slope *(Resistence - NTS_XH103[Index].Resistence)) + NTS_XH103[Index].Temperature;
         Scratch.Result = (Slope *(NTS_XH103[Index - 1].Resistence - Resistence)) + NTS_XH103[Index - 1].AjdTemp;
         //Scratch.Result=(uint32_t)Scrtch1;

         Registers[FAN_COUNTS3].Value=Scratch.Numerator;
         Registers[FAN_COUNTS3].Changed=True;
         Registers[FAN_COUNTS4].Value=Scratch.Denominator;
         Registers[FAN_COUNTS4].Changed=True;
         
         Thm->Temperature->Value=Scratch.Numerator;
         Thm->Temperature->Changed=True;
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static int FindIndex(uint32_t Value)
{
   int Index=0;
   while((NTS_XH103[Index++].Resistence >= Value) && (Index < 34));
   return Index - 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static Boolean _therm_Alloc(int * Index)
{
  Boolean Results = False;
  int i = 0;

  do
  {
    if(Therms[i].Used) // if used
      i++; // Go to the next element in the table
    else
    { // Found a free spot so lets exit.
      Results = True;
      *Index = i;
    }
  }while((Results == False) && (i < NO_OF_TERMISTERS));
  return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input:                                                            */
/*       Output:                                                            */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static void Clear(Thermister * Therms)
{
  memset(Therms, 0, sizeof(Thermister));
}
