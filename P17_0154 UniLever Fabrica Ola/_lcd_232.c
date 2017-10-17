/*                     -------------------   Firmware BOmbeiros Pa�os Ferreira --------------------------------
                       
*/

#include <16f883.h>
#fuses HS,WDT,PROTECT,NOLVP,NODEBUG,NOMCLR,NOBROWNOUT
#use delay(clock=10000000)  
#use fast_io(a)
#use fast_io(b)
#use fast_io(c)

//1er endere�o MSB, segundo LSB


#use RS232(baud=9600,xmit=PIN_C6,rcv=PIN_C7,parity=N,errors,stream=PORTA,RESTART_WDT)
//#rom int8 0x2100={1,2,0,5,5,10,0,0,0,0,0}
//Defines s

#define IN1    PIN_C5      //IN1 -> Espira E1   
#define IN2    PIN_C4      //IN2 -> Espira E2 
#define IN3    PIN_C3      //IN3 -> Botoneira Chamada A�ucar    
#define IN4    PIN_C2      //IN4 -> Sinal R�dio
#define IN5    PIN_C1      //IN5 -> 
#define IN6    PIN_C0      //IN6 ->   
// PIN_A4 -> OUT 9 C3


int8 data_receive=0;

Unsigned int8 Data_Buffer[9];    //
int8 current_data=0;
int1 BufferOK=0;
int1 Carriage_Return=1;
int8 count_TimerA=0,count_TimerB=0;
int8  TIMER_LC=180;                  //Timer Linha Comandos 60segundos
int16 TIMER_A=0;
int16 TIMER_B=0;                     //Timer limpeza G2 -> G1 
int16 TIMER_C=0;                     //Timer limpeza G2 -> G1 

int16 TimerI1on=0,TimerI1off=0;    //Timer 5ms
int16 TimerI2on=0,TimerI2off=0;    //Timer 5ms
int16 TimerI3on=0,TimerI3off=0;    //Timer 5ms
int16 TimerI4on=0,TimerI4off=0;    //Timer 5ms

int8 auxI1on=0,auxI1off=0;
int8 auxI2on=0,auxI2off=0;
int8 auxI3on=0,auxI3off=0;
int8 auxI4on=0,auxI4off=0;


int1 ING_I1=0,ING_I2=0,ING_I3=0,ING_I4=0;
int1 MEM_I1=0,MEM_I2=0,MEM_I3=0,MEM_I4=0;
int1 SIGNAL_BLINK=0;

int16 Tempos[15];

int8 EstadoG1=0;
int8 EstadoG2=0;
int8 EstadoG3=0;
int8 EstadoG4=0;

int8 saida=0x00;              
int1 LinhaComando=0;        //0-> OFF, modo r�dio activo, 1-> ON com linha de comandos

int16 TimeOut=255;
int16 TimeOutTimer=255;
Unsigned int8 StartByte='#';

int8 stepR1=0,stepR2=0;
int8 auxTimeR1=0,auxTimeR2=0;

int1 debugInputs=0;

int1 ChamadaAcucar=0;
int1 LimpezaAcucar=0;

#INT_RDA
void trata_INT_RDA()
{
  //output_toggle(PIN_B7);
  //output_toggle(PIN_A4);
  data_receive=fgetc(PORTA);
  if(data_receive==StartByte && Carriage_Return)          //
    {
     //TimeOut=0;
     //count_TimerA=0;
     current_data=0;
     Data_Buffer[current_data]=data_receive;
     current_data++;
     Carriage_Return=0;
     BufferOK=0; 
    }
  else  
    {
     if(!Carriage_Return)
       { 
      if(current_data==8 || data_receive==13)           //  || Timeout
        {Data_Buffer[current_data]=data_receive;
         Carriage_Return=1;
         current_data=0;
         bufferOK=1;
         if(data_receive!=13)     //erro
            bufferOK=0; 
        }
      else
        {Data_Buffer[current_data]=data_receive;
        current_data++;         
        }
    }
   }  
}



