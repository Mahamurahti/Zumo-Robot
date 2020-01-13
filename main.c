/**
* @mainpage ZumoBot Project
* @brief    You can make your own ZumoBot with various sensors.
* @details  <br><br>
    <p>
    <B>General</B><br>
    You will use Pololu Zumo Shields for your robot project with CY8CKIT-059(PSoC 5LP) from Cypress semiconductor.This 
    library has basic methods of various sensors and communications so that you can make what you want with them. <br> 
    <br><br>
    </p>
    
    <p>
    <B>Sensors</B><br>
    &nbsp;Included: <br>
        &nbsp;&nbsp;&nbsp;&nbsp;LSM303D: Accelerometer & Magnetometer<br>
        &nbsp;&nbsp;&nbsp;&nbsp;L3GD20H: Gyroscope<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Reflectance sensor<br>
        &nbsp;&nbsp;&nbsp;&nbsp;Motors
    &nbsp;Wii nunchuck<br>
    &nbsp;TSOP-2236: IR Receiver<br>
    &nbsp;HC-SR04: Ultrasonic sensor<br>
    &nbsp;APDS-9301: Ambient light sensor<br>
    &nbsp;IR LED <br><br><br>
    </p>
    
    <p>
    <B>Communication</B><br>
    I2C, UART, Serial<br>
    </p>
*/


/*
    TOP 3 CODES ARE FOR COMPETITION, IN MOTOR.C WE HAVE 2 OF OUR OWN MECHANISMS FOR ROBOT TURNING
    OUR GROUP: ERIC, JONNE AND MAKSIM
*/
#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
/**
 * @file    main.c
 * @brief   
 * @details  ** Enable global interrupt since Zumo library uses interrupts. **<br>&nbsp;&nbsp;&nbsp;CyGlobalIntEnable;<br>
*/
#if 1
// LINE FOLLOWING FOR COMPETITION
void zmain(void)
{
    TickType_t initialize = xTaskGetTickCount(), start, end, stop, timeStamp; // Setting variables to count time
    int passedTimes=0;  // Passed times calculates how many times the robot has passed a line.
    uint32_t IR_val;    
    bool dontCount;     // This boolean is used to count a line only once.
    
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    motor_start();
    motor_forward(0,0);
    
    IR_Start();
    
    send_mqtt("Zumo032/debug", "Boot");
 
    while(SW1_Read()==1);  // Robot waits for button press to travel to a black line.
    
    motor_forward(40,0);
    
    while(!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
    {
        reflectance_digital(&dig);   // Robot moves forward until a line is detected.
    }
    
    motor_forward(0,0);
    print_mqtt("Zumo032/ready", "line");
    IR_wait(); // Robot waits for an IR signal to start the line following.

    start = xTaskGetTickCount();
    print_mqtt("Zumo032/start", "%d", start);
    motor_forward(100,0);
    
    while(passedTimes<2) // Line following will loop until 2 lines are passed.
    {
        reflectance_digital(&dig); 
        // MOVE FORWARD
        if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_forward(140,1);
        }
        // MOVE MILDLY RIGHT
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0)
        {
            motor_turn(140,80,1);
        }
        // MOVR MIDLY LEFT
        else if (dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(80,140,1);
        }
        // MOVE HARD RIGHT
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_turn(240,5,1);
        }
        // MOVE HARD LEFT
        else if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(5,240,1);
        }
        // MOVE VERY HARD RIGHT
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 1)
        {
            motor_turn(255,0,1);
        }
        // MOVE VERY HARD LEFT
        else if (dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(0,255,1);
        }
        // IF A VERTICAL LINE IS DETECTED AND COUNTER ENABLER IS FALSE, INCREMET PASSEDTIMES
        // AND SET COUNTER ENABLER TO TRUE
        if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1 && dontCount == false)
        {
            passedTimes++; 
            dontCount = true;
        }
        // IF NOT ON A VERTICAL LINE SET THE COUNTER ENABLER TO FALSE
        else if (!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
        {
            dontCount = false;
        }
    } 
    
    motor_forward(0,0);

    stop = xTaskGetTickCount();
    print_mqtt("Zumo032/stop", "%d", stop);
    
    end = stop - start;
    print_mqtt("Zumo032/time", "%d", end);
    
    while(1)
    {
        vTaskDelay(100);
    }
}   

#endif

