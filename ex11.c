
#include<p18f4520.h>
#include<stdio.h>

#pragma config OSC = HS 
#pragma config LVP = OFF 
#pragma config WDT = OFF
#pragma config PBADEN = OFF

#define rs PORTCbits.RC0 
#define rw PORTCbits.RC1
#define en PORTCbits.RC2 

#define v_input PORTAbits.RA0 

#define trig PORTBbits.RB1 
#define echo PORTBbits.RB4 

#define solenoid_valve PORTEbits.RE0 
#define res_factor 6                   //value of voltage per cm (inverse relation)

void delay(int cnt);
void lcd_cmd(unsigned char val);
void lcd_data(unsigned int val);
void LCD_String(char msg[]);
void lcd_init();
void adc_init();

unsigned int  adc_out(int analog_value);
void start_filling(unsigned int);

void main()
{    	
	float v_height,req_level;
   	unsigned int dig_req_level;
    int i;
    char a[] = "EveryDropCounts";
    delay(6000);
    
   	TRISD=0X00;                      //OUTPUT
    TRISCbits.TRISC0=0;              //Setting rs ,
    TRISCbits.TRISC1=0;              //rw, and
    TRISCbits.TRISC2=0;              //en
        
    adc_init();
    lcd_init();
    
    LCD_String(a);
         
   	v_height= 30-(v_input*res_factor);
   	req_level = 0.95 * v_height;
   	dig_req_level = adc_out(req_level);
   	start_filling(dig_req_level);
   
   while(1);
}	

void delay(int cnt)
{
   int i;
   for(i=0;i<cnt;i++);
}

void lcd_cmd(unsigned char val)
{
   PORTD=val;
   rs=0;
   rw=0; 
   en=1;
   delay(2500);
   en=0;
}

void lcd_data(unsigned int val)
{
   PORTD=val;
   rs=1;
   rw=0; 
   en=1;
   delay(2500);
   en=0;
}

void LCD_String(char a[])
{   
	int i;
	for(i=0;a[i]!='\0';i++)
      {
	      lcd_data(a[i]);
	      delay(100);
   	  }
}
  
void lcd_init()
{
         lcd_cmd(0x38); //2 lines and 5x7 matrix
         delay( 1000);
         lcd_cmd(0x01); //clear LCD display
         delay( 1000);
         lcd_cmd(0x0e); //Display on, cursor blinking
         delay( 1000);
         lcd_cmd(0x80); //force cursor to beginning of 1st line
         delay( 1000);
         lcd_cmd(0x0c); //display on ,cursor off
         delay( 1000);
         lcd_cmd(0x80);  // display on llne 1
         delay( 1000);

}


void adc_init()
{
    ADCON0=0x00;				
    ADCON1=0x0E;				
    ADCON2=0x8E; 				
    ADCON0bits.ADON=1; 
}

unsigned int  adc_out(int analog_value)
{
	unsigned int digital_op;
	ADCON0bits.GO = 1;                                       
   	while(ADCON0bits.GO == 1);
    digital_op=ADRESL;
    digital_op|=((unsigned int)ADRESH) << 8;
	return digital_op;
}

void start_filling(unsigned int dig_req_level)
{   
	unsigned int current_level;   
	float Time, Distance;
	char str[] = "Water Level: ";
    int a[2], dist;  // from the range of the Ultrasonic sensor, we know that the value of distance will not
	                 // be greater than 3 digits, in our case, it's even smaller---> 2 digits
	    
	    INTCON2bits.RBPU=0;	//Pull up for PortB
	    
	    TRISBbits.TRISB1 = 0;  // TRIG---> OUTPUT
	    TRISBbits.TRISB4 = 1;  // ECHO---> INPUT
	    TRISEbits.TRISE0 = 0; 

		solenoid_valve = 1;
		T1CON = 0X80;
		
	while(current_level <= dig_req_level)
	{  
		trig=1;
		delay(50);
		trig=0;             	//Transmit 10us pulse to HC-SR04 
		
		  lcd_cmd(0x01);
		  delay(1000);
		  	       
          while(echo==0);	            // Wait for rising edge at Echo pin 
          TMR1L=0;		         	// Load Timer1 register with 0 
          TMR1H=0;
          T1CONbits.TMR1ON=1;		//Turn ON Timer1
          while(echo==1 && !PIR1bits.TMR1IF);    // Wait for falling edge 
          Time = ((TMR1L)|(TMR1H<<8));		//Copy Time when echo is received 
          T1CONbits.TMR1ON=0;			//Turn OFF Timer1 */
          Distance = ((float)Time*17/2000); // Distance =(velocity x Time)/2 
          current_level = 30-Distance;
          dist = (int)current_level;
          
		//Next, we display this value on the LCD
	      a[0] = dist/10;
	      a[1] = dist%10;
	    
	    lcd_cmd(0xC0);  // Display on line 2
	    delay(1000);
	    LCD_String(str);
	    lcd_data(a[0]+0x30);
	    delay(1000);
	    lcd_data(a[1]+0x30);
	    delay(1000);	    	

  }  
     
         solenoid_valve = 0;
}
	



