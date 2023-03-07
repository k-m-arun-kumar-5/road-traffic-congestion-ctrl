/* ********************************************************************
FILE                   : timer13.c

PROGRAM DESCRIPTION    :  4 way traffic control sequence on one lane present in each way with GO sequence E-W-N-S direction, with density control. 
 * Next sequence direction Go means next direction traffic GO state eg, if in east direction traffic is in GO, 
   then next direction traffic GO is West.
 * If next sequence directions traffic congestion are in level 0 ie less traffic congestion,
 then use normal level 0 Green for next sequence direction traffic GO,
 * eg if in east direction traffic is in GO, then in West GO duration = normal level 0 West GO duration time.
 If next sequence direction has level 1 traffic congestion ie level 1 congestion more than level 0 congestion,  * 
 *then switching to this sequence will have more green duration time than level 0 green duration time.
 * eg.if in east direction traffic is in GO, then in West GO duration =  level 1 West GO duration time and 
 * level 1 West GO duration time >  level 0 West GO duration time. 
 * If next sequence direction has level 2 traffic congestion ie, level 2 congestion  more than level 1 congestion, 
 * then switching to this sequence will have more green duration time than level 1 green duration time.
 * eg.if in east direction traffic is in GO, then in West GO duration =  level 2 West GO duration time and 
 * level 2 West GO duration time >  level 1 West GO duration time. We use IR_LEVEL1_SW PRESSED ON to indicate level 1 congestion and
   IR_LEVEL2_SW PRESSED ON to indicate level 2 congestion for each direction 

AUTHOR                : K.M.Arun Kumar alias Arunkumar Murugeswaran
	 
KNOWN BUGS            : fix issue of timer to get precise time using timer. Use PIC16F887-repo->05_timer->timer17.X->timer.c as base code to get precise time using timer.

NOTE                  : 
                       
CHANGE LOGS           : 

*****************************************************************************/    
#ifdef HI_TECH_COMPILER
  #include <pic.h>
 __CONFIG(0X2CE4);
#else
  #include <xc.h>
#endif

#define E_RED_PIN                   RA0
#define E_YELLOW_PIN                RA1
#define E_GREEN_PIN                 RA2 
#define E_GREEN_RIGHT_PIN           RA3
#define W_RED_PIN                   RA4
#define W_YELLOW_PIN                RA5
#define W_GREEN_PIN                 RA6 
#define W_GREEN_RIGHT_PIN           RA7
#define W_E_PORT                  PORTA

#define N_RED_PIN                   RC0
#define N_YELLOW_PIN                RC1
#define N_GREEN_PIN                 RC2 
#define N_GREEN_RIGHT_PIN           RC3
#define S_RED_PIN                   RC4
#define S_YELLOW_PIN                RC5
#define S_GREEN_PIN                 RC6 
#define S_GREEN_RIGHT_PIN           RC7
#define S_N_PORT                    PORTC

#define RS_PIN                       RE0
#define RW_PIN                       RE1
#define EN_PIN                       RE2
#define LCD_PORT                     PORTD

#define SW_E_IR_LEVEL1                  RB0  
#define SW_E_IR_LEVEL2                  RB1 
#define SW_W_IR_LEVEL1                  RB2 
#define SW_W_IR_LEVEL2                  RB3 
#define SW_N_IR_LEVEL1                  RB4 
#define SW_N_IR_LEVEL2                  RB5 
#define SW_S_IR_LEVEL1                  RB6 
#define SW_S_IR_LEVEL2                  RB7 

#define LED_OFF                      (0)
#define LED_ON                       (1)  
#define SW_PRESSED_ON                 (1)
#define SW_NOT_PRESSED                (0)
 
#define TICK_MS  (50UL) // TIMER1 expires every 50ms
#define TIME_UNIT (1000UL)
#define OSC_PER_INST (4)
#define UPDATE_TIME (1000UL)/TICK_MS //UPDATE_TIME configured for 1 sec
#define _XTAL_FREQ   (4000000UL)
#define INC (unsigned int)((unsigned long)(_XTAL_FREQ * TICK_MS) / (unsigned long)(OSC_PER_INST * TIME_UNIT))
 
/* for 20 * 4 LCD disp */                             
#define BEGIN_LOC_LINE1                      (0X80u)
#define BEGIN_LOC_LINE2                      (0xC0u)
#define BEGIN_LOC_LINE3                      (0x94u) 
#define BEGIN_LOC_LINE4                      (0xD4u)
#define END_LOC_LINE1                        (0x93u)
#define END_LOC_LINE2                        (0xD3u)
#define END_LOC_LINE3                        (0xA7u) 
#define END_LOC_LINE4                        (0xE7u)