#if 0
// MAZE SOLVING FOR COMPETITION
void zmain(void)
{
    TickType_t initialize = xTaskGetTickCount(), start, end, stop, timeStamp = 0; // Setting variables to count time
    uint32_t IR_val;
    int x = 200, y = 200; 
    // x is the turn speed and y is the time for how long will the turn last when relfectance is 
    // either all black, all white or when the robot sees an obstacle
    
    bool inMaze=false; // This boolean tells the code if the robot is in the Maze
    
    struct sensors_ ref;
    struct sensors_ dig;

    Ultra_Start();
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    motor_start();
    motor_forward(0,0);
    
    IR_Start();

    send_mqtt("Zumo032/debug", "Boot");
 
    while(SW1_Read()==1);  // Robot waits for button press to travel to a black line.
    
    motor_forward(40,0);
    
    while(!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
    {
        reflectance_digital(&dig);   // Robot moves forward until a line is detected.
    }
    
    motor_forward(0,0);
    print_mqtt("Zumo032/ready", "maze");
    IR_wait();  // Robot waits for IR signal to start solving the maze.

    start = xTaskGetTickCount();
    print_mqtt("Zumo032/start", "%d", start);
    
    motor_forward(100,200);
    inMaze = true;
    
    // This maze solving will follow the rules that in every intersection
    // the robot will turn left, if it can't turn left it will go straight
    // and if it can't turn left or go straight it will turn right.
    
    // Motor_reverse_turn_left/right can be found in Motor.c
    
    while(inMaze)
    {
        reflectance_digital(&dig);
        int d = Ultra_GetDistance();
        // IF ROBOT SEES OBSTACLE, IT TURNS RIGHT
        if (d <= 9)
        {
            motor_reverse_turn_right(x,x,y);
        }
        // IF ROBOT SEES ALL WHITE, IT TURNS RIGHT
        if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_forward(80,100);
            motor_reverse_turn_right(x,x,y);
        }
        // IF ROBOT SEES ALL BLACK, IT TURNS LEFT
        else if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_forward(80,200);
            motor_reverse_turn_left(x,x,y);
        }
        // MOVE FORWARD
        if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_forward(80,1);
        }
        // MOVE MILDLY RIGHT
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0)
        {
            motor_turn(80,50,1);
        }
        // MOVR MIDLY LEFT
        else if (dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(50,80,1);
        }
        
        // MOVE HARD RIGHT
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_turn(100,30,1);
        }
        // MOVE HARD LEFT
        else if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(30,100,1);
        }
        // DETECT IF THERE IS A BLACK LINE ON EITHER SIDE OF ROBOT
        if(dig.l3 == 0 && dig.r3 == 0)
        {
            timeStamp = xTaskGetTickCount(); // Count for how long a blackline was last seen
            if(timeStamp >= 5300) // If a black line has not been found in 5.3s, stop robot
            {
                inMaze = false;
            }
        }
        else // If a blackline is spotted, reset the timer
        {
            timeStamp = 0;
        }
    }
    motor_forward(0,0);

    stop = xTaskGetTickCount();
    print_mqtt("Zumo032/stop", "%d", stop);
    
    end = stop - start;
    print_mqtt("Zumo032/time", "%d", end);
    
    while(1)
    {
        vTaskDelay(100);
    }
}

#endif

#if 0
// ZUMO WRESTLING FOR COMPETITION
void zmain(void)
{
    TickType_t initialize = xTaskGetTickCount(), start, end, stop, timeStamp = 0; // Setting variables to count time
    
    struct accData_ data;
    struct sensors_ ref;
    struct sensors_ dig;
    Ultra_Start();
    motor_start();
    IR_Start();
    bool push, inZumo=false; // Push bool is used to indicate when to push and inZumo tells the robot if its in the ring

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    reflectance_digital(&dig);
    int d = Ultra_GetDistance();
    
    send_mqtt("Zumo032/debug", "Boot");
    
    printf("Accelerometer test...\n");
    if(!LSM303D_Start())
    {
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        while(1) vTaskDelay(10);
    }
    else
    {
        printf("Device Ok...\n");
    }
    
    while(SW1_Read()==1); // After button press, travels to line
    
    motor_forward(40,0);
    
    while(!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
    {
        reflectance_digital(&dig); // Checks if there is a line
    }
    print_mqtt("Zumo032/ready", "zumo");

    motor_forward(0,0);
    
    IR_wait(); //Waits for ir signal
    
    inZumo=true;
    start = xTaskGetTickCount();
    print_mqtt("Zumo032/start","%d", start);
    
    motor_forward(100,200);
    
    while (inZumo)
    {   
        LSM303D_Read_Acc(&data);
        if(data.accX < -10000 || data.accY < -3000) //Reads X and Y axel from Accelometer and prints hits
        {
            timeStamp = xTaskGetTickCount();
            print_mqtt("Zumo032/hit", "%d", timeStamp);
        }
        reflectance_digital(&dig);
        d = Ultra_GetDistance();
        motor_forward(200,0);
        vTaskDelay(5);
        //When hits line from left side, robot goes backwards and turns right
        if ((dig.l3 == 1 || dig.l2 == 1 || dig.l1 == 1) && (dig.r3 == 0 && dig.r2 == 0 && dig.r1 == 0))
        {
            motor_backward(255,150);
            motor_turn(0,255,300);
        }
        //When hits line from right side, robot goes backwards and turns left
        if ((dig.r1 == 1 || dig.r2 == 1 || dig.r3 == 1) && (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0))
        {
            motor_backward(255,150);
            motor_turn(255,0,300);
        }
        //When everything is black, backup and turn
        if ((dig.r1 == 1 || dig.r2 == 1 || dig.r3 == 1) && (dig.l3 == 1 || dig.l2 == 1 || dig.l1 == 1))
        {
            motor_backward(255,150);
            motor_turn(255,0,300);
            
        }
        //Push mechanism
        if (d <= 20 && !(dig.l3 == 1 || dig.l2 == 1 || dig.l1 == 1 || dig.r1 == 1 || dig.r2 == 1 || dig.r3 == 1))
        {
            motor_forward(255,0);
            while (!(dig.l3 == 1 || dig.l2 == 1 || dig.l1 == 1 || dig.r1 == 1 || dig.r2 == 1 || dig.r3 == 1))
            {
               reflectance_digital(&dig); // Robot checks the line even when pushing not to go off the ring
            }
            // If a line is found during a push, go backwards and turn
            motor_backward(255,200);
            motor_turn(255,0,100);
        }
        
        if(SW1_Read() == 0)
        {
            inZumo=false;
        }
    }

    motor_forward(0,0); // Stopping motor after match is over
    
    stop = xTaskGetTickCount();
    print_mqtt("Zumo032/stop", "%d", stop);
    
    end = stop - start;
    print_mqtt("Zumo032/time", "%d", end);
    
    while(1)
    {
        vTaskDelay(100);
    }  
} 
#endif


#if 0
// WEEK 1 EXERCISE 1
void zmain(void)
{
    printf("\nBoot\n");

    uint16_t Delay = 500;
    
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    vTaskDelay(Delay*5);
    BatteryLed_Write(1);
    vTaskDelay(Delay*3);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay*3);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay*3);
    BatteryLed_Write(0);
    vTaskDelay(Delay*5);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    vTaskDelay(Delay);
    BatteryLed_Write(1);
    vTaskDelay(Delay);
    BatteryLed_Write(0);
    
    
    //BatteryLed_Write(1); // Switch led on 
    //vTaskDelay(Delay);
    //BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    bool led = false;
    
    while(true)
    {
        // toggle led state when button is pressed
        //vTaskDelay(Delay); // sleep (in an infinite loop)
        if(SW1_Read() == 0) {
            led = !led;
            BatteryLed_Write(led);
            if(led) printf("Led is ON\n");
            else printf("Led is OFF\n");
            Beep(1000, 150);
            while(SW1_Read() == 0) vTaskDelay(10); // wait while button is being pressed
        }        
    }
 } 
