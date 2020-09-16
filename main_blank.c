/*
 * CANopen main program file.
 *
 * This file is a template for other microcontrollers.
 * Modified by hustGs to make CanOpenNode work on F280049
 *
 * @file        main_generic.c
 * @author      Janez Paternoster ,  hustGs
 * @copyright   2004 - 2020 Janez Paternoster ,  hustGs
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>

#include "CANopen.h"

#include "driverlib.h"
#include "device.h"

#include "InterruptResponses.h"
//#include "canDebug.h"


#define TMR_TASK_INTERVAL   (1000)          /* Interval of tmrTask thread in microseconds */
#define INCREMENT_1MS(var)  (var++)         /* Increment 1ms variable in tmrTask */

#define log_printf(macropar_message, ...) \
        printf(macropar_message, ##__VA_ARGS__)


/* Global variables and objects */
    volatile static bool_t CANopenConfiguredOK = false; /* Indication if CANopen modules are configured */
    volatile uint16_t   CO_timer1ms = 0U;   /* variable increments each millisecond */
//    uint8_t LED_red, LED_green;
//    const OD_t ODxyz;


void initCPUTimers(void);
void configCPUTimer(uint32_t cpuTimer, float freq, float period);

/* main ***********************************************************************/
int main (void){
    CO_ReturnError_t err;
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    uint32_t heapMemoryUsed;
    void *CANmoduleAddress = NULL; /* CAN module address */
    uint8_t pendingNodeId = 10; /* read from dip switches or nonvolatile memory, configurable by LSS slave */
    uint8_t activeNodeId = 10; /* Copied from CO_pendingNodeId in the communication reset section */
    uint16_t pendingBitRate = 500000;  /*bps, read from dip switches or nonvolatile memory, configurable by LSS slave */
    /* Configure microcontroller. */
    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Initialize GPIO and configure GPIO pins for CANTX/CANRX
    //
    Device_initGPIO();
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANRXA);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_CANTXA);


    /* Allocate memory */
    err = CO_new(&heapMemoryUsed);
    if (err != CO_ERROR_NO) {
        log_printf("Error: Can't allocate memory\n");
        return 0;
    }
    else {
        log_printf("Allocated %d bytes for CANopen objects\n", heapMemoryUsed);
    }

    /* initialize EEPROM */


    /* increase variable each startup. Variable is stored in EEPROM. */
    OD_powerOnCounter++;

    log_printf("CANopenNode - Reset application, count = %d\n", OD_powerOnCounter);


    while(reset != CO_RESET_APP){
/* CANopen communication reset - initialize CANopen objects *******************/
        uint32_t timer1msPrevious;

        log_printf("CANopenNode - Reset communication...\n");

        /* disable CAN and CAN interrupts */
        CANopenConfiguredOK = false;

        /* initialize CANopen */
        err = CO_CANinit(CANmoduleAddress, pendingBitRate);
        if (err != CO_ERROR_NO) {
            log_printf("Error: CAN initialization failed: %d\n", err);
            return 0;
        }
        err = CO_LSSinit(&pendingNodeId, &pendingBitRate);
        if(err != CO_ERROR_NO) {
            log_printf("Error: LSS slave initialization failed: %d\n", err);
            return 0;
        }
        activeNodeId = pendingNodeId;
        err = CO_CANopenInit(activeNodeId);
        if(err == CO_ERROR_NO) {
            CANopenConfiguredOK = true;
        }
        else if(err != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
            log_printf("Error: CANopen initialization failed: %d\n", err);
            return 0;
        }

        /* Configure Timer interrupt function for execution every 1 millisecond */
        //
        // ISRs for each CPU Timer interrupt
        //
        Interrupt_register(INT_TIMER0, &cpuTimer0ISR);
        Interrupt_register(INT_TIMER1, &cpuTimer1ISR);

        //
        // Initializes the Device Peripheral. For this example, only initialize the
        // Cpu Timers.
        //
        initCPUTimers();

        //
        // Configure CPU-Timer 0, 1,  to interrupt every second:
        //  Period (in uSeconds)
        //
        configCPUTimer(CPUTIMER0_BASE, DEVICE_SYSCLK_FREQ, 1000);//1ms interrupt
        configCPUTimer(CPUTIMER1_BASE, DEVICE_SYSCLK_FREQ, 1000);


        //
        // To ensure precise timing, use write-only instructions to write to the
        // entire register. Therefore, if any of the configuration bits are changed
        // in configCPUTimer and initCPUTimers, the below settings must also
        // be updated.
        //
        CPUTimer_enableInterrupt(CPUTIMER0_BASE);
        CPUTimer_enableInterrupt(CPUTIMER1_BASE);

        //
        // Enables CPU int1, int13, and int14 which are connected to CPU-Timer 0,
        // CPU-Timer 1 respectively.
        // Enable TINT0 in the PIE: Group 1 interrupt 7
        //
        Interrupt_enable(INT_TIMER0);
        Interrupt_enable(INT_TIMER1);

        //
        // Starts CPU-Timer 0, CPU-Timer 1.
        //
        CPUTimer_startTimer(CPUTIMER0_BASE);
        CPUTimer_startTimer(CPUTIMER1_BASE);


        /* Configure CAN transmit and receive interrupt */

        //
        // Interrupts that are used in this example are re-mapped to
        // ISR functions found within this file.
        // This registers the interrupt handler in PIE vector table.
        //
        Interrupt_register(INT_CANA0, &canISR);

        //
        // Enable the CAN interrupt signal
        //
        Interrupt_enable(INT_CANA0);
        CAN_enableGlobalInterrupt(CANA_BASE, CAN_GLOBAL_INT_CANINT0);

        /* Configure CANopen callbacks, etc */
        if(CANopenConfiguredOK) {

        }


        /* start CAN */
        CO_CANsetNormalMode(CO->CANmodule[0]);

        reset = CO_RESET_NOT;
        timer1msPrevious = CO_timer1ms;
 //       timer1msPrevious= CpuTimer0Regs.TIM.all>> 5;//tim计数器右移5位切换到1ms

        log_printf("CANopenNode - Running...\n");
        fflush(stdout);

        while(reset == CO_RESET_NOT){
/* loop for normal program execution ******************************************/
            uint32_t timer1msCopy, timer1msDiff;

            timer1msCopy = CO_timer1ms;
            timer1msDiff = timer1msCopy - timer1msPrevious;
            timer1msPrevious = timer1msCopy;
            //该死循环必须在1s内执行一次，1s是发送报文的时间周期，如果1s没有执行此循环，下次发送报文的缓存将为空
            /* CANopen process */
            reset = CO_process(CO, (uint32_t)timer1msDiff*1000, NULL);
//            LED_red = CO_LED_RED(CO->LEDs, CO_LED_CANopen);
//            LED_green = CO_LED_GREEN(CO->LEDs, CO_LED_CANopen);

            /* Nonblocking application code may go here. */


            /* Process EEPROM */

            /* optional sleep for short time */
        }
    }