#define NUM_LINE1                  (1u)
#define NUM_LINE2                  (2u)
#define NUM_LINE3                  (3u)
#define NUM_LINE4                  (4u)
#define NUM_COL1                   (1u)
 
#define EAST_LINE_NUM            NUM_LINE1
#define WEST_LINE_NUM            NUM_LINE2
#define NORTH_LINE_NUM           NUM_LINE3
#define SOUTH_LINE_NUM           NUM_LINE4

#define COLOUR_COL_NUM          (3u)
#define STATE_COL_NUM           (10U)
#define LVL_COL_NUM             (14U)
#define TIME_COL_NUM            (15U)
#define SEC_MSG_COL_NUM         (19U)
 
#define E_GO_FSM        (1)
#define E_GO_RIGHT_FSM  (2)
#define E_WAIT_FSM      (3)
#define W_GO_FSM        (4)
#define W_GO_RIGHT_FSM  (5)
#define W_WAIT_FSM      (6)
#define N_GO_FSM        (7)
#define N_GO_RIGHT_FSM  (8)
#define N_WAIT_FSM      (9)
#define S_GO_FSM        (10)
#define S_GO_RIGHT_FSM  (11)
#define S_WAIT_FSM      (12)
/* all time are for level 0 */
#define E_GO_TIME         (10)
#define E_WAIT_TIME       (5)  
#define W_GO_TIME         (10)
#define W_WAIT_TIME       (5) 
#define N_GO_TIME         (10)
#define N_WAIT_TIME       (5)  
#define S_GO_TIME         (10)
#define S_WAIT_TIME       (5)  

#define IR_LEVEL1_GO_TIME_ADD  (5u)
#define IR_LEVEL2_GO_TIME_ADD  (5u)  
      


void delay_time(unsigned int);
void LCD_Pulse();
void Write_LCD_Command (const unsigned int);
void Write_LCD_Data(const char);
void Data_Str_Disp_LCD(const char * );
void Data_2Digit_Num_Disp(const unsigned int);
void Data_3Digit_Num_Disp_LCD(const unsigned int data_int);
void LCD_Init();
void Traffic_Init(const unsigned int);
void Goto_XY_LCD_Disp(const unsigned int line, const unsigned int col);
void LCD_Const_Disp();
void Prescale_Timer1_Calc();     
void Timer1_Tick();
void Run_Timer1();
void Traffic_Fsm();
void Traffic_Init(unsigned int);
void E_Go_Fsm_Proc();
void E_Go_Right_Fsm_Proc();
void E_Wait_Fsm_Proc();
void W_Go_Fsm_Proc();
void W_Go_Right_Fsm_Proc();
void W_Wait_Fsm_Proc();
void N_Go_Fsm_Proc();
void N_Go_Right_Fsm_Proc();
void N_Wait_Fsm_Proc();
void S_Go_Fsm_Proc();
void S_Go_Right_Fsm_Proc();
void S_Wait_Fsm_Proc();
void Startup_E_Go_Fsm_Proc();
void Startup_E_Wait_Fsm_Proc();
void Startup_W_Go_Fsm_Proc();
void Startup_W_Wait_Fsm_Proc();
void Startup_N_Go_Fsm_Proc();
void Startup_N_Wait_Fsm_Proc();
void Startup_S_Go_Fsm_Proc();
void Startup_S_Wait_Fsm_Proc();
unsigned int prescale_timer1 = 0x01, prescale_shift= 0,each_fsmstate_secs = 0;
unsigned int timer1_init = 0;
unsigned long int num_calls = 0;
unsigned int cur_disp_lcd_loc = BEGIN_LOC_LINE1;

