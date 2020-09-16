/*
 * Interrupt functions
 *
 *
 * @file        interruptResponses.c
 * @author      hustGs
 * @copyright   hustGs
 *
 */
#include "interruptResponses.h"
#include "driverlib.h"
#include <stdint.h>
//#include "301/CO_driver_target.h"
#include "canopen.h"


uint16_t cpuTimer0IntCount = 0;
uint16_t cpuTimer1IntCount = 0;


static CO_CANmodule_t *CANmodule = 0;
//CANmodule = CO->CANmodule[0];
extern void tmrTask_thread(void);
//
// CAN ISR - The interrupt service routine called when a CAN interrupt is
//           triggered.  It checks for the cause of the interrupt, and
//           maintains a count of all messages that have been transmitted.
//
__interrupt void
canISR(void)
{
    uint32_t status;
    if(CANmodule == 0)
    {
        CANmodule = CO->CANmodule[0];
    }
    //
    // Read the CAN interrupt status to find the cause of the interrupt
    //
    status = CAN_getInterruptCause(CANA_BASE);

    //
    // If the cause is a controller status interrupt, then get the status
    //
    if(status == CAN_INT_INT0ID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.
        //
        status = CAN_getStatus(CANA_BASE);

        //
        // Check to see if an error occurred.
        //
        if(((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 7) &&
           ((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 0))
        {
            //
            // Set a flag to indicate some errors may have occurred.
            //
//            errorFlag = 1;
        }
    }
    //
    // Check if the cause is the receive message object 2
    //
    else if(status == RX_MSG_OBJ_ID)
    {
        CO_CANrxMsg_t *rcvMsg;      /* pointer to received message in CAN module */
          uint16_t index;             /* index of received message */
          uint32_t rcvMsgIdent;       /* identifier of the received message */
          CO_CANrx_t *buffer = NULL;  /* receive message buffer from CO_CANmodule_t object. */
          bool_t msgMatched = false;

          /* get message from module here */
          uint16_t rxMsgData[6] = {0};
          CO_CANrxMsg_t tmp;

          //
          // Get the received message
          //
          CAN_readMessage(CANA_BASE, RX_MSG_OBJ_ID, rxMsgData);
          tmp.DLC = (uint8_t)rxMsgData[5];
          memcpy(tmp.data, rxMsgData, sizeof(uint16_t)*4);
          tmp.ident = (uint32_t)rxMsgData[4];
          rcvMsg = &tmp;

          rcvMsgIdent = rcvMsg->ident;
          if(CANmodule->useCANrxFilters){
              /* CAN module filters are used. Message with known 11-bit identifier has */
              /* been received */
              index = 0;  /* get index of the received message here. Or something similar */
              if(index < CANmodule->rxSize){
                  buffer = &CANmodule->rxArray[index];
                  /* verify also RTR */
                  if(((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U){
                      msgMatched = true;
                  }
              }
          }
          else{
              /* CAN module filters are not used, message with any standard 11-bit identifier */
              /* has been received. Search rxArray form CANmodule for the same CAN-ID. */
              buffer = &CANmodule->rxArray[0];
              for(index = CANmodule->rxSize; index > 0U; index--){
                  if(((rcvMsgIdent ^ buffer->ident) & buffer->mask) == 0U){
                      msgMatched = true;
                      break;
                  }
                  buffer++;
              }
          }

          /* Call specific function, which will process the message */
          if(msgMatched && (buffer != NULL) && (buffer->CANrx_callback != NULL)){
              buffer->CANrx_callback(buffer->object, (void*) rcvMsg);
          }

          /* Clear interrupt flag */
          //
          // Getting to this point means that the RX interrupt occurred on
          // message object 2, and the message RX is complete.  Clear the
          // message object interrupt.
          //
          CAN_clearInterruptStatus(CANA_BASE, RX_MSG_OBJ_ID);


    }

    //
    // If something unexpected caused the interrupt, this would handle it.
    //
    else
    {
        //
        // Spurious interrupt handling can go here.
        //
    }

    //
    // Clear the global interrupt flag for the CAN interrupt line
    //
    CAN_clearGlobalInterruptStatus(CANA_BASE, CAN_GLOBAL_INT_CANINT0);

    //
    // Acknowledge this interrupt located in group 9
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}
//
// cpuTimer0ISR - Counter for CpuTimer0
//
__interrupt void
cpuTimer0ISR(void)
{

    cpuTimer0IntCount++;
    if(CANmodule == 0)
    {
        CANmodule = CO->CANmodule[0];
    }
    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);

    //准备同步消息
    tmrTask_thread();


    /* First CAN message (bootup) was sent successfully */
    CANmodule->firstCANtxMessage = false;
     /* clear flag from previous message */
     CANmodule->bufferInhibitFlag = false;
     /* Are there any new messages waiting to be send */
     if(CANmodule->CANtxCount > 0U){
         uint16_t i;             /* index of transmitting message */

         /* first buffer */
         CO_CANtx_t *buffer = &CANmodule->txArray[0];
         /* search through whole array of pointers to transmit message buffers. */
         for(i = CANmodule->txSize; i > 0U; i--){
             /* if message buffer is full, send it. */
             if(buffer->bufferFull){
                 buffer->bufferFull = false;
                 CANmodule->CANtxCount--;

                 /* Copy message to CAN buffer */
                 CANmodule->bufferInhibitFlag = buffer->syncFlag;
                 /* canSend... */
                 //
                 // Initialize the transmit message object used for sending CAN messages.
                 // Message Object Parameters:
                 //      Message Object ID Number: 1
                 //      Message Identifier: 0x1
                 //      Message Frame: Standard
                 //      Message Type: Transmit
                 //      Message ID Mask: 0x0
                 //      Message Object Flags: Transmit Interrupt
                 //      Message Data Length: 4 Bytes
                 //
                 CAN_setupMessageObject(CANA_BASE, (uint32_t)i+1, buffer->ident, CAN_MSG_FRAME_STD,
                                        CAN_MSG_OBJ_TYPE_TX, 0, CAN_MSG_OBJ_NO_FLAGS,
                                        8);
                 CAN_sendMessage(CANA_BASE, (uint32_t)i+1, (uint16_t)buffer->DLC,
                                           (uint16_t*)buffer->data);
                 break;                      /* exit for loop */
             }
             buffer++;
         }/* end of for loop */

         /* Clear counter if no more messages */
         if(i == 0U){
             CANmodule->CANtxCount = 0U;
         }
     }
     if (cpuTimer0IntCount== 0xFFFF)
         cpuTimer0IntCount = 0;
}

//
// cpuTimer1ISR - Counter for CpuTimer1
//
uint16_t updateStatus = 0;
__interrupt void
cpuTimer1ISR(void)
{
    if(CANmodule == 0)
    {
        CANmodule = CO->CANmodule[0];
    }
    //
    // The CPU acknowledges the interrupt.
    //
    cpuTimer1IntCount++;

    if (cpuTimer1IntCount% 20 == 0 )
    {
        CO_SDOclientUploadInitiate(CO->SDOclient[0],
                                     0x2110, //
                                     5,
                                     800,0);
        updateStatus = 1 ;
    }

    char test[30]={0};

    uint16_t sizeIndicated = 0,sizeTransferred = 0 , res = 0;

    if(updateStatus == 1)
    {
        res = CO_SDOclientUpload(CO->SDOclient[0],
                           (uint32_t)1000*1000,
                           NULL,
                           &sizeIndicated,
                           &sizeTransferred,
                           NULL);
        if(res == CO_SDO_RT_ok_communicationEnd)
            {
                CO_SDOclientUploadBufRead(CO->SDOclient[0],
                                          test,
                                          30);
        //                CO_SDOclientDownloadInitiate();
                updateStatus = 0;
            }

    }



    if (cpuTimer1IntCount== 0xFFFF)
        cpuTimer1IntCount = 0;
}