#endif

#if 0
// WEEK 1 EXERCISE 3
void zmain(void)
{
    ADC_Battery_Start();        

    int16 adcresult =0;
    float volts = 0.0;
    int vref = 5;
    float vbat = 0.0;
    float adcres = 4095.0;
    float totalres = 1.5;
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed

    while(true)
    {
        char msg[80];
        ADC_Battery_StartConvert(); // start sampling
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            
            volts = vref * (adcresult/adcres);
            vbat = volts * totalres;
            // you need to implement the conversion
            
            // Print both ADC results and converted value
            printf("ADCRESULT is %d and VOLTS are %f\r\n",adcresult, volts);
            printf("BATTERY is %f\n", vbat);
            
            while (vbat < 4)
            {
                if(SW1_Read() == 0)
                {
                    break;   
                }
                else
                {
                    BatteryLed_Write(1); // Switch led on 
                    vTaskDelay(500);
                    BatteryLed_Write(0); // Switch led off
                    vTaskDelay(500);
                    BatteryLed_Write(1); // Switch led on 
                    vTaskDelay(500);
                    BatteryLed_Write(0); // Switch led off
                }
            }
        }
        vTaskDelay(1000);
    }
}    
#endif

#if 0
    //WEEK 2 EXERCISE 2
    //ultrasonic sensor//
void zmain(void)
{
    Ultra_Start();              // Ultra Sonic Start function
    motor_start();              // enable motor controller
    motor_forward(0,0);         // set speed to zero to stop motors
    
    while(true) {
        int d = Ultra_GetDistance();
        // Print the detected distance (centimeters)
        printf("DISTANCE = %d\r\n", d);
        motor_forward(100,1);     // moving forward
        
        if(d <= 15)
        {
            motor_forward(0,0);
            Beep(100,40);
            motor_backward(255,200);    // moving backward
            motor_turn(255,10,200);     // turn
        }
    }
} 
    
#endif

#if 0
// FOR RACE
void zmain(void)
{
    uint32_t IR_val;
    motor_start();
    motor_forward(0,0);
    
    struct sensors_ ref;
    struct sensors_ dig;
    
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    IR_Start();
    IR_flush();
    
    printf("\n\n ROBOT STARTING... \n\n");
    
    while(true)
    {
        motor_forward(50,0);
        reflectance_digital(&dig);
        if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_forward(0,0);
            IR_wait();
        }
    }
    
}
    
#endif

#if 0
// WEEK 4 EXERCISE 1

void zmain(void)
{
    int passedTimes=0;
    uint32_t IR_val;
    bool onLine, firstLine;
    
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    
    motor_start();
    motor_forward(0,0);
    IR_Start();
    IR_flush();

    printf("\n\nRobot starting...\n\n");
    
    while(passedTimes<4)
    {
        printf("PassedTimes is %d\n", passedTimes);
        onLine = false;
        firstLine = false;
        
        motor_forward(50,1);
        
        /*
        read raw sensor values
        reflectance_read(&ref);
        print out each period of reflectance sensors
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
        */
        reflectance_digital(&dig); 
        //print out 0 or 1 according to results of reflectance period
        //printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);  

        //    LEFT 3        LEFT 2        LEFT 1         RIGHT 1          RIGHT 2       RIGHT 3
        if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            onLine = true;
            passedTimes++;
            //vTaskDelay(150);
            printf("PassedTimes is %d\n", passedTimes);
            
        }
        while (onLine)
        {
            reflectance_digital(&dig);
            if (!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
            {
                onLine = false;
                printf("PassedTimes is %d\n", passedTimes);
                if(passedTimes==1)
                {
                    firstLine = true;
                    printf("PassedTimes is %d\n", passedTimes);
                }
                if(firstLine == true)
                {
                    motor_forward(0,0);
                    printf("PassedTimes is %d\n", passedTimes);
                    
                    if(IR_get(&IR_val, portMAX_DELAY))
                    {
                        firstLine = false;
                        printf("PassedTimes is %d\n", passedTimes);
                        motor_forward(50,100);
                    }
                }
            }
        }    
    }
    
    motor_forward(0, 0);
    
    while(1)
    {
        vTaskDelay(100);
    }
}

    
    