unsigned int traffic_fsm_state =E_GO_FSM;
const char led_on_disp[] = " ON", led_off_disp[] = "OFF", \
secs_disp[]={"Se"},east_disp[]= "E",west_disp[]= "W",north_disp[]= "N",\
south_disp[]= "S", red_disp[]= {"RED     "}, green_disp[]={"GREEN   "}, green_right_disp[]="GREEN->", yellow_disp[] = "YELLOW  ", \
level0_traffic_disp[] = "N", level1_traffic_disp[] = "F",level2_traffic_disp[] = "S", error_traffic_disp[] = "E", blank_traffic_disp[] = " ";
unsigned int cur_e_red_left_secs = 0, cur_e_yellow_left_secs = 0, cur_e_green_left_secs = 0, cur_e_green_right_secs_left = 0;
unsigned int cur_w_red_left_secs = 0, cur_w_yellow_left_secs = 0, cur_w_green_left_secs = 0, cur_w_green_right_secs_left = 0;
unsigned int cur_n_red_left_secs = 0, cur_n_yellow_left_secs = 0, cur_n_green_left_secs = 0, cur_n_green_right_secs_left = 0;
unsigned int cur_s_red_left_secs = 0, cur_s_yellow_left_secs = 0, cur_s_green_left_secs = 0, cur_s_green_right_secs_left = 0;
unsigned int e_green_duration = E_GO_TIME, w_green_duration = W_GO_TIME, n_green_duration = N_GO_TIME, s_green_duration = S_GO_TIME;  
void main()
{
   TRISA = 0x00;
   PORTA = 0x00;
   TRISB = 0x00;
   PORTB = 0x00;
   TRISC = 0x00;
   PORTC = 0x00;
   TRISD = 0x00;
   PORTD = 0x00;
   TRISE = 0x00;
   PORTE = 0x00;  
   ANSEL =0X00;
   ANSELH=0X00;
     
   LCD_Init();
   LCD_Const_Disp();
   Traffic_Init(S_WAIT_FSM);   
   Run_Timer1();   
   for(;;)
   {
     Timer1_Tick();     
  } 
}
/* Every 1 second Traffic_Fsm() is called*/                                                                                                                                                                                                                                                                                                                      
void Traffic_Fsm()
{    
      --cur_e_red_left_secs; 
      --cur_e_yellow_left_secs;
      --cur_e_green_left_secs;
      

      --cur_w_red_left_secs;
      --cur_w_yellow_left_secs;
      --cur_w_green_left_secs;
     

      --cur_n_red_left_secs;
      --cur_n_yellow_left_secs;
	  --cur_n_green_left_secs;
     

      --cur_s_red_left_secs;
      --cur_s_yellow_left_secs;
      --cur_s_green_left_secs;
                         
     switch(traffic_fsm_state)
    {
      case E_GO_FSM:
        ++each_fsmstate_secs;     
       
        if(each_fsmstate_secs >= e_green_duration )
        {
          each_fsmstate_secs= 0;
          traffic_fsm_state = E_WAIT_FSM;        
          Startup_E_Wait_Fsm_Proc(); 
          break;              
        }          
        E_Go_Fsm_Proc();
      break;
      case E_WAIT_FSM:
        ++each_fsmstate_secs;
                             
        if(each_fsmstate_secs >= E_WAIT_TIME )
        {
          each_fsmstate_secs = 0;
		  if(SW_W_IR_LEVEL1 == SW_PRESSED_ON && SW_W_IR_LEVEL2 == SW_PRESSED_ON)
		  {
			  w_green_duration = W_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(WEST_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level2_traffic_disp);
		  }
          else if(SW_W_IR_LEVEL1 ==SW_PRESSED_ON && SW_W_IR_LEVEL2 == SW_NOT_PRESSED)
		  {	  
             w_green_duration = W_GO_TIME + IR_LEVEL1_GO_TIME_ADD;
			 Goto_XY_LCD_Disp(WEST_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level1_traffic_disp);
		  }	 
          else if(SW_W_IR_LEVEL1 == SW_NOT_PRESSED && SW_W_IR_LEVEL2 == SW_NOT_PRESSED)	
		  {	  
             w_green_duration = W_GO_TIME;
			 Goto_XY_LCD_Disp(WEST_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level0_traffic_disp);
		  }	 
          else
		  {
			  /* error: IR level 1 not pressed and IR level 2 pressed on*/
			  w_green_duration = W_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(WEST_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(error_traffic_disp);
		  } 			  
          traffic_fsm_state = W_GO_FSM;
          Startup_W_Go_Fsm_Proc();		  
          break;               
        }
        E_Wait_Fsm_Proc(); 
      break;
    case W_GO_FSM:
      ++each_fsmstate_secs; 
        
      if(each_fsmstate_secs >= w_green_duration )
      {
        each_fsmstate_secs= 0;
        traffic_fsm_state = W_WAIT_FSM;        
        Startup_W_Wait_Fsm_Proc(); 
        break;              
      }          
      W_Go_Fsm_Proc();
      break;
      case W_WAIT_FSM:
      ++each_fsmstate_secs;
                                   
      if(each_fsmstate_secs >= W_WAIT_TIME )
      {
        each_fsmstate_secs = 0;
         if(SW_N_IR_LEVEL1 == SW_PRESSED_ON && SW_N_IR_LEVEL2 == SW_PRESSED_ON)
		  {
			  n_green_duration = N_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(NORTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level2_traffic_disp);
		  }
          else if(SW_N_IR_LEVEL1 ==SW_PRESSED_ON && SW_N_IR_LEVEL2 == SW_NOT_PRESSED)
		  {	  
             n_green_duration = N_GO_TIME + IR_LEVEL1_GO_TIME_ADD;
			 Goto_XY_LCD_Disp(NORTH_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level1_traffic_disp); 
		  }	 
          else if(SW_N_IR_LEVEL1 == SW_NOT_PRESSED && SW_N_IR_LEVEL2 == SW_NOT_PRESSED)	
		  {	  
             n_green_duration = N_GO_TIME;
			 Goto_XY_LCD_Disp(NORTH_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level0_traffic_disp); 
		  }
          else
		  {
			  /* error: IR level 1 not pressed and IR level 2 pressed on*/
			  n_green_duration = N_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(NORTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(error_traffic_disp);
		  } 				
        traffic_fsm_state =N_GO_FSM;              
        Startup_N_Go_Fsm_Proc(); 
        break;               
      }
      W_Wait_Fsm_Proc(); 
      break;
   case N_GO_FSM:
      ++each_fsmstate_secs;   
      
      if(each_fsmstate_secs >= n_green_duration )
      {
        each_fsmstate_secs= 0;
        traffic_fsm_state = N_WAIT_FSM;
        Startup_N_Wait_Fsm_Proc(); 
        break;              
      }          
      N_Go_Fsm_Proc();
      break;
      case N_WAIT_FSM:
      ++each_fsmstate_secs;
                  
      if(each_fsmstate_secs >= N_WAIT_TIME )
      {
        each_fsmstate_secs = 0;
		if(SW_S_IR_LEVEL1 == SW_PRESSED_ON && SW_S_IR_LEVEL2 == SW_PRESSED_ON)
		{
			  s_green_duration = S_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level2_traffic_disp);
		}
        else if(SW_S_IR_LEVEL1 ==SW_PRESSED_ON && SW_S_IR_LEVEL2 == SW_NOT_PRESSED)
		{	
             s_green_duration = S_GO_TIME + IR_LEVEL1_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level1_traffic_disp);
		}
        else if(SW_S_IR_LEVEL1 == SW_NOT_PRESSED && SW_S_IR_LEVEL2 == SW_NOT_PRESSED)	
		{	
             s_green_duration = S_GO_TIME;
			 Goto_XY_LCD_Disp(SOUTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level0_traffic_disp);
		}	 
        else
		{
			  /* error: IR level 1 not pressed and IR level 2 pressed on*/
			  s_green_duration = S_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(error_traffic_disp);
		} 		
        traffic_fsm_state =S_GO_FSM;
        Startup_S_Go_Fsm_Proc(); 
        break;               
      }
      N_Wait_Fsm_Proc(); 
      break;
   case S_GO_FSM:
      ++each_fsmstate_secs;   
        
      if(each_fsmstate_secs >= s_green_duration )
      {
        each_fsmstate_secs= 0;
        traffic_fsm_state = S_WAIT_FSM;
        Startup_S_Wait_Fsm_Proc(); 
        break;              
      }          
      S_Go_Fsm_Proc();
      break;
     case S_WAIT_FSM:
      ++each_fsmstate_secs;      
                            
      if(each_fsmstate_secs >= S_WAIT_TIME )
      {
        each_fsmstate_secs = 0;
        if(SW_E_IR_LEVEL1 == SW_PRESSED_ON && SW_E_IR_LEVEL2 == SW_PRESSED_ON)
		{
			  e_green_duration = E_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(EAST_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(level2_traffic_disp);
		}
        else if(SW_E_IR_LEVEL1 ==SW_PRESSED_ON && SW_E_IR_LEVEL2 == SW_NOT_PRESSED)
		{		
             e_green_duration = E_GO_TIME + IR_LEVEL1_GO_TIME_ADD;
			 Goto_XY_LCD_Disp(EAST_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level1_traffic_disp);
	    }		 
        else if(SW_E_IR_LEVEL1 == SW_NOT_PRESSED && SW_E_IR_LEVEL2 == SW_NOT_PRESSED)
        {			
             e_green_duration = E_GO_TIME;
			 Goto_XY_LCD_Disp(EAST_LINE_NUM, LVL_COL_NUM);
			 Data_Str_Disp_LCD(level0_traffic_disp);
	    }		  
        else
		{
			  /* error: IR level 1 not pressed and IR level 2 pressed on*/
			  e_green_duration = E_GO_TIME + IR_LEVEL1_GO_TIME_ADD + IR_LEVEL2_GO_TIME_ADD;
			  Goto_XY_LCD_Disp(EAST_LINE_NUM, LVL_COL_NUM);
			  Data_Str_Disp_LCD(error_traffic_disp);
		} 			
        traffic_fsm_state =E_GO_FSM;
        Startup_E_Go_Fsm_Proc(); 
        break;               
      }
      S_Wait_Fsm_Proc(); 
      break;    
    }    
}
void Traffic_Init(unsigned int traffic_fsm_init)
{
     each_fsmstate_secs = 0;
     Write_LCD_Command(0x01);
     LCD_Const_Disp();     
       
   traffic_fsm_state =traffic_fsm_init;  
   switch(traffic_fsm_state)
   {
    case E_GO_FSM: 
	   Startup_E_Go_Fsm_Proc();   
      
      break;
   case E_WAIT_FSM:
       Startup_E_Wait_Fsm_Proc();   
      
      break; 
    case W_GO_FSM:
	  Startup_W_Go_Fsm_Proc();
	  
      break;
    case W_WAIT_FSM: 
	  Startup_W_Wait_Fsm_Proc();
	  
      break;
    case N_GO_FSM:
      Startup_N_Go_Fsm_Proc();
	  
      break;
   case N_WAIT_FSM:
      Startup_N_Wait_Fsm_Proc();
      
      break;
    case S_GO_FSM:
	  Startup_S_Go_Fsm_Proc();
	  
      break;
   case S_WAIT_FSM:  
      Startup_S_Wait_Fsm_Proc();     
      break;     
   }   
}
void Startup_E_Go_Fsm_Proc()
{
      cur_e_red_left_secs = e_green_duration + E_WAIT_TIME;            //OFF
	  cur_e_yellow_left_secs = e_green_duration;                       //OFF
      cur_e_green_left_secs = e_green_duration;                        //ON	
	  cur_w_red_left_secs = e_green_duration + E_WAIT_TIME;            //ON
	  cur_w_yellow_left_secs = e_green_duration + E_WAIT_TIME + w_green_duration;  //OFF
      cur_w_green_left_secs = e_green_duration + E_WAIT_TIME;          //OFF 
      cur_n_red_left_secs =  e_green_duration + E_WAIT_TIME +  w_green_duration + W_WAIT_TIME ;   //ON
	  cur_n_yellow_left_secs = e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration;  //OFF
      cur_n_green_left_secs = e_green_duration + E_WAIT_TIME +  w_green_duration + W_WAIT_TIME ;   //OFF;
      cur_s_red_left_secs = e_green_duration + E_WAIT_TIME +  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;        //ON 
      cur_s_yellow_left_secs = e_green_duration + E_WAIT_TIME +  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration ;               //OFF
      cur_s_green_left_secs = e_green_duration + E_WAIT_TIME +  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;       //OFF  

     Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(green_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);  
	 
     E_Go_Fsm_Proc();		
}
void Startup_E_Wait_Fsm_Proc()
{
      cur_e_red_left_secs = E_WAIT_TIME;                                  //OFF
	  cur_e_yellow_left_secs = E_WAIT_TIME;                              //ON
      cur_e_green_left_secs =   E_WAIT_TIME +  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;            //OFF	     
	  cur_w_red_left_secs =  E_WAIT_TIME;            //ON
	  cur_w_yellow_left_secs = E_WAIT_TIME + w_green_duration;  //OFF
      cur_w_green_left_secs = E_WAIT_TIME;          //OFF 
      cur_n_red_left_secs = E_WAIT_TIME +  w_green_duration + W_WAIT_TIME ;   //ON
	  cur_n_yellow_left_secs = E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration;  //OFF
      cur_n_green_left_secs = E_WAIT_TIME +  w_green_duration + W_WAIT_TIME ;   //OFF;
      cur_s_red_left_secs =  E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;    //ON 
      cur_s_yellow_left_secs = E_WAIT_TIME +  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration ;                 //oFF	  
      cur_s_green_left_secs =  E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;  //OFF  
	 
	 Goto_XY_LCD_Disp(EAST_LINE_NUM, LVL_COL_NUM);
	 Data_Str_Disp_LCD(blank_traffic_disp);
	 Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(yellow_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
      E_Wait_Fsm_Proc();	
}

void Startup_W_Go_Fsm_Proc()
{
      cur_e_red_left_secs =  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration;         //OFF
	  cur_e_green_left_secs =  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;          //OFF
	  cur_w_red_left_secs = w_green_duration + W_WAIT_TIME;                                  //OFF
	  cur_w_yellow_left_secs = w_green_duration;                 //OFF
      cur_w_green_left_secs = w_green_duration;                                              //ON 
      cur_n_red_left_secs =   w_green_duration + W_WAIT_TIME ;                              //ON
	  cur_n_yellow_left_secs = w_green_duration + W_WAIT_TIME + n_green_duration;  //OFF
      cur_n_green_left_secs =  w_green_duration + W_WAIT_TIME ;                             //OFF;
      cur_s_red_left_secs = w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;         //ON 
      cur_s_yellow_left_secs =  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration; //OFF	  
      cur_s_green_left_secs =  w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;       //OFF 

     Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(green_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);	  
      W_Go_Fsm_Proc();	
}	

void Startup_W_Wait_Fsm_Proc()
{
      cur_e_red_left_secs = W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =   W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration;   //OFF
      cur_e_green_left_secs =  W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;          //OFF
	  cur_w_red_left_secs =  W_WAIT_TIME;                                                                //OFF  
	  cur_w_yellow_left_secs = W_WAIT_TIME;                                                             //ON
	  cur_w_green_left_secs =   W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME;                                        //OFF 	  
	  cur_n_red_left_secs =   W_WAIT_TIME ;                              //ON
	  cur_n_yellow_left_secs =  W_WAIT_TIME + n_green_duration;  //OFF
      cur_n_green_left_secs =  W_WAIT_TIME ;                             //OFF;
      cur_s_red_left_secs =  W_WAIT_TIME + n_green_duration + N_WAIT_TIME;         //ON  
	  cur_s_yellow_left_secs =  W_WAIT_TIME + n_green_duration + N_WAIT_TIME + s_green_duration; //OFF
      cur_s_green_left_secs =   W_WAIT_TIME + n_green_duration + N_WAIT_TIME;       //OFF 

	  Goto_XY_LCD_Disp(WEST_LINE_NUM, LVL_COL_NUM);
	 Data_Str_Disp_LCD(blank_traffic_disp);  
     Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(yellow_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     	  
      W_Wait_Fsm_Proc(); 	
}
void Startup_N_Go_Fsm_Proc()
{
      cur_e_red_left_secs =  n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =  n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration;        //OFF
      cur_e_green_left_secs =   n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME;          //OFF
	  cur_w_red_left_secs =    n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME;             //ON  
	  cur_w_yellow_left_secs =   n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration;   //OFF	   
	  cur_w_green_left_secs =   n_green_duration + N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration;    //OFF 	  
	  cur_n_red_left_secs =   n_green_duration + N_WAIT_TIME ;                              //OFF
	  cur_n_yellow_left_secs =   n_green_duration;                                        //OFF
      cur_n_green_left_secs =  n_green_duration ;                                           //ON;
      cur_s_red_left_secs =   n_green_duration + N_WAIT_TIME;                               //ON 
	  cur_s_yellow_left_secs =   n_green_duration + N_WAIT_TIME + s_green_duration;                //OFF
      cur_s_green_left_secs =    n_green_duration + N_WAIT_TIME;                            //OFF  
	  
	 Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(green_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
      N_Go_Fsm_Proc();	
}
void Startup_N_Wait_Fsm_Proc()
{
	  cur_e_red_left_secs =  N_WAIT_TIME + s_green_duration + S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =   N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration;         //OFF
      cur_e_green_left_secs =   N_WAIT_TIME + s_green_duration + S_WAIT_TIME;          //OFF	
      cur_w_red_left_secs =     N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME;           //ON
	  cur_w_yellow_left_secs = N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration;                           //OFF	 
	  cur_w_green_left_secs = N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME  ;                            //OFF       
      cur_n_red_left_secs =   N_WAIT_TIME ;                              //OFF
	  cur_n_yellow_left_secs =   N_WAIT_TIME;                                        //ON
      cur_n_green_left_secs =  N_WAIT_TIME + s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME;            //OFF;
      cur_s_red_left_secs =    N_WAIT_TIME;                               //ON
	  cur_s_yellow_left_secs =   N_WAIT_TIME + s_green_duration;                //OFF
      cur_s_green_left_secs =   N_WAIT_TIME;                            //OFF  
	  
	  Goto_XY_LCD_Disp(NORTH_LINE_NUM, LVL_COL_NUM);
	 Data_Str_Disp_LCD(blank_traffic_disp); 
	 Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(yellow_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
	  
	  
      N_Wait_Fsm_Proc();
}	
void Startup_S_Go_Fsm_Proc()
{
      cur_e_red_left_secs =  s_green_duration + S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =   s_green_duration + S_WAIT_TIME + e_green_duration;         //OFF
      cur_e_green_left_secs =   s_green_duration + S_WAIT_TIME;          //OFF	
      cur_w_red_left_secs =      s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME;           //ON 
      cur_w_yellow_left_secs =   s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration ;   //OFF	 	  
	  cur_w_green_left_secs = s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME ;   //OFF       
      cur_n_red_left_secs =  s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME  ;                 //ON
	  cur_n_yellow_left_secs =  s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration ;                             //OFF
      cur_n_green_left_secs =   s_green_duration + S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME ;           //OFF;
      cur_s_red_left_secs =    s_green_duration + S_WAIT_TIME;                                   //OFF  
      cur_s_yellow_left_secs =    s_green_duration;                //OFF	  
      cur_s_green_left_secs =   s_green_duration ;                                               //ON

	  
	 Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(green_disp);
      S_Go_Fsm_Proc();	
}
void Startup_S_Wait_Fsm_Proc()
{
      cur_e_red_left_secs =   S_WAIT_TIME;            //ON
	  cur_e_yellow_left_secs =   S_WAIT_TIME + e_green_duration;         //OFF	  
      cur_e_green_left_secs =   S_WAIT_TIME;          //OFF	
      cur_w_red_left_secs =     S_WAIT_TIME + e_green_duration + E_WAIT_TIME;           //ON  
	  cur_w_yellow_left_secs = S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME ;    //OFF	  
	  cur_w_green_left_secs =   S_WAIT_TIME + e_green_duration + E_WAIT_TIME; //OFF       
      cur_n_red_left_secs =   S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME;    //ON
	  cur_n_yellow_left_secs = S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration ;              //OFF  	  
      cur_n_green_left_secs =  S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration;           //OFF;
      cur_s_red_left_secs =     S_WAIT_TIME;                                   //OFF  
	  cur_s_yellow_left_secs =   S_WAIT_TIME ;                //ON	
      cur_s_green_left_secs =  S_WAIT_TIME + e_green_duration + E_WAIT_TIME + w_green_duration + W_WAIT_TIME + n_green_duration + N_WAIT_TIME;    //OFF 
     
	  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, LVL_COL_NUM);
	 Data_Str_Disp_LCD(blank_traffic_disp);
	 
     Goto_XY_LCD_Disp(EAST_LINE_NUM, COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(WEST_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(NORTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(red_disp);
     Goto_XY_LCD_Disp(SOUTH_LINE_NUM,COLOUR_COL_NUM);
     Data_Str_Disp_LCD(yellow_disp);
	 
      S_Wait_Fsm_Proc();	
}
void E_Go_Fsm_Proc()
{
   W_E_PORT = 0x14;
   S_N_PORT = 0x11;

   
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_green_left_secs);
   
   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
  
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs);   
}
void E_Wait_Fsm_Proc()
{
   W_E_PORT = 0x12;
   S_N_PORT = 0x11;
  
   
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_yellow_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
  
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs);   
             
}
void W_Go_Fsm_Proc()
{
   W_E_PORT = 0x41;
   S_N_PORT = 0x11;
  
 
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_green_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs);  
  
}
void W_Wait_Fsm_Proc()
{
   W_E_PORT = 0x21;
   S_N_PORT = 0x11;
   
   
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_yellow_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
 
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs);  
}
void N_Go_Fsm_Proc()
{
   W_E_PORT = 0x11;
   S_N_PORT = 0x14;
 
  
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_green_left_secs);  
 
 
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs); 
   
}

