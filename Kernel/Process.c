//****************************************************************************/
/*                                                                          */
/*  Programmer: Gregory L Montgomery                                        */
/*                                                                          */
/*  Copyright © 2010-2019                                                   */
/*                                                                          */
/*  COPYING: (See the file COPYING.md for the GNU General Public License).  */
/*  this program is free software, and you may redistribute it and/or       */
/*  modify it under the terms of the GNU General Public License as          */
/*  published by the Free Software Foundation                               */
/*                                                                          */
/* This file is part of Gregory L Montgomery's code base collection Project.*/
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is free software:*/
/*     you can redistribute it and/or modify  it under the terms of the GNU */
/*     General Public License as published by the Free Software Foundation, */
/*     either version 3 of the License, or (at your option)                 */
/*     any later version.                                                   */
/*                                                                          */
/*     Gregory L Montgomery's code base collection Project is distributed   */
/*     in the hope that it will be useful, but WITHOUT ANY WARRANTY;        */
/*     without even the implied warranty of MERCHANTABILITY or FITNESS FOR  */
/*     A PARTICULAR PURPOSE.  See the GNU General Public License for more   */
/*     details.                                                             */
/*                                                                          */
/*     You should have received a copy of the GNU General Public License    */
/*     along with Gregory L Montgomery's code base collection Project.      */
/*     If not, see <https://www.gnu.org/licenses/>.                         */
/*                                                                          */
/****************************************************************************/
#include <xc.h>
#include <string.h>
#include "defs.h"
#include "System.h"
#include "Process.h"
#include "Device.h"
#include "SysCalls.h"

typedef struct rList
{
   t_Process    * aProcess;
   struct rList * Next;
}SEMIS;

// Private Methods
t_Process * SwitchTo(t_Process * Prev, t_Process * Next);
static void RunTask(void);
static t_Process * NextProcess(t_Process * );
static Boolean AllocProcess(int * Index);
static Boolean AddProcess(Service * );
static void SleepUntil(Service *, unsigned );

// Kernel Object
static Kernel This =
{
   .Run = RunTask,
   .Add = AddProcess,
   .System = SystemCall,
   .WaitOn = SleepUntil,
};

// File/Object scope private variables
static t_Process Processes[MAX_PROCESSES];
static t_Process  * BlockedList[MAX_PROCESSES]={0};
static t_Process  * CurrentProcess = 0;
static t_Process  * Head = 0;

static SEMIS        Symaphrs[MAX_PROCESSES];
// SEMIS      * SyncList = 0;
static SEMIS      * SyncHead  = 0;