#endif

#if 0
// WEEK 4 EXERCISE 2

void zmain(void)
{
    int passedTimes=0;
    int x=180,y=180,z=250;
    uint32_t IR_val;
    bool onLine, firstLine;
    
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(7000, 7000, 10000, 10000, 7000, 7000); // set center sensor threshold to 11000 and others to 9000
    
    motor_start();
    motor_forward(0,0);
    IR_Start();
    IR_flush();

    printf("\n\nRobot starting...\n\n");
    
    while(passedTimes<6)
    {
        printf("PassedTimes is %d\n", passedTimes);
        onLine = false;
        firstLine = false;
        motor_forward(50,1);
        /*
        read raw sensor values
        reflectance_read(&ref);
        print out each period of reflectance sensors
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
        */
        reflectance_digital(&dig); 
        //print out 0 or 1 according to results of reflectance period
        //printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);  

        //    LEFT 3        LEFT 2        LEFT 1         RIGHT 1          RIGHT 2       RIGHT 3
        if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            onLine = true;
            passedTimes++;
            vTaskDelay(150);
            printf("PassedTimes is %d\n", passedTimes);
            
        }
        while (onLine)
        {
            reflectance_digital(&dig);
            if (!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
            {
                onLine = false;
                printf("PassedTimes is %d\n", passedTimes);
                if(passedTimes==1)
                {
                    firstLine = true;
                    printf("PassedTimes is %d\n", passedTimes);
                }
                else if(passedTimes==2)
                {
                    //motor_turn(0,240,400);
                    motor_reverse_turn_left(x, y, z);
                    reflectance_digital(&dig); 
                }
                else if(passedTimes==3)
                {
                    //motor_turn(240,0,400);
                    motor_reverse_turn_right(x, y, z);
                    reflectance_digital(&dig); 
                }
                else if(passedTimes==4)
                {
                    //motor_turn(240,0,400);
                    motor_reverse_turn_right(x, y, z);
                    reflectance_digital(&dig); 
                }
                else if(passedTimes==5)
                {
                    motor_forward(0,0);    
                }
                if(firstLine == true)
                {
                    motor_forward(0,0);
                    printf("PassedTimes is %d\n", passedTimes);
                    
                    if(IR_get(&IR_val, portMAX_DELAY))
                    {
                        firstLine = false;
                        printf("PassedTimes is %d\n", passedTimes);
                        motor_forward(50,100);
                    }
                }
            }
        }    
    }
    
    motor_forward(0, 0);
    
    while(1)
    {
        vTaskDelay(100);
    }
}
    
    
#endif

#if 0
    
// WEEK 4 EXERCISE 3
// WITHOUT EXTRA DIGITALS AND ON THE SMALL TRACK
    
void zmain(void)
{
    uint32_t IR_val;
    bool onTrack = false, confirmation = false;
    
    //struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(8000, 8000, 9000, 9000, 8000, 8000); // set center sensor threshold to 11000 and others to 9000
    
    motor_start();
    motor_forward(0,0);
    IR_Start();
    IR_flush();

    printf("\n\nRobot starting...\n\n");
    
    while(true)
    {
        if(SW1_Read() == 0)
        {
            motor_forward(100,1);
            
        }
        reflectance_digital(&dig);
        if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            printf("ROBOT ON LINE\n");
            confirmation = true;
            motor_forward(0,0);
            if(IR_get(&IR_val, portMAX_DELAY))
            {
                onTrack = true;
            }
        }
        while(onTrack == true && confirmation == true)
        {
            printf("ROBOT ON TRACK\n");
            reflectance_digital(&dig);
            // FORWARD
            if(dig.l1 == 1 && dig.r1 == 1)
            {
                motor_forward(100,0);
            }
            // STOP IF ON LINE AGAIN
            else if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
            {
                motor_turn(0,0,0);
                motor_forward(0,0);
                motor_stop();
                onTrack = false;
                confirmation = false;
                Beep(1000,100);
                Beep(1000,200);
                Beep(1000,100);
                Beep(1000,200);
                Beep(100,100);
                Beep(100,200);
                Beep(100,100);
                Beep(100,200);
            }
            // MILD TURN LEFT
            else if(dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1)
            {
                motor_turn(100,90,0);
            }
            // AVERAGE TURN LEFT
            else if(dig.r1 == 1)
            {
                motor_turn(100,60,0);
            }
            // TURN LEFT
            else if(dig.r1 == 1 && dig.r2 == 1)
            {
                motor_turn(90,40,0);
            }
            // HARD TURN LEFT
            else if(dig.r2 == 1 && dig.r3 == 1)
            {
                motor_turn(150,10,0);
            }
            // VERY HARD TURN LEFT
            else if(dig.r3 == 1)
            {
                motor_turn(240,10,0);
            }
            // MILD TURN RIGHT
            else if(dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1)
            {
                motor_turn(90,100,0);
            }
            // AVERAGE TURN RIGHT
            else if(dig.l1 == 1)
            {
                motor_turn(60,100,0);
            }
            // TURN RIGHT
            else if(dig.l2 == 1 && dig.l1 == 1)
            {
                motor_turn(40,90,0);
            }
            // HARD TURN RIGHT
            else if(dig.l3 == 1 && dig.l2 == 1)
            {
                motor_turn(10,150,0);
            }
            // VERY HARD TURN RIGHT
            else if(dig.l3 == 1)
            {
                motor_turn(10,240,0);
            }
        }
        confirmation = false;
    }
}
#endif