void N_Wait_Fsm_Proc()
{
   W_E_PORT = 0x11;
   S_N_PORT = 0x12; 
   
   
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_yellow_left_secs);  
 
   
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_red_left_secs); 
   
           
}
void S_Go_Fsm_Proc()
{
   W_E_PORT = 0x11;
   S_N_PORT = 0x41;
   
   
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
   
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_green_left_secs); 
}

void S_Wait_Fsm_Proc()
{
   W_E_PORT = 0x11;
   S_N_PORT = 0x21;
 
  
   Goto_XY_LCD_Disp(EAST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_e_red_left_secs);

   
   Goto_XY_LCD_Disp(WEST_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_w_red_left_secs);    
   
   
   Goto_XY_LCD_Disp(NORTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_n_red_left_secs);  
 
   
   Goto_XY_LCD_Disp(SOUTH_LINE_NUM, TIME_COL_NUM);
   Data_3Digit_Num_Disp_LCD(cur_s_yellow_left_secs);
   
}
void LCD_Const_Disp()
{
  Goto_XY_LCD_Disp(EAST_LINE_NUM, NUM_COL1);
  Data_Str_Disp_LCD(east_disp);
  Goto_XY_LCD_Disp(EAST_LINE_NUM, STATE_COL_NUM);
  Data_Str_Disp_LCD(led_on_disp);
  Goto_XY_LCD_Disp(EAST_LINE_NUM, SEC_MSG_COL_NUM);
  Data_Str_Disp_LCD(secs_disp);
  
  Goto_XY_LCD_Disp(WEST_LINE_NUM, NUM_COL1);
  Data_Str_Disp_LCD(west_disp);
  Goto_XY_LCD_Disp(WEST_LINE_NUM, STATE_COL_NUM);
  Data_Str_Disp_LCD(led_on_disp);
  Goto_XY_LCD_Disp(WEST_LINE_NUM, SEC_MSG_COL_NUM);
  Data_Str_Disp_LCD(secs_disp);
  
  Goto_XY_LCD_Disp(NORTH_LINE_NUM, NUM_COL1);
  Data_Str_Disp_LCD(north_disp);
  Goto_XY_LCD_Disp(NORTH_LINE_NUM, STATE_COL_NUM);
  Data_Str_Disp_LCD(led_on_disp);
  Goto_XY_LCD_Disp(NORTH_LINE_NUM, SEC_MSG_COL_NUM);
  Data_Str_Disp_LCD(secs_disp);
  
  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, NUM_COL1);
  Data_Str_Disp_LCD(south_disp);
  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, STATE_COL_NUM);
  Data_Str_Disp_LCD(led_on_disp);
  Goto_XY_LCD_Disp(SOUTH_LINE_NUM, SEC_MSG_COL_NUM);
  Data_Str_Disp_LCD(secs_disp);  
}
void Run_Timer1()
{
/*internal timer1 clock  with 1:1 prescale,Timer1 counts with gate control disabled */
   T1CON =0x85;   
   prescale_timer1 =  0x01;
   prescale_shift= 0;
   Prescale_Timer1_Calc();
   timer1_init = 65536ul - (INC/prescale_timer1); 
   TMR1H = timer1_init / 256ul;
   TMR1L = timer1_init % 256ul;
}  
void Timer1_Tick()
{
     while(TMR1IF == 0);
     TMR1IF = 0;
     timer1_init = 65536ul - (INC/prescale_timer1); 
     TMR1H = timer1_init / 256ul;
     TMR1L = timer1_init % 256ul; 
     if(++num_calls >= UPDATE_TIME)
     {
        Traffic_Fsm();
        num_calls = 0;        
     }       
} 
void Prescale_Timer1_Calc()
{
   if(T1CKPS0 == 1)
   {
      prescale_shift |= 0x01;           
   }
   if(T1CKPS1 == 1)
   {
     prescale_shift |= 0x02;
   }  
   prescale_timer1 = prescale_timer1  << prescale_shift;                                                      
}
void LCD_Init()
{
    Write_LCD_Command(0x30);
    Write_LCD_Command(0x30);
    Write_LCD_Command(0x30);
    Write_LCD_Command(0x38);  
	Write_LCD_Command(0x01);
    Write_LCD_Command(0x0C);
    Write_LCD_Command(0x06);                                       
}     

