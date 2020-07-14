/*                       Digital to Analog Converter                         */
/*                       TARGET DEVICE: PIC18F27K40                          */
/*****************************************************************************/

//Included PIC18F27K40 header file to gain access to PIC specific functions 
#include <18F27K40.h> 

//Setting fuses using preprocessor directive (#fuses)
//CLKOUT - Clock output enable
//NOMCLR - Master clear disabled
//NOPUT - Power-up timer disabled
//NOWDT - Windowed watchdog timer disabled
//NOLVP - Low-voltage programming disabled
//NOPROTECT - User NVM code protection disabled
//NOCPD - Data NVM code protection disabled
#fuses CLKOUT NOMCLR NOPUT NOWDT NOLVP NOPROTECT NOCPD
#use delay(internal=32MHZ,clock_out)   //set clock speed to 32MHZ
#pin_select PWM4=pin_A7                //RA7 route to output of PWM4
#pin_select SDO2=pin_C6                //RC6 route to output of SDO2
#pin_select SCK2=pin_C7                //RC7 route to output of SCK2
#use SPI(SPI2, MASTER, MODE=0, BITS=8) //SPI setup with mastermode & 8 bits

//Structure definitions
struct IO_def1
{
   int ExperimentSelection:4; //RA0..3 used for experiment selection (EX2)
   int1 Debug;                //RA4 used for debug in interrupt      (EX3)
   int1 unused_A1;            //RA5 unused
   int1 Clock_Output;         //RA6 clock output                     (EX2)
   int1 LDAC_OUT;             //RA7 used for LDAC signal             (EX3)
   int B_inputs:4;            //RB0..3 for inputs                    (EX2)
   int1 CS;                   //RB4 used for CS signal               (EX3)
   int unused_B:3;            //RB5..7 unused 
   int Multi_LED:6;           //RC0..5 used to display output        (EX2)
   int1 SDO2;                 //RC6 as SDO                           (EX3)
   int1 SCK2;                 //RC7 as SCK                           (EX3)
};

//Structure delcarations
struct IO_def1 IO_Port_1;       //Structure mapping to PORTA,PORTB,PORTC
struct IO_def1 IO_Port_1_Latch; //Structure for mapping to LATA,LATB,LATC
struct IO_def1 IO_Port_1_Direction; //Structure  mapping to TRISA,TRSIB,TRISC 

#byte IO_Port_1 = 0xF8D //Load structure to addresses of PORTA,PORTB,PORTC
#byte IO_Port_1_Latch = 0xF83 //Load structure to addresses of LATA,LATB,LATC
#byte IO_Port_1_Direction = 0xF88 //Load structure to addresses of PORTA,B,C

// Global Variables
int LUT_index=0; //set up index for looping

//Setting LUT for each waveform
const int16 Sine_1KHZ[32]={
   0x17FF,0x1972,0x1AD6,0x1C1F,0x1D3F,0x1E2B,0x1EDA,0x1F46,
   0x1F6B,0x1F46,0x1EDA,0x1E2B,0x1D3F,0x1C1F,0x1AD6,0x1972,
   0x17FF,0x168C,0x1528,0x13DF,0x12BF,0x11D3,0x1124,0x10B8,
   0x1093,0x10B8,0x1124,0x11D3,0x12BF,0x13DF,0x1528,0x168C};
                           
const int16 Sine_2KHZ[32]={
   0x17FF,0x1AD6,0x1D3F,0x1EDA,0x1F6B,0x1EDA,0x1D3F,0x1AD6,
   0x17FF,0x1528,0x12BF,0x1124,0x1093,0x1124,0x12BF,0x1528,
   0x17FF,0x1AD6,0x1D3F,0x1EDA,0x1F6B,0x1EDA,0x1D3F,0x1AD6,
   0x17FF,0x1528,0x12BF,0x1124,0x1093,0x1124,0x12BF,0x1528};
                           
const int16 Sine_1and2KHZ[32]={
   0x17FF,0x1A41,0x1C41,0x1DC6,0x1EAA,0x1EDA,0x1E5E,0x1D52,
   0x1BE7,0x1A55,0x18D8,0x17A3,0x16DA,0x168F,0x16BB,0x1743,
   0x17FF,0x18BB,0x1943,0x196F,0x1924,0x185B,0x1726,0x15A9,
   0x1417,0x12AC,0x11A0,0x1124,0x1154,0x1238,0x13BD,0x15BD};
                  
#int_timer2 //Timer 2 interrupt directive
void Timer2_Interrupt_function(void) //timer 2 interrupt routine
{
   IO_Port_1.Debug=0b1; //set debug signal high
   IO_Port_1.CS=0b0;    //set cs signal low
   switch(IO_Port_1.ExperimentSelection) //Switch case use to transmit data
   {
      case 0b0000: //switch 0000 detected
      {
            spi_write2(Sine_1KHZ[LUT_index]>>8);         //transmit high byte
            spi_write2(Sine_1KHZ[LUT_index]&0x00FF);     //transmit low byte 
            break;
      }
      case 0b0001: //switch 0001 detected
      {
            spi_write2(Sine_2KHZ[LUT_index]>>8);         //transmit high byte
            spi_write2(Sine_2KHZ[LUT_index]&0x00FF);     //transmit low byte
            break;
      }
      case 0b0010: //switch 0010 detected
      {
            spi_write2(Sine_1and2KHZ[LUT_index]>>8);     //transmit high byte
            spi_write2(Sine_1and2KHZ[LUT_index]&0x00FF); //transmit low byte  
            break;
      }
   }   
   LUT_index=(LUT_index+1)%32;   //update index
   IO_Port_1.CS=0b1;             //set cs high
   IO_Port_1.Debug=0b0;          //set debug low
}

void main()
{
   //Setting direction
   IO_Port_1_Direction.ExperimentSelection=0b1111; //RA0..3 as inputs
   IO_Port_1_Direction.Debug=0b0;                  //RA4 as output
   IO_Port_1_Direction.unused_A1=0b0;              //RA5 as output
   IO_Port_1_Direction.Clock_Output=0b0;           //RA6 as output
   IO_Port_1_Direction.LDAC_OUT=0b0;               //RA7 as output
   IO_Port_1_Direction.B_inputs=0b0000;            //RB0..3 as output(unused)
   IO_Port_1_Direction.CS=0b0;                     //RB4 as output
   IO_Port_1_Direction.unused_B=0b000;             //RB5..7 as output
   IO_Port_1_Direction.Multi_LED=0b000000;         //RC0..5 as output
   IO_Port_1_Direction.SDO2=0b0;                   //RC6 as output
   IO_Port_1_Direction.SCK2=0b0;                   //RC7 as output
   
   port_a_pullups(0x0F); //Pullup resistor RA0..3 enabled

   //Set CCP to operate in PWM mode with timer 2 selected
   setup_ccp2(CCP_PWM|CCP_USE_TIMER1_AND_TIMER2);
   //Enable PWM4 with active high and using timer 2
   setup_pwm4(PWM_ENABLED|PWM_ACTIVE_HIGH|PWM_TIMER2);
   //Setting timer 2 to produce 32KHz
   setup_timer_2(T2_CLK_INTERNAL | T2_DIV_BY_1, 249,1);
   //Setting PWM4 pulse width to be 30.25 us
   set_pwm4_duty(968);

  enable_interrupts(INT_TIMER2); //Enable timer 2 interrupt
  enable_interrupts(GLOBAL);     //Enable global interrupt
  while(TRUE)
  {
   //Prevents microprocessor from sleeping
  } 
}