#if 0
    
// WEEK 4 EXERCISE 3
// WITH EXTRA DIGITAL
    
void zmain(void)
{
    uint32_t IR_val;
    bool onTrack = false, confirmation = false;
    
    //struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(8000, 8000, 9000, 9000, 8000, 8000); // set center sensor threshold to 11000 and others to 9000
    
    motor_start();
    motor_forward(0,0);
    IR_Start();
    IR_flush();

    printf("\n\nRobot starting...\n\n");
    
    while(true)
    {
        if(SW1_Read() == 0)
        {
            motor_forward(100,1);
            
        }
        reflectance_digital(&dig);
        if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            printf("ROBOT ON LINE\n");
            confirmation = true;
            motor_forward(0,0);
            if(IR_get(&IR_val, portMAX_DELAY))
            {
                onTrack = true;
            }
        }
        while(onTrack == true && confirmation == true)
        {
            printf("ROBOT ON TRACK\n");
            reflectance_digital(&dig);
            // FORWARD
            if(dig.l1 == 1 && dig.r1 == 1)
            {
                motor_forward(160,0);
                reflectance_digital(&dig);
            }
            // STOP IF ON LINE AGAIN
            else if(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
            {
                onTrack = false;
                confirmation = false;
                motor_turn(0,0,0);
                motor_forward(0,0);
                motor_stop();
                Beep(1000,100);
                Beep(1000,200);
                Beep(1000,100);
                Beep(1000,200);
                Beep(100,100);
                Beep(100,200);
                Beep(100,100);
                Beep(100,200);
            }
            // MILD TURN LEFT
            else if(dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1)
            {
                motor_turn(160,140,0);
                reflectance_digital(&dig);
            }
            // AVERAGE TURN LEFT
            else if(dig.r1 == 1)
            {
                motor_turn(150,120,0);
                reflectance_digital(&dig);
            }
            // TURN LEFT
            else if(dig.r1 == 1 && dig.r2 == 1)
            {
                motor_turn(190,40,0);
                reflectance_digital(&dig);
            }
            /* HARD TURN LEFT
            else if(dig.r2 == 1 && dig.r3 == 1)
            {
                motor_turn(220,20,0);
                reflectance_digital(&dig);
            }*/
            // VERY HARD TURN LEFT
            else if(dig.r3 == 1 && dig.r2 == 1)
            {
                motor_turn(250,0,0);
                reflectance_digital(&dig);
            }
            // MILD TURN RIGHT
            else if(dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1)
            {
                motor_turn(140,160,0);
                reflectance_digital(&dig);
            }
            // AVERAGE TURN RIGHT
            else if(dig.l1 == 1)
            {
                motor_turn(120,150,0);
                reflectance_digital(&dig);
            }
            // TURN RIGHT
            else if(dig.l2 == 1 && dig.l1 == 1)
            {
                motor_turn(40,190,0);
                reflectance_digital(&dig);
            }
            /* HARD TURN RIGHT
            else if(dig.l3 == 1 && dig.l2 == 1)
            {
                motor_turn(20,220,0);
                reflectance_digital(&dig);
            }*/
            // VERY HARD TURN RIGHT
            else if(dig.l3 == 1 && dig.l2 == 1)
            {
                motor_turn(0,250,0);
                reflectance_digital(&dig);
            }
        }
        confirmation = false;
    }
}
#endif