void LCD_Pulse()
{
        EN_PIN = 1;
        delay_time(100);
        EN_PIN = 0;
        delay_time(100);
}
  void Write_LCD_Command (const unsigned int cmd)
 {
         RW_PIN =0;
         RS_PIN = 0; 
         LCD_PORT = cmd;
         LCD_Pulse();
 }
 void Write_LCD_Data(const char ch)
{
     RW_PIN = 0;
     RS_PIN = 1;
     LCD_PORT =ch;
     LCD_Pulse();
} 
void Data_Str_Disp_LCD(const char *char_ptr)
{ 
       while(*char_ptr)
       {
           Write_LCD_Data(*(char_ptr++));
       }
}
void Data_3Digit_Num_Disp_LCD(const unsigned int data_int)
{
    unsigned int hundreds_digit,tens_digit, unit_digit,num;
    char num_data[] ={'0','1','2','3','4','5','6','7','8','9'};
	num = data_int % 1000;
	hundreds_digit = (unsigned int) (num / (unsigned long) (100));
	Write_LCD_Data(num_data[hundreds_digit]);
	num = data_int % 100;
    tens_digit = (unsigned int) (num / 10);
    Write_LCD_Data(num_data[tens_digit]);
    unit_digit = (unsigned int) (data_int % 10);
    Write_LCD_Data(num_data[unit_digit]);                                 
}
void Data_2Digit_Num_Disp(const unsigned int data_int)
{
    unsigned int tens_digit, unit_digit,num;
    char num_data[] ={'0','1','2','3','4','5','6','7','8','9'}; 
    num = data_int % 100;	
    unit_digit = data_int % 10;
    tens_digit = num / 10;
    Write_LCD_Data(num_data[tens_digit]);
   // Write_LCD_Data(tens_digit + 30); // num in ascii code   
    Write_LCD_Data(num_data[unit_digit]); 
   // Write_LCD_Data(unit_digit + 30); // num in ascii code                                 
}
void delay_time(unsigned int time_delay)
{
    while(time_delay--);
} 
void Goto_XY_LCD_Disp(const unsigned int line_num, const unsigned int col_num)
{
	unsigned int line_lcd = line_num -1, col_lcd = col_num - 1;
   if(line_lcd < 4 &&  col_lcd < 20 )
   {
     cur_disp_lcd_loc = BEGIN_LOC_LINE1;
    if( line_lcd& 0x01)
       cur_disp_lcd_loc = cur_disp_lcd_loc | (1 << 6); 
    if( line_lcd& 0x02)
    {
      cur_disp_lcd_loc = cur_disp_lcd_loc | (1 << 2); 
      cur_disp_lcd_loc = cur_disp_lcd_loc | (1 << 4);     
    }    
    cur_disp_lcd_loc = cur_disp_lcd_loc + col_lcd;
    Write_LCD_Command(cur_disp_lcd_loc);
   }
}