#INT_TIMER1
void trata_t1()         //Passa c� a cada 50ms
{
count_TimerA++;
count_TimerB++;

TimerI1on++;    //Timer 5ms
TimerI1off++;   //Timer 5ms
TimerI2on++;    //Timer 5ms
TimerI2off++;   //Timer 5ms
TimerI3on++;    //Timer 5ms
TimerI3off++;   //Timer 5ms
TimerI4on++;    //Timer 5ms
TimerI4off++;   //Timer 5ms

RESTART_WDT();
if(count_timerA<=10)  //0,5seg
   SIGNAL_BLINK=1;
else
  {
   SIGNAL_BLINK=0;
   if(count_timerA>=10)   //1 segundo
     {
      count_timerA=0;
      if(TIMER_A<255)
         TIMER_A++;
     }
   if(count_timerB>=10)   //1 segundo
     {
      count_timerB=0;
      if(TIMER_B<255)
         TIMER_B++;
     }
      
  }
 
set_timer1(3036);
//SET_TIMER1(34286);
}



void ImprimeMSGRS232(int1 msg) //0:Erro, 1:OK
{

if(!msg)
  {fputc('e',PORTA); //ERRO
   fputc('r',PORTA); //
   fputc('r',PORTA); //
   fputc('o',PORTA); //
  }
else
  {
  fputc('o',PORTA); 
  fputc('k',PORTA);
  }
fputc(13,PORTA);
fputc(10,PORTA);
}



void trata_msgAtoI(int8 i_ee,int8 i_temp,int16 max)   //m�x 199
{
int16 aux=0;
int16 MSB=0,LSB=0;
int1 valid_data=0;

if((Data_Buffer[2]>='0' && Data_Buffer[2]<='9') && (Data_Buffer[3]>='0' && Data_Buffer[3]<='9') && (Data_Buffer[4]>='0' && Data_Buffer[4]<='9') && (Data_Buffer[5]==13))   //s�o 3 digitos
  {
   MSB=(int16) Data_Buffer[2]-48;
   MSB=MSB*100;
   LSB=(int16) Data_Buffer[3]-48;
   LSB=LSB*10;
   MSB=MSB+LSB;
   LSB=(int16) Data_Buffer[4]-48;
   aux=MSB+LSB;
   valid_data=1;
  }
else
   if((Data_Buffer[2]>='0' && Data_Buffer[2]<='9') && (Data_Buffer[3]>='0' && Data_Buffer[3]<='9') && (Data_Buffer[4]==13))   //s�o dois digitos
     {
      MSB=(int16) Data_Buffer[2]-48;
      MSB=MSB*10;
      LSB=(int16) Data_Buffer[3]-48;
      aux=MSB+LSB;
      valid_data=1;
     }
   else
      if((Data_Buffer[2]>='0' && Data_Buffer[2]<='9') && (Data_Buffer[3]==13))   //� 1 d�gito
        {
         LSB=(int16) Data_Buffer[2]-48;
         aux=LSB;
         valid_data=1;
        }


if(valid_data && aux<=max)
{
  Tempos[i_temp]=aux;
  write_eeprom (i_ee,make8(aux,1));         // Escreve o MSB de 16bits
  delay_ms(10);
  write_eeprom (i_ee+1,make8(aux,0));       // Escreve o LSB de 16bits      
  ImprimeMSGRS232(1);          //OK
}
else
  ImprimeMSGRS232(0);          //ERRO



}