#if 0
// WEEK 4 ass 3
// Jonne Version :P
void zmain(void)
{
    int passedTimes=0, button=0;
    uint32_t IR_val;
    bool onLine, firstLine;
    
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    motor_start();
    motor_forward(0,0);
    
    IR_Start();
    IR_flush();
    
    RTC_TIME_DATE now;
    now.Hour = 0;
    now.Min = 0;
    now.Sec = 0;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2018;
    //RTC_WriteTime(&now);

    send_mqtt("Zumo047/debug", "Boot\n");
    printf("\n\nRobot Starting...\n\n");
 
    while(passedTimes<2)
    {
        onLine = false;
        firstLine = false;

        reflectance_digital(&dig); 
  
        if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_forward(120,1);
            printf("forward\n");
        }
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 0)
        {
            motor_turn(200,150,1);
            printf("mild right\n");
        }
        else if (dig.l3 == 0 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(150,200,1);
            printf("mild left\n");
        }
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 1 && dig.r3 == 1)
        {
            motor_turn(200,80,1);
            printf("hard right\n");
        }
        else if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(80,200,1);
            printf("hard left\n");
        }
        else if (dig.l3 == 0 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 1)
        {
            motor_turn(255,10,1);
            printf("hard right2\n");
        }
        else if (dig.l3 == 1 && dig.l2 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r2 == 0 && dig.r3 == 0)
        {
            motor_turn(10,255,1);
            printf("hard left2\n");
        }
        //    LEFT 3        LEFT 2        LEFT 1         RIGHT 1          RIGHT 2       RIGHT 3
        if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            onLine = true;
            passedTimes++;
            printf("PassedTimes is %d\n", passedTimes);        
        }

        while (onLine)
        {
            reflectance_digital(&dig);
            if (!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
            {
                onLine = false;
                printf("PassedTimes is %d\n", passedTimes);
                if(passedTimes==1)
                {
                    firstLine = true;
                    printf("PassedTimes is %d\n", passedTimes);
                }
                else if(passedTimes==2)
                {
                    motor_forward(0, 0);
                }
                 
                if(firstLine == true)
                {
                    motor_forward(0,100);
                    printf("PassedTimes is %d\n", passedTimes);
                    
                    if(IR_get(&IR_val, portMAX_DELAY))
                    {
                        firstLine = false;
                        printf("PassedTimes is %d\n", passedTimes);
                        motor_forward(50,100);
                        
                        // read the current time
                        RTC_DisableInt(); /* Disable Interrupt of RTC Component */
                         now = *RTC_ReadTime(); /* copy the current time to a local variable */
                        RTC_EnableInt(); /* Enable Interrupt of RTC Component */

                        // print the current time
                        printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
                        print_mqtt("Zumo047/lap", "%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
                    }
                }
            }
        }    
    }
 

    
    motor_forward(0, 0);
    
    // read the current time
    RTC_DisableInt(); /* Disable Interrupt of RTC Component */
    now = *RTC_ReadTime(); /* copy the current time to a local variable */
    RTC_EnableInt(); /* Enable Interrupt of RTC Component */

    // print the current time
    printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
    print_mqtt("Zumo047/lap", "%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
    
    while(1)
    {
        vTaskDelay(100);
    }
}

#endif    
    
#if 0
    
// WEEK 5 EXERCISE 1    
    
// MQTT test
void zmain(void)
{
    int ctr = 0, x = 0, y = 0;
    
    printf("\nBoot\n");
    send_mqtt("Zumo047/debug", "Booty");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    vTaskDelay(10000);
    printf("Enter current hour:");
    scanf("%d", &x);
    printf("Enter current minute:");
    scanf("%d", &y);
    print_mqtt("Zumo047/debug", "Entered hour is %d and minute is %d", x, y);
    
    
    RTC_TIME_DATE now;

    // set current time
    now.Hour = x;
    now.Min = y;
    now.Sec = 0;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2019;
    RTC_WriteTime(&now); // write the time to real time clock

    
    RTC_Start(); // start real time clock

    while(true)
    {
        
        if(SW1_Read() == 0) {
            // read the current time
            RTC_DisableInt(); /* Disable Interrupt of RTC Component */
            now = *RTC_ReadTime(); /* copy the current time to a local variable */
            RTC_EnableInt(); /* Enable Interrupt of RTC Component */

            // print the current time
            printf("%2d:%02d\n", now.Hour, now.Min);
            print_mqtt("Zumo047/debug", "%2d:%02d\n", now.Hour, now.Min);
            
            // wait until button is released
            while(SW1_Read() == 0) vTaskDelay(50);
            
            printf("Ctr: %d, Button: %d\n", ctr, SW1_Read());
            //print_mqtt("Zumo047/debug", "Ctr: %d, Button: %d", ctr, SW1_Read());
            ctr++;
        }
        vTaskDelay(50);
    }    
 } 
    
#endif

#if 0

//WEEK 5 EXERCISE 2
//Ultrasonic sensor on MQTT//
void zmain(void)
{
    int ctr = 0;
    int rndNum;
    
    printf("\nBoot\n");
    send_mqtt("Zumo047/debug", "Boot");

    srand(time(0));
    
    Ultra_Start();              // Ultra Sonic Start function
    motor_start();              // enable motor controller
    motor_forward(0,0);         // set speed to zero to stop motors
    
    while(true) {
        int d = Ultra_GetDistance();
        // Print the detected distance (centimeters)
        printf("DISTANCE = %d\r\n", d);
        motor_forward(100,0);     // moving forward
        printf("Ctr: %d, Button: %d\n", ctr, SW1_Read());
        //print_mqtt("Zumo047/debug", "Ctr: %d, Button: %d, Distance: %d", ctr, SW1_Read(), d);
        ctr++;
        
        rndNum = rand() % 2;
        
        
        if(d <= 15)
        {
            print_mqtt("Zumo047/debug", "OBSTACLE DETECTED AND REVERSING");
            print_mqtt("Zumo047/debug", "Random Number is %d", rndNum);
            motor_forward(0,0);
            Beep(100,40);
            motor_backward(255,200);    // moving backward
            if(rndNum == 1)
            {
                motor_turn(255,10,200);     // turn
            }
            else if(rndNum == 0)
            {
                motor_turn(10,255,200);     // turn
            }
        }
    }
}

#endif


#if 0
//WEEK 5 EXERCISE 3
void zmain(void)
{
    bool onLine;
    RTC_Start();
    
    RTC_TIME_DATE now;

    // set current time
    now.Hour = 0;
    now.Min = 0;
    now.Sec = 0;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2018;
    //RTC_WriteTime(&now);
    
    motor_start();
    motor_forward(0,0);
    
    IR_Start();
    IR_flush();
    
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    send_mqtt("Zumo047/debug", "Boot");
    printf("Robot Starting...");


    while(true)
    {
        onLine = false;
        
        motor_forward(50,1);
       
        reflectance_digital(&dig); 
        
        
        //    LEFT 3        LEFT 2        LEFT 1         RIGHT 1          RIGHT 2       RIGHT 3
        if (dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1)
        {
            // read the current time
            RTC_DisableInt(); /* Disable Interrupt of RTC Component */
            now = *RTC_ReadTime(); /* copy the current time to a local variable */
            RTC_EnableInt(); /* Enable Interrupt of RTC Component */

            // print the current time
            printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
            print_mqtt("Zumo047/lap", "%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
            onLine = true;
            
        }
        while (onLine)
        {
            RTC_WriteTime(&now);
            reflectance_digital(&dig);
            if (!(dig.l3 == 1 && dig.l2 == 1 && dig.l1 == 1 && dig.r1 == 1 && dig.r2 == 1 && dig.r3 == 1))
            {
                //onLine = false;
                motor_forward(0,0);
                
                if(SW1_Read() == 0)
                {
                    // read the current time
                    RTC_DisableInt(); /* Disable Interrupt of RTC Component */
                    now = *RTC_ReadTime(); /* copy the current time to a local variable */
                    RTC_EnableInt(); /* Enable Interrupt of RTC Component */

                    // print the current time
                    printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
                    print_mqtt("Zumo047/lap", "%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
                    onLine = false;
                    motor_forward(50,100);
                }
            }
        }
    }    
}

    
#endif

#if 0
// button
void zmain(void)
{
    while(true) {
        printf("Press button within 5 seconds!\n");
        int i = 50;
        while(i > 0) {
            if(SW1_Read() == 0) {
                break;
            }
            vTaskDelay(100);
            --i;
        }
        if(i > 0) {
            printf("Good work\n");
            while(SW1_Read() == 0) vTaskDelay(10); // wait until button is released
        }
        else {
            printf("You didn't press the button\n");
        }
    } 
}
#endif
#if 0
// Hello World!
void zmain(void)
{
    printf("\nHello, World!\n");

    while(true)
    {
        vTaskDelay(100); // sleep (in an infinite loop)
    }
 }   
#endif

#if 0
// Name and age
void zmain(void)
{
    char name[32];
    int age;
    
    
    printf("\n\n");
    
    printf("Enter your name: ");
    //fflush(stdout);
    scanf("%s", name);
    printf("Enter your age: ");
    //fflush(stdout);
    scanf("%d", &age);
    
    printf("You are [%s], age = %d\n", name, age);

    while(true)
    {
        BatteryLed_Write(!SW1_Read());
        vTaskDelay(100);
    }
 }   
#endif


#if 0
//battery level//
void zmain(void)
{
    ADC_Battery_Start();        

    int16 adcresult =0;
    float volts = 0.0;

    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed

    while(true)
    {
        char msg[80];
        ADC_Battery_StartConvert(); // start sampling
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for ADC converted value
            adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
            // convert value to Volts
            // you need to implement the conversion
            
            // Print both ADC results and converted value
            printf("%d %f\r\n",adcresult, volts);
        }
        vTaskDelay(500);
    }
 }   
#endif

#if 0
// button
void zmain(void)
{
    while(true) {
        printf("Press button within 5 seconds!\n");
        int i = 50;
        while(i > 0) {
            if(SW1_Read() == 0) {
                break;
            }
            vTaskDelay(100);
            --i;
        }
        if(i > 0) {
            printf("Good work\n");
            while(SW1_Read() == 0) vTaskDelay(10); // wait until button is released
        }
        else {
            printf("You didn't press the button\n");
        }
    }
}
#endif

#if 0
// button
void zmain(void)
{
    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    // SW1_Read() returns zero when button is pressed
    // SW1_Read() returns one when button is not pressed
    
    bool led = false;
    
    while(true)
    {
        // toggle led state when button is pressed
        if(SW1_Read() == 0) {
            led = !led;
            BatteryLed_Write(led);
            if(led) printf("Led is ON\n");
            else printf("Led is OFF\n");
            Beep(1000, 150);
            while(SW1_Read() == 0) vTaskDelay(10); // wait while button is being pressed
        }        
    }
 }   
#endif


#if 0
//ultrasonic sensor//
void zmain(void)
{
    Ultra_Start();                          // Ultra Sonic Start function
    
    while(true) {
        int d = Ultra_GetDistance();
        // Print the detected distance (centimeters)
        printf("distance = %d\r\n", d);
        vTaskDelay(200);
    }
}   
#endif

#if 0
//IR receiverm - how to wait for IR remote commands
void zmain(void)
{
    IR_Start();
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    bool led = false;
    // Toggle led when IR signal is received
    while(true)
    {
        IR_wait();  // wait for IR command
        led = !led;
        BatteryLed_Write(led);
        if(led) printf("Led is ON\n");
        else printf("Led is OFF\n");
    }    
 }   
#endif



#if 0
//IR receiver - read raw data
void zmain(void)
{
    IR_Start();
    
    uint32_t IR_val; 
    
    printf("\n\nIR test\n");
    
    IR_flush(); // clear IR receive buffer
    printf("Buffer cleared\n");
    
    // print received IR pulses and their lengths
    while(true)
    {
        if(IR_get(&IR_val, portMAX_DELAY)) {
            int l = IR_val & IR_SIGNAL_MASK; // get pulse length
            int b = 0;
            if((IR_val & IR_SIGNAL_HIGH) != 0) b = 1; // get pulse state (0/1)
            printf("%d %d\r\n",b, l);
        }
    }    
 }   
#endif


#if 0
//reflectance
void zmain(void)
{
    struct sensors_ ref;
    struct sensors_ dig;

    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000); // set center sensor threshold to 11000 and others to 9000
    

    while(true)
    {
        // read raw sensor values
        reflectance_read(&ref);
        // print out each period of reflectance sensors
        printf("%5d %5d %5d %5d %5d %5d\r\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
        
        // read digital values that are based on threshold. 0 = white, 1 = black
        // when blackness value is over threshold the sensors reads 1, otherwise 0
        reflectance_digital(&dig); 
        //print out 0 or 1 according to results of reflectance period
        printf("%5d %5d %5d %5d %5d %5d \r\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);        
        
        vTaskDelay(200);
    }
}   
#endif


#if 0
//motor
void zmain(void)
{
    motor_start();              // enable motor controller
    motor_forward(0,0);         // set speed to zero to stop motors

    vTaskDelay(3000);
    
    motor_forward(100,2000);     // moving forward
    motor_turn(200,50,2000);     // turn
    motor_turn(50,200,2000);     // turn
    motor_backward(100,2000);    // moving backward
     
    motor_forward(0,0);         // stop motors

    motor_stop();               // disable motor controller
    
    while(true)
    {
        vTaskDelay(100);
    }
}
#endif

#if 0
/* Example of how to use te Accelerometer!!!*/
void zmain(void)
{
    struct accData_ data;
    
    printf("Accelerometer test...\n");

    if(!LSM303D_Start()){
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        vTaskSuspend(NULL);
    }
    else {
        printf("Device Ok...\n");
    }
    
    while(true)
    {
        LSM303D_Read_Acc(&data);
        printf("%8d %8d %8d\n",data.accX, data.accY, data.accZ);
        vTaskDelay(50);
    }
 }   
#endif    

#if 0
// MQTT test
void zmain(void)
{
    int ctr = 0;

    printf("\nBoot\n");
    send_mqtt("Zumo01/debug", "Boot");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 

    while(true)
    {
        printf("Ctr: %d, Button: %d\n", ctr, SW1_Read());
        print_mqtt("Zumo01/debug", "Ctr: %d, Button: %d", ctr, SW1_Read());

        vTaskDelay(1000);
        ctr++;
    }
 }   
#endif


#if 0
void zmain(void)
{    
    struct accData_ data;
    struct sensors_ ref;
    struct sensors_ dig;
    
    printf("MQTT and sensor test...\n");

    if(!LSM303D_Start()){
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        vTaskSuspend(NULL);
    }
    else {
        printf("Accelerometer Ok...\n");
    }
    
    int ctr = 0;
    reflectance_start();
    while(true)
    {
        LSM303D_Read_Acc(&data);
        // send data when we detect a hit and at 10 second intervals
        if(data.accX > 1500 || ++ctr > 1000) {
            printf("Acc: %8d %8d %8d\n",data.accX, data.accY, data.accZ);
            print_mqtt("Zumo01/acc", "%d,%d,%d", data.accX, data.accY, data.accZ);
            reflectance_read(&ref);
            printf("Ref: %8d %8d %8d %8d %8d %8d\n", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);       
            print_mqtt("Zumo01/ref", "%d,%d,%d,%d,%d,%d", ref.l3, ref.l2, ref.l1, ref.r1, ref.r2, ref.r3);
            reflectance_digital(&dig);
            printf("Dig: %8d %8d %8d %8d %8d %8d\n", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
            print_mqtt("Zumo01/dig", "%d,%d,%d,%d,%d,%d", dig.l3, dig.l2, dig.l1, dig.r1, dig.r2, dig.r3);
            ctr = 0;
        }
        vTaskDelay(10);
    }
 }   

#endif

#if 0
void zmain(void)
{    
    RTC_Start(); // start real time clock
    
    RTC_TIME_DATE now;

    // set current time
    now.Hour = 12;
    now.Min = 34;
    now.Sec = 56;
    now.DayOfMonth = 25;
    now.Month = 9;
    now.Year = 2018;
    RTC_WriteTime(&now); // write the time to real time clock

    while(true)
    {
        if(SW1_Read() == 0) {
            // read the current time
            RTC_DisableInt(); /* Disable Interrupt of RTC Component */
            now = *RTC_ReadTime(); /* copy the current time to a local variable */
            RTC_EnableInt(); /* Enable Interrupt of RTC Component */

            // print the current time
            printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
            
            // wait until button is released
            while(SW1_Read() == 0) vTaskDelay(50);
        }
        vTaskDelay(50);
    }
 }   
#endif

#if 0
void zmain(void)
{
    musicPlay();
}
void musicPlay()
{
    printf("Music is playing");
    Beep(10, 40);
    vTaskDelay(10);
    Beep(10, 40);
    vTaskDelay(10);
    Beep(10, 40);
    vTaskDelay(10);
    Beep(500, 60);
    vTaskDelay(10);
    Beep(500, 75);
    vTaskDelay(10);
    Beep(500, 80);
    vTaskDelay(10);
    Beep(100, 40);
    vTaskDelay(10);
    Beep(100, 60);
    vTaskDelay(10);
    Beep(500, 80);
}
    
#endif

/* [] END OF FILE */