/* program exit ***************************************************************/
    /* stop threads */


    /* delete objects from memory */
    CO_delete((void*) 0/* CAN module address */);

    log_printf("CANopenNode finished\n");

    /* reset */
    return 0;
}


/* timer thread executes in constant intervals ********************************/
void tmrTask_thread(void){



        /* sleep for interval */

        INCREMENT_1MS(CO_timer1ms);

        if(CO->CANmodule[0]->CANnormal) {
            bool_t syncWas;

            /* Process Sync */
            syncWas = CO_process_SYNC(CO, TMR_TASK_INTERVAL, NULL);

            /* Read inputs */
            CO_process_RPDO(CO, syncWas);

            /* Further I/O or nonblocking application code may go here. */

            /* Write outputs */
            CO_process_TPDO(CO, syncWas, TMR_TASK_INTERVAL, NULL);

            /* verify timer overflow */
            if(0) {
                CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0U);
            }
        }

}


/* CAN interrupt function executes on received CAN message ********************/
void /* interrupt */ CO_CAN1InterruptHandler(void){

    /* clear interrupt flag */
}

void initCPUTimers(void)
{
    //
    // Initialize timer period to maximum
    //
    CPUTimer_setPeriod(CPUTIMER0_BASE, 0xFFFFFFFF);
    CPUTimer_setPeriod(CPUTIMER1_BASE, 0xFFFFFFFF);
//    CPUTimer_setPeriod(CPUTIMER2_BASE, 0xFFFFFFFF);

    //
    // Initialize pre-scale counter to divide by 1 (SYSCLKOUT)
    //
    CPUTimer_setPreScaler(CPUTIMER0_BASE, 0);
    CPUTimer_setPreScaler(CPUTIMER1_BASE, 0);
//    CPUTimer_setPreScaler(CPUTIMER2_BASE, 0);

    //
    // Make sure timer is stopped
    //
    CPUTimer_stopTimer(CPUTIMER0_BASE);
    CPUTimer_stopTimer(CPUTIMER1_BASE);
//    CPUTimer_stopTimer(CPUTIMER2_BASE);

    //
    // Reload all counter register with period value
    //
    CPUTimer_reloadTimerCounter(CPUTIMER0_BASE);
    CPUTimer_reloadTimerCounter(CPUTIMER1_BASE);
//    CPUTimer_reloadTimerCounter(CPUTIMER2_BASE);

    //
    // Reset interrupt counter
    //
    cpuTimer0IntCount = 0;
    cpuTimer1IntCount = 0;
//    cpuTimer2IntCount = 0;
}

//
// configCPUTimer - This function initializes the selected timer to the
// period specified by the "freq" and "period" parameters. The "freq" is
// entered as Hz and the period in uSeconds. The timer is held in the stopped
// state after configuration.
//
void
configCPUTimer(uint32_t cpuTimer, float freq, float period)
{
    uint32_t temp;

    //
    // Initialize timer period:
    //
    temp = (uint32_t)(freq / 1000000 * period);
    CPUTimer_setPeriod(cpuTimer, temp);

    //
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    //
    CPUTimer_setPreScaler(cpuTimer, 0);

    //
    // Initializes timer control register. The timer is stopped, reloaded,
    // free run disabled, and interrupt enabled.
    // Additionally, the free and soft bits are set
    //
    CPUTimer_stopTimer(cpuTimer);
    CPUTimer_reloadTimerCounter(cpuTimer);
    CPUTimer_setEmulationMode(cpuTimer,
                              CPUTIMER_EMULATIONMODE_RUNFREE);
    CPUTimer_enableInterrupt(cpuTimer);

    //
    // Resets interrupt counters for the three cpuTimers
    //
    if (cpuTimer == CPUTIMER0_BASE)
    {
        cpuTimer0IntCount = 0;
    }
    else if(cpuTimer == CPUTIMER1_BASE)
    {
        cpuTimer1IntCount = 0;
    }
}