void trata_buffer(){
int8 i=0,kx=0;
char ch='0';
int16 data_example=0;
//delay_ms(10);

switch(Data_Buffer[1]){
      case '#':if(Data_Buffer[2]=='#' && Data_Buffer[3]==13)
                 {
                  if(!LinhaComando)
                    {LinhaComando=1;
                     fprintf(PORTA,"Modo Linha de Comandos Activado\n\r");
                     TIMER_LC=0;
                     
                    }
                  else
                    {
                     LinhaComando=0;
                     fprintf(PORTA,"Modo Linha de Comandos Desactivado\n\r");
                    }            
                 }
               break;    
    
      case 'A':if(LinhaComando)
                  trata_msgAtoI(0,3,100);       //TLimpeza Entrada de Cami�o -> Tempos[3]                   
               break;
      
      case 'B':if(LinhaComando)
                  trata_msgAtoI(2,7,100);      //TLimpeza Sa�da de Cami�o -> Tempos[7]
      
      case 'C':if(LinhaComando)
                  trata_msgAtoI(4,8,100);      //Tempo Verde M�nimo Empilhadores G3-G2 -> Tempos[8]          
      
      case 'D':if(LinhaComando)
                  trata_msgAtoI(6,12,100);    //Tempo M�nimo Empilhadores G2-G3 -> Tempos[12]  
                  
      case 'E':if(LinhaComando)
                  trata_msgAtoI(8,11,100);    //TLimpeza Empilhadores  Tempos[11]                 
               break;
            
               break;
      case 'Z':if(LinhaComando)
                 if(!debugInputs)
                    {
                     debugInputs=1;
                     fprintf(PORTA,"Modo Debug Inputs Ativado\n\r");                    
                    }
                  else
                    {
                     debugInputs=0;
                     fprintf(PORTA,"Modo Debug Inputs Desativado\n\r");
                    }      
               break;
      case 'R':if(LinhaComando)
                 if(Data_Buffer[2]=='S' && Data_Buffer[3]==13)
                   {
                    ImprimeMSGRS232(1);      //OK
                    delay_ms(2);
                    reset_cpu();
                   }
                 else
                    ImprimeMSGRS232(0);
               break;
      case 'S':if(LinhaComando)
                  if(Data_Buffer[2]=='W' && Data_Buffer[3]==13)
                   {
                   fprintf(PORTA,"Tempo Limpeza Entrada de Cami�o = %Lu \n\r",Tempos[3]);
                   fprintf(PORTA,"Tempo Limpeza Sa�da de Cami�o = %Lu \n\r",Tempos[7]);
                   fprintf(PORTA,"Tempo Verde Maximo Empilhadores = %Lu \n\r",Tempos[9]);                                     
                   fprintf(PORTA,"Tempo Limpeza Empilhadores G3 -> G2 = %Lu \n\r",Tempos[11]);          
                   }
                 else
                    ImprimeMSGRS232(0);
               break;   
               
               
               
                        
           //default:ImprimeMSGRS232(0);
           //        break;      
        }
        
}