static Register   * SystemRegs;
static byte Allocated;
static unsigned T3Config;
static unsigned Reload = 0; 
static ThreadsState State;
static volatile unsigned  StackPt;
static volatile unsigned  StackMx;
static volatile unsigned  FramPtr;
static volatile unsigned  StatusRegister;
static IndicatorBits * SystemStatus;
static unsigned BusSemephors;

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
Kernel * InitOS(void)
{
   byte PreScale = 1;
   unsigned long TimerTick = (0.000009 * Fosc); // (100uS * Frequency) 
   
   TimerTick >>=2; // Divide by (4) Clocks Per instruction  
   Allocated = 0;
   
   if(PreScale < 8)
   {
      Reload = (unsigned)TimerTick;
   }
   
   // This is to shut the compiler up about BusSemephors not being used!
   if(BusSemephors != 0) BusSemephors = 0;
   memset((void*)Symaphrs,0, sizeof(Symaphrs));
   // Setup Timer Tick, and process table
   Timer3Init(Reload, PreScale);
   T3Config = T3CON;
   memset((void*)Processes, 0, sizeof(Processes)); // Clear the process table
   SystemRegs = getRegisterSet(); // Save the reference to System Registers
   SystemStatus = getSystemStat();
   return &This;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: T                                                          */
/*                                                                          */
/****************************************************************************/
static void SleepUntil(Service * Srv, unsigned WhichOne)
{
   t_Process  * aProcess;
   Boolean NotFound = True;
   
   for(aProcess = Head; ((aProcess) && (NotFound)); (aProcess = aProcess->Next))
   {
      if(aProcess->Dev == Srv)
      {
         NotFound = False;
         aProcess->State = Blocked;
         BlockedList[WhichOne] = aProcess;
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: T                                                          */
/*                                                                          */
/****************************************************************************/
static Boolean AddProcess(Service * Srv)
{
   Boolean Results = False;
   t_Process * ProcessPtr = 0;
   int Index = 0;

      if (AllocProcess(&Index))
      {
         Processes[Index].Used = True;
         Processes[Index].Priority = 0;
         Processes[Index].Splim = (((unsigned)&Processes[Index]) + STACK_SIZE) - 2;
         Processes[Index].Address_1 = 0;
         Processes[Index].Address_2 = 0;
         Processes[Index].Quantum = Reload;
         Processes[Index].State = Ready;
         Processes[Index].FnType = Srv->FnType;
         Processes[Index].PID = Index;
         Processes[Index].Dev = Srv;
         Processes[Index].Next = 0;
         switch (Srv->FnType)
         {
         case UpDateFn:
            Processes[Index].Arg = Srv->Arg;
            Processes[Index].UpDateThread = Srv->UpDate;
            break;
         case DeviceFn:
            Processes[Index].Dev = Srv->Dev;
            Processes[Index].DeviceThread = Srv->Device;
            break;
         case ProcessFn:
            Processes[Index].Arg = 0;
            Processes[Index].ProcessThread = Srv->Thread;
            break;
         }
         
         // Build Linked List of processes
         if(Head)         
         {
            ProcessPtr = Head;
            while(ProcessPtr->Next) 
               ProcessPtr = ProcessPtr->Next;
            ProcessPtr->Next = &Processes[Index];
         }
         else
         {
            Head = &Processes[Index];
         }
         
         Results = True;
   }
   return Results; 
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview:                                                            */
/*                                                                          */
/****************************************************************************/
static Boolean AllocProcess(int * Index)
{
   Boolean Results = False;
   int i = 0;

   do
   {
      if (Processes[i].Used) // if used
         i++; // Go to the next element in the table
      else
      { // Found a free spot so lets exit.
         Results = True;
         *Index = i;
         Allocated++;
      }
   }while ((Results == False) && (i < MAX_PROCESSES));
   return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function: System Call                                                */
/*        Input: Index of which system call is being requested              */
/*       Return: None                                                       */
/*     Overview: This is an implementation of system calls. When this       */
/*               function is called it will set a flag which indicates      */
/*               which function call is being called and then trigger       */
/*               an interrupt which will call the actual function to carry  */
/*               out the function call.                                     */
/*                                                                          */
/****************************************************************************/
void SystemCall(int WhichOne)
{
   //asm("MOV  W0, _Scratch1");
   IEC2bits.C1IE = 1;
   IFS2bits.C1IF = 1;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: T                                                          */
/*                                                                          */
/****************************************************************************/
void WaitOn(unsigned WhichOne)
{
   BlockedList[WhichOne]->State = Blocked;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This is a round robin schedulare                           */
/*                                                                          */
/****************************************************************************/
Boolean FindNextFree(int * Index)
{
   Boolean NotFound=True;
   int i=-1;
   
   do
   {
      if(&Symaphrs[++i] == 0)
      {
         NotFound = False;
      }
   }while((i < MAX_PROCESSES) && NotFound);
   *Index=i;
   return (NotFound==False);
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This is a round robin schedulare                           */
/*                                                                          */
/****************************************************************************/
void Exec(Bus WhichBuss, void (*Fn)(void))
{
   switch (WhichBuss)
   {
      case _I2C_1:
         asm("BTSTS _BusSemephors, #0x00");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x00");
         break;
      case _I2C_2:
         asm("BTSTS _BusSemephors, #0x01");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x01");
         break;
      case _I2C_3:
         asm("BTSTS _BusSemephors, #0x02");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x02");
         break;
      case _SPI_1:
         asm("BTSTS _BusSemephors, #0x03");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x03");
         break;
      case _SPI_2:
         asm("BTSTS _BusSemephors, #0x04");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x04");
         break;
      case _SPI_3:
         asm("BTSTS _BusSemephors, #0x05");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x05");
         break;
      case _Uart_1:
         asm("BTSTS _BusSemephors, #0x06");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x06");
         break;
      case _Uart_2:
         asm("BTSTS _BusSemephors, #0x07");
         asm("BRA   NZ, END_EXEC");
         Fn();
         asm("BCLR _BusSemephors, #0x07");
         break;
      default:
         asm("BRA   END_CAPTURE");
   }
   asm("END_EXEC:");
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This is a round robin schedulare                           */
/*                                                                          */
/****************************************************************************/
void Capture(Bus WhichOne)
{
   int Free = 0;
   SEMIS * aProcess;
   
   if (FindNextFree(&Free))
   {
      //asm("DISI  #0x05");
      switch (WhichOne)
      {
         case _I2C_1:
            asm("BTSTS _BusSemephors, #0x00");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _I2C_2:
            asm("BTSTS _BusSemephors, #0x01");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _I2C_3:
            asm("BTSTS _BusSemephors, #0x02");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _SPI_1:
            asm("BTSTS _BusSemephors, #0x03");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _SPI_2:
            asm("BTSTS _BusSemephors, #0x04");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _SPI_3:
            asm("BTSTS _BusSemephors, #0x05");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _Uart_1:
            asm("BTSTS _BusSemephors, #0x06");
            asm("BRA   Z, END_CAPTURE");
            break;
         case _Uart_2:
            asm("BTSTS _BusSemephors, #0x07");
            asm("BRA   Z, END_CAPTURE");
            break;
         default:
            asm("BRA   END_CAPTURE");
      }
      
      // Put the current process to sleep
      CurrentProcess->State = Blocked;
      CurrentProcess->Resource = WhichOne;
      Symaphrs[Free].aProcess = CurrentProcess;

      // Build Linked List of processes waiting for bus resources
      if (SyncHead)
      {
         aProcess = SyncHead;
         while (aProcess->Next)
            aProcess = aProcess->Next;
         aProcess->Next = &Symaphrs[Free];
      }
      else
      {
         SyncHead = &Symaphrs[Free];
      }

      // Change Context
      asm("PUSH.D   W0"); // Save Registers
      asm("PUSH.D   W2");
      asm("PUSH.D   W4");
      //asm("PUSH.D   W6");
      asm("MOV      SR,            W0");
      asm("AND.B    #0x1F,         W0");
      asm("PUSH.D   W0");

      // Save the Stack Pointer, Splim, and update the processes state
      asm("MOV     _CurrentProcess, W2");
      asm("ADD    #_StackAlc,       W2");
      asm("MOV     SPLIM,           W0");
      asm("MOV     W0,             [W2++]");
      asm("MOV.D   W14,            [W2++]");

      //CurrentProcess->State = Running;
      asm("MOV     #0x01,           W0");
      asm("MOV     W0,            _State");
      asm("MOV     W0,             [W2]");

      // Execute return to Kernel
      asm("MOV      _CurrentProcess, W2");
      asm("PUSH     [W2++]");
      asm("PUSH     [W2++]");
      asm("LNK      #0x0");
   }
   asm("END_CAPTURE:");
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This is a round robin schedulare                           */
/*                                                                          */

/****************************************************************************/
void Release(void)
{
   SEMIS * sList = SyncHead;
   SEMIS * Prev;
   Bus BlockedOn = CurrentProcess->Resource;
   CurrentProcess->Resource = _NONE_;
   Boolean NotFound = False;
   t_Process * aProcess;
   // TODO: Finish figuring out how to complete/save the current task
   //       and context. Currently the program is designed to save the
   //       current task and 'switch to' the blocked task. It may be much
   //       cleaner to set things up so that the current task finishes, and
   //       the blocked task is queued such that the actual 
   //       Switch-to-function actually switches in the task that was blocked.
   
   if (SyncHead)
   {

      while ((sList) && (NotFound))
      {
         Prev = sList;
         sList = sList->Next;
         aProcess = sList->aProcess;
         if (aProcess->Resource == BlockedOn)
         { // Unlink process from blocked list
            Prev->Next = sList->Next;
            aProcess->State = Running;
            NotFound = False;

            //asm("MOV.D  [--15], W0")

            // Save Current Context
            asm("PUSH.D   W0"); // Save Registers
            asm("PUSH.D   W2");
            asm("PUSH.D   W4");
            //asm("PUSH.D   W6");
            asm("MOV      SR,            W0");
            asm("AND.B    #0x1F,         W0");
            asm("PUSH.D   W0");

            // Save the Stack Pointer, Splim, and update the processes state
            asm("MOV     _CurrentProcess, W2");
            asm("ADD    #_StackAlc,       W2");
            asm("MOV     SPLIM,           W0");
            asm("MOV     W0,             [W2++]");
            asm("MOV.D   W14,            [W2++]");

            //CurrentProcess->State = Running;
            asm("MOV     #0x01,           W0");
            asm("MOV     W0,            _State");
            asm("MOV     W0,             [W2]");

            // Execute return to Kernel
            //asm("MOV      _CurrentProcess, W2");
            //asm("PUSH     [W2++]");
            //asm("PUSH     [W2++]");
            //asm("LNK      #0x0");

            CurrentProcess = aProcess;
            //memset(&BlockedList[WhichOne], 0, sizeof (SEMIS));

            asm("MOV      _CurrentProcess, W8");
            asm("ADD     #_StackAlc,       W8");
            asm("MOV     [W8++],           W0");
            asm("DISI    #0x0E");
            asm("MOV     W0,            SPLIM");
            asm("MOV.D   [W8++],           W14");
            asm("MOV.D   [--W15],          W0"); //Pop  the Status Register
            asm("MOV     W0,               SR");
            //asm("MOV.D  [--W15],         W6"); //Pop  W6:W7
            asm("MOV.D  [--W15],           W4"); //Pop  W4:W5
            asm("MOV.D  [--W15],           W2"); //Pop  W2:W3
            asm("MOV.D  [--W15],           W0"); //Pop  W0:W1
            IEC0bits.T3IE = 1; // Enable Timer1 interrupts
            T3CONbits.TON = 1; // Turn T3 on
            asm("ULNK");
            asm("RETURN");
         }
      }
   }
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: This is a round robin schedulare                           */
/*                                                                          */
/****************************************************************************/
static t_Process * NextProcess(t_Process * ProcessPtr)
{
   t_Process * Results = 0;
   
   asm("SCHEDULAR:    MOV  _SystemStatus, W0");
   asm("              MOV  [W0], W0");

   // Bit test and skip set Uart Ready Bit
   asm("HANDLE_UART1: BTSS  W0, #0");
   asm("              BRA   HANDLE_NVRAM");
   Results = BlockedList[UART1Rdy];
   Results->State = Ready;
   SystemStatus->Uart1Rdy = False;
   asm("              BRA   EXIT_SCHEDULAR");
   
   // Bit test and skip set NVRam Save
   asm("HANDLE_NVRAM: BTSS  W0, #5");
   asm("              BRA   SCHEDULER");
   Results = BlockedList[CONFIGDrty];
   Results->State = Ready;
   SystemStatus->CfgDirty = False;
   asm("              BRA  EXIT_SCHEDULAR");
         
   asm("SCHEDULER:    NOP");
   do
   {
      if(ProcessPtr->Next != 0)
         ProcessPtr = ProcessPtr->Next;
      else
         ProcessPtr = Head;
            
      if((ProcessPtr->State == Ready) || (ProcessPtr->State == Running))
      {
         Results = ProcessPtr;
      }
   }while (Results == 0);

   asm("EXIT_SCHEDULAR:");
   return Results;
}

/****************************************************************************/
/*                                                                          */
/*     Function:                                                            */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: T                                                          */
/*                                                                          */
/****************************************************************************/
void RunTask(void)
{
   // Save the current Stack and Status
   asm("MOV     W15,        _StackPt \n"
       "MOV     W14,        _FramPtr \n"
       "MOV     SPLIM,      W0 \n"
       "MOV     W0,         _StackMx \n"
       "MOV     SR,         W0 \n"
       "MOV     W0,         _StatusRegister \n");
   // Setup Process List
   asm("MOV     _Head,      W0");
   asm("MOV     W0,         _CurrentProcess");

   do
   { // Get the next process to run
      CurrentProcess = NextProcess(CurrentProcess);

      // Set the process's state to Running
      IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
      IEC0bits.T3IE = 1; // Enable Timer 3 interrupt
      State = Ready;

      switch (CurrentProcess->State)
      {
            /******************************************************************/
         case Ready: // If it is a new process then prime the pump
            // Give the process a fresh stack
            asm("DISI  #0x05");
            asm("MOV   _CurrentProcess, W14"); // The stack is the first the
            asm("MOV   _CurrentProcess, W15"); // beginning of CurrentProcess
            asm("MOV   #_StackLim,      W0");
            asm("ADD   W14,             W0, W0");
            asm("MOV   W0,              SPLIM");

            switch (CurrentProcess->FnType) // Run Process
            {
               case UpDateFn:
                  T3CONbits.TON = 1; // Turn on Timer 3
                  CurrentProcess->UpDateThread(CurrentProcess->Arg); // Execute Thread
                  break;
               case DeviceFn:
                  T3CONbits.TON = 1; // Turn on Timer 3
                  CurrentProcess->DeviceThread(CurrentProcess->Dev); // Execute Thread
                  break;
               case ProcessFn:
                  T3CONbits.TON = 1; // Turn on Timer 3
                  CurrentProcess->ProcessThread(); // Execute Thread
                  break;
            }

            // Stop timer tick
            T3CONbits.TON = 0;
            IEC0bits.T3IE = 0;
            TMR3 = 0x00; // Clear contents of the timer register

            if (State == Ready)
               CurrentProcess->State = Ready;

            // Restore the System's Stack(StackPt, StackMx);
            asm("MOV   _StackMx,         W0");
            asm("DISI  #0x05"); // Disable interrupts for 5 instruction cycles
            asm("MOV    W0,              SPLIM");
            asm("MOV   _FramPtr,         W14");
            asm("MOV   _StackPt,         W15");
            asm("MOV   _StatusRegister,  W0"); //; Fetch previous Status Register Value"
            asm("MOV    W0,              SR");
            break;
            /******************************************************************/
         case Running: // If the process is running complete it if possible
            //asm("NOP");
            asm("MOV      _CurrentProcess, W8");
            asm("ADD     #_StackAlc,       W8");
            asm("MOV     [W8++],           W0");
            asm("DISI    #0x0E");
            asm("MOV     W0,            SPLIM");
            asm("MOV.D   [W8++],           W14");
            asm("MOV.D   [--W15],          W0"); //Pop  the Status Register
            asm("MOV     W0,               SR");
            //asm("MOV.D  [--W15],         W6"); //Pop  W6:W7
            asm("MOV.D  [--W15],           W4"); //Pop  W4:W5
            asm("MOV.D  [--W15],           W2"); //Pop  W2:W3
            asm("MOV.D  [--W15],           W0"); //Pop  W0:W1
            IEC0bits.T3IE = 1; // Enable Timer1 interrupts
            T3CONbits.TON = 1; // Turn T3 on
            asm("ULNK");
            asm("RETURN");
            break;
         case Blocked:
            break;
         case Terminated:
            break;
      }
   }
   while (LOOPING_FOREVER); // Repeat Forever
}

/****************************************************************************/
/*                                                                          */
/*     Function: _T3Interrupt                                               */
/*        Input: None                                                       */
/*       Return: None                                                       */
/*     Overview: The Timer3Init                                             */
/*                                                                          */
/****************************************************************************/
void __attribute__((interrupt, shadow, no_auto_psv)) _T3Interrupt(void)
{ // Make sure the counter is off
   // Stop Timer 3
   T3CONbits.TON = 0;
   IEC0bits.T3IE = 0;

   // Save current Context
   asm("PUSH.D   W0"); // Save Registers
   asm("PUSH.D   W2");
   asm("PUSH.D   W4");
   //asm("PUSH.D   W6");
   asm("MOV      SR,            W0");
   asm("AND.B    #0x1F,         W0");
   asm("PUSH.D   W0");

   // Save the Stack Pointer, Splim, and update the processes state
   asm("MOV     _CurrentProcess, W2");
   asm("ADD     #_StackAlc,      W2");
   asm("MOV     SPLIM,           W0");
   asm("MOV     W0,             [W2++]");
   asm("MOV.D   W14,            [W2++]");

   //CurrentProcess->State = Running;
   asm("MOV     #0x01,           W0");
   asm("MOV     W0,            _State");
   asm("MOV     W0,             [W2]");

   // Execute return to Kernel
   asm("MOV      _CurrentProcess, W2");
   asm("PUSH     [W2++]");
   asm("PUSH     [W2++]");
   asm("LNK      #0x0");

   IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
}
