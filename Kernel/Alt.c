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
      // asm("MOV     W0,            _State");
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
            //asm("MOV     W0,            _State");
            asm("MOV     W0,             [W2]");

            // Execute return to Kernel
            //asm("MOV      _CurrentProcess, W2");
            //asm("PUSH     [W2++]");
            //asm("PUSH     [W2++]");
            //asm("LNK      #0x0");

            CurrentProcess = aProcess;

            asm("MOV    _CurrentProcess, W8");
            asm("ADD   #_StackAlc,       W8");
            asm("MOV    [W8++],          W0");
            asm("DISI  #0x0E");
            asm("MOV    W0,              SPLIM");
            asm("MOV.D  [W8++],          W14");
            asm("MOV.D  [--W15],         W0"); //Pop  the Status Register
            asm("MOV    W0,              SR");
            asm("MOV.D  [--W15],         W4"); //Pop  W4:W5
            asm("MOV.D  [--W15],         W2"); //Pop  W2:W3
            asm("MOV.D  [--W15],         W0"); //Pop  W0:W1
            IEC0bits.T3IE = 1; // Enable Timer1 interrupts
            T3CONbits.TON = 1; // Turn T3 on
            asm("ULNK");
            asm("RETURN");
         }
      }
   }
}