void LER_ENTRADAS()
{

//Leitura Espiras Simples
 RESTART_WDT();

 //Entradas digitais
switch(auxI1on){
      case 0:if(input(IN1))
               {
                TimerI1on=0;
                auxI1on=1;
               }
             break;  
      case 1:if(TimerI1on<10)          //50ms
               {if(!input(IN1))
                  {auxI1on=0;          //Se houve um pico ou foi um sinal falso
                   TimerI1on=0;      
                  }            
               }
             else
               {
               if(debugInputs)
                  fprintf(PORTA,"Input 1 -> ON\n\r");
                ING_I1=1;         //Espira de Entrada Ocupada
                MEM_I1=1;
                auxI1on=0;         
               }
             break;
         }
 
switch(auxI1off){
      case 0:if(!input(IN1))
               {
                TimerI1off=0;
                auxI1off=1;
               }
             break;  
      case 1:if(TimerI1off<10)          //50ms
               {if(input(IN1))
                  {auxI1off=0;          //Se houve um pico ou foi um sinal falso
                   TimerI1off=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 1 -> OFF\n\r");
                ING_I1=0;         //Espira de Entrada Ocupada
                auxI1off=0;         
               }
             break;
         }

switch(auxI2on){
      case 0:if(input(IN2))
               {
                TimerI2on=0;
                auxI2on=1;
               }
             break;  
      case 1:if(TimerI2on<10)          //50ms
               {if(!input(IN2))
                  {auxI2on=0;          //Se houve um pico ou foi um sinal falso
                   TimerI2on=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 2-> ON\n\r");
                ING_I2=1;         //Espira de Entrada Ocupada
                MEM_I2=1;     
                auxI2on=0;         
               }
             break;
         }
 
switch(auxI2off){
      case 0:if(!input(IN2))
               {
                TimerI2off=0;
                auxI2off=1;
               }
             break;  
      case 1:if(TimerI2off<10)          //50ms
               {if(input(IN2))
                  {auxI2off=0;          //Se houve um pico ou foi um sinal falso
                   TimerI2off=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 2-> OFF\n\r");
                ING_I2=0;         //Espira de Entrada Ocupada
                auxI2off=0;         
               }
             break;
         }


switch(auxI3on){
      case 0:if(input(IN3))
               {
                TimerI3on=0;
                auxI3on=1;
               }
             break;  
      case 1:if(TimerI3on<10)          //50ms
               {if(!input(IN3))
                  {auxI3on=0;          //Se houve um pico ou foi um sinal falso
                   TimerI3on=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 3-> ON\n\r");
                ING_I3=1;         //Espira de Entrada Ocupada
                MEM_I3=1;
                auxI3on=0;         
               }
             break;
         }
 
switch(auxI3off){
      case 0:if(!input(IN3))
               {
                TimerI3off=0;
                auxI3off=1;
               }
             break;  
      case 1:if(TimerI3off<10)          //50ms
               {if(input(IN3))
                  {auxI3off=0;          //Se houve um pico ou foi um sinal falso
                   TimerI3off=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 3-> OFF\n\r");
                ING_I3=0;         //Espira de Entrada Ocupada              
                auxI3off=0;         
               }
             break;
         }


switch(auxI4on){
      case 0:if(input(IN4))
               {
                TimerI4on=0;
                auxI4on=1;
               }
             break;  
      case 1:if(TimerI4on<10)          //50ms
               {if(!input(IN4))
                  {auxI4on=0;          //Se houve um pico ou foi um sinal falso
                   TimerI4on=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 4-> ON\n\r");
                ING_I4=1;         //Espira de Entrada Ocupada
                MEM_I4=1;   
                auxI4on=0;         
               }
             break;
         }
 
switch(auxI4off){
      case 0:if(!input(IN4))
               {
                TimerI4off=0;
                auxI4off=1;
               }
             break;  
      case 1:if(TimerI4off<10)          //50ms
               {if(input(IN4))
                  {auxI4off=0;          //Se houve um pico ou foi um sinal falso
                   TimerI4off=0;      
                  }            
               }
             else
               {if(debugInputs)
                  fprintf(PORTA,"Input 4-> OFF\n\r");
                ING_I4=0;         //Espira de Entrada Ocupada
                auxI4off=0;
               }
             break;
         }
}

//Tricolor 0: Verde, 1:Amarelo, 2: Vermelho, 3:Blink  , 4: OFF, >=5: Mant�m estado anterior
//BUS: 0: Verde, 2: Vermelho, 3:Blink  , 4: OFF; 5: Verde c/ Blink, 6:Vermelho c/ Blink, 7:Verde c/ Yellow fix, 8: Vermelho c/ Yellow fix ;>10: Mant�m estado anterior 
void EstadoGrupos(int8 G1, int8 G2, int8 G3, int8 G4)
{

switch(G1){
      case 0:bit_clear(saida,0);
             bit_set(saida,1);
             EstadoG1=0;              // Verde
             break;  
      case 1:bit_set(saida,0);
             bit_clear(saida,1);
             EstadoG1=1;             // Vermelho
             break;
          }      

switch(G2){
      case 0:bit_clear(saida,2);
             bit_set(saida,3);
             EstadoG2=0;              // Verde
             break;  
      case 1:bit_set(saida,2);
             bit_clear(saida,3);
             EstadoG2=1;             // Vermelho
             break;
          }

switch(G3){
      case 0:bit_clear(saida,4);
             bit_set(saida,5);
             EstadoG3=0;              // Verde
             break;  
      case 1:bit_set(saida,4);
             bit_clear(saida,5);
             EstadoG3=1;             // Vermelho
             break;
          }

switch(G4){
      case 0:bit_clear(saida,6);
             bit_set(saida,7);
             EstadoG4=0;              // Verde
             break;  
      case 1:bit_set(saida,6);
             bit_clear(saida,7);
             EstadoG4=1;             // Vermelho
             break;
          }

}


//++++++++++++++++++++++++++++++ Controlo Independente Rampa A  ++++++++++++++++++++++++++++++++++++++ 
void ControlLights()
{
  switch(stepR1){
        case 0:if(TIMER_A>=Tempos[0]-Tempos[2]) //Verde m�nimo G1
                  {
                   stepR1=1;
                   auxTimeR1=Tempos[0]-Tempos[2];
                   TIMER_A=0;
                  }
               break;
        case 1:if(TIMER_A>=Tempos[2]) 
                 {
                  auxTimeR1=auxTimeR1+Tempos[2];
                  if((auxTimeR1>Tempos[1]) && (MEM_I3))  //Chamada da Botoneira Acucar
                    {stepR1=2;
                     TIMER_A=0;
                     EstadoGrupos(1,3,3,1); //0: Verde, 1:VErmelho,3:Mant�m        //ALLRED
                     ChamadaAcucar=1;
                    }
                  else
                    {TIMER_A=0;                     
                    }
                 }
                      
               break;   
        case 2:if(TIMER_A>=Tempos[3] && LimpezaAcucar)      //Tempo de limpeza maior
                 {stepR1=3;
                  TIMER_A=0;
                  EstadoGrupos(0,3,3,1); //0: Verde, 1:VErmelho,3:Mant�m           //G2 Verde                
                 }
               break;
        case 3:if(TIMER_A>=Tempos[4]-Tempos[6]) //Verde m�nimo G2
                  {
                   MEM_I3=0;
                   stepR1=4;
                   auxTimeR1=Tempos[4]-Tempos[6];
                   TIMER_A=0;
                  }
               break;        
        case 4:if(TIMER_A>=Tempos[6]) 
                 {
                  auxTimeR1=auxTimeR1+Tempos[6];
                  if(ING_I1 && ING_I2)
                    {stepR1=5;                                        
                     EstadoGrupos(1,3,3,1); //0: Verde, 1:VErmelho,3:Mant�m        //ALLRED
                     ChamadaAcucar=0; 
                    }
                  else
                    {TIMER_A=0;                     
                    }
                 }
               break;
        case 5:if( ( !ING_I1 && !ING_I2 ) )    //Sentido Sa�da p/ entrada
                 {                  
                 stepR1=6;
                 TIMER_A=0;
                 }
               break;                
       case 6: if(TIMER_A>=Tempos[7]){ 
                  stepR1=0;                                  
                  TIMER_A=0;
                  EstadoGrupos(1,3,3,0); //0: Verde, 1:VErmelho,3:Mant�m           //G2 Verde
                 } //fprintf(PORTA,"Libertou Por contagem\n\r",);
                 break;              
                 }
                  
           
    
        
 // Rampa 2 - Semaforos 2 e 3 - Empilhadoras
 switch(stepR2){
 
        case 0:if(TIMER_B>=Tempos[8]-Tempos[10]) //Verde m�nimo G3
                  {
                   stepR2=1;
                   auxTimeR2=Tempos[8]-Tempos[10];
                   TIMER_B=0;
                  }
               break;
        case 1:if(TIMER_B>=Tempos[10]) 
                 {
                  auxTimeR2=auxTimeR2+Tempos[10];
                  if((auxTimeR2>Tempos[9]) && (MEM_I4 || ChamadaAcucar))  
                    {stepR2=2;
                     TIMER_B=0;
                     EstadoGrupos(3,1,1,3); //0: Verde, 1:VErmelho,3:Mant�m        //ALLRED                     
                    }
                  else
                    {TIMER_B=0;                     
                    }
                 }
               break;                            
        case 2:if(TIMER_B>=Tempos[11])
                 {
                  LimpezaAcucar=1;
                  stepR2=3;
                  TIMER_B=0;
                  EstadoGrupos(3,0,1,3); //0: Verde, 1:VErmelho,3:Mant�m           //G4 Verde
                 }
               break;
               
        case 3:if(TIMER_B>=Tempos[12]-Tempos[14]) //Verde m�nimo G2
                  {MEM_I4=0;
                   stepR2=4;
                   auxTimeR2=Tempos[12]-Tempos[14];
                   TIMER_B=0;
                  }
               break;        
        case 4:if(TIMER_B>=Tempos[14]) 
                 {                
                  auxTimeR2=auxTimeR2+Tempos[14];
                  if((auxTimeR2>Tempos[9] || MEM_I4==0) && !ChamadaAcucar) // Tempos 9 e n�o 13 porque quero Tmax igual para ambos os sentidos.
                    {                   
                     stepR2=5;
                     TIMER_B=0;
                     EstadoGrupos(3,1,1,3); //0: Verde, 1:VErmelho,3:Mant�m        //ALLRED
                    }                
                  else
                    {TIMER_B=0;
                     MEM_I4=0;
                    }
                 }
               break;
               
        case 5:if(TIMER_B>=Tempos[11])
                 {stepR2=0;
                  TIMER_B=0;
                  EstadoGrupos(3,1,0,3); //0: Verde, 1:VErmelho,3:Mant�m           //G2 Verde       
                 }
               break; 
        } 
  }





void main() {
unsigned int8 i=0,k=0;
int16 TIMER_A_OLD=0;


set_tris_a(0b00100011);
set_tris_b(0x00);              //Configura Porto B como sa�das
set_tris_c(0b10111111);


output_b(0x00);
setup_adc(ADC_OFF);

SETUP_TIMER_1(T1_INTERNAL|T1_DIV_BY_4);  //timer1 16bits
SET_TIMER1(3036);

enable_interrupts(INT_TIMER1);
enable_interrupts(INT_RDA);
enable_interrupts(global);                 // Habilito Interrupciones
delay_ms(100);

setup_wdt(WDT_2304MS);

fprintf(PORTA,"***Inicio de Funcionamento***\n\r");


// Ler EEPROM


//--------------- LOAD MODE FROM EEPROM -----------------                                                                         



//------------------- LOAD TIMES FROM EEPROM -----------------
//k=0;
//for(i=0;i<16;i++)
  {
   Tempos[i]=MAKE16(read_eeprom (k), read_eeprom (k+1));
   delay_ms(2);
   k=k+2;
  }
K=0;  //Endere�o EEPROM
Tempos[3]=MAKE16(read_eeprom (k), read_eeprom (k+1));  
K=2;  //Endere�o EEPROM
Tempos[7]=MAKE16(read_eeprom (k), read_eeprom (k+1)); 
K=4;  //Endere�o EEPROM
Tempos[9]=MAKE16(read_eeprom (k), read_eeprom (k+1)); 
K=6;  //Endere�o EEPROM
Tempos[11]=MAKE16(read_eeprom (k), read_eeprom (k+1));  
 
Tempos[0]=10;
Tempos[1]=20;
Tempos[2]=6;
//Tempos[3]=20;
Tempos[4]=10;
Tempos[5]=20;
Tempos[6]=6;
//Tempos[7]=20;
//Tempos[8]=10;
//Tempos[9]=15;
Tempos[10]=6;
//Tempos[11]=20;
//Tempos[12]=10;
Tempos[13]=20;
Tempos[14]=6;
//Tempos[15]=20;

                          

//--------------- TRAFFIC LIGHTS START --------------

//SET_TIMER1(3036);  //Restar TimerA
count_TimerA=0;
TIMER_A=0;         //TIMER SEGUNDOS
TIMER_A_OLD=0;
TIMER_A_OLD = TIMER_A;

output_low(PIN_A2);  //PIN RED Besouro ON
output_low(PIN_A3);  //PIN Green Besouro OFF



//**********Start traffic Lights**********

EstadoGrupos(1,1,0,0);


restart_wdt();
while(true)
{  
   //RESTART_WDT();
   if(BufferOK)  //Faz soma de checksum
     {
      //fprintf(PORTA,"***Buffer***\n\r");
      delay_ms(2);
      trata_buffer();
      BufferOK=0;
     }
    
restart_wdt();

if(TIMER_LC>=60 && LinhaComando)        //Reset timer linha comandos
   {LinhaComando=0;
   }
   
/*if(TIMER_A!=TIMER_A_OLD){ 
fprintf(PORTA,"StepR1 = %d \n\r",stepR1);
fprintf(PORTA,"StepR2 = %d \n\r",stepR2);
fprintf(PORTA,"MEMI3 = %d \n\r",MEM_I3);
fprintf(PORTA,"MEMI4 = %d \n\r",MEM_I4);
fprintf(PORTA,"Chamada Acucar = %d \n\r",ChamadaAcucar);
fprintf(PORTA,"AuxTimeR2= %d \n\r",auxTimeR2);

TIMER_A_OLD=TIMER_A;
}
*/

output_b(saida);


TimeOutTimer=TimeOut;          //IGNORA OK's e Timeout

LER_ENTRADAS();

ControlLights();
      

  
        
}
}




