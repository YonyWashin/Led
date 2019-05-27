#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#define SHIFT_REGISTER_DDRX  DDRB
#define SHIFT_REGISTER_PORT PORTB
#define DATA  (1<<PB3)     
#define SS    (1<<PB2)    
#define CLOCK (1<<PB5)       
#define LATCH()  SHIFT_REGISTER_PORT |=(1<<PB2);PORTB&=~(1<<PB2);
#define DataHigh SHIFT_REGISTER_PORT |=DATA;
#define DataLow  SHIFT_REGISTER_PORT &=~DATA;
#define ClkHigh  SHIFT_REGISTER_PORT |=CLOCK;
#define ClkLow   SHIFT_REGISTER_PORT &=~CLOCK;

void spi_init();
uint8_t send_74hc595_spi( volatile uint8_t data);
void TickPCM(uint16_t i);
void tim_init( void ) ;
void led_encode_timeslices( uint8_t intensity[] );
void showFrames();
void setAllOf(); 
void cometaGoIzquierdo();
void cometaGoDerecho();
void init_ports();

uint16_t ValueTik[8]={1,4,8,16,32,64,128,256}; 
volatile uint8_t g_timeslice[8] ; // Array de 8 para almacenar la codificación PCM
volatile uint8_t g_bitpos = 0;
uint8_t brightness[8];  // Array de 8 para almacenar el nivel de brillo de cada canal

int main(void)
{
	init_ports();
		
	while(1)
	{
		cometaGoIzquierdo(); //efecto cometa
		cometaGoDerecho();		
	}
	return(0);
}
void cometaGoIzquierdo(){
	const uint8_t cometa[4]={180,60,20,2}; 
	uint8_t dec=255,a;
	 setAllOf();
	 
 while(++dec<4){
	for(a=0;a<8;a++)brightness[a]=brightness[a+1];
	brightness[7]=cometa[dec];
	led_encode_timeslices( brightness);_delay_ms(100);
 }dec=255;
 while(++dec<8){ 
 for(a=0;a<8;a++)brightness[a]=brightness[a+1];
 brightness[7]=0;
 led_encode_timeslices( brightness);
 _delay_ms(100);
 }
}

void cometaGoDerecho(){
 const uint8_t cometa[4]={180,60,20,2}; 
 uint8_t dec=255,a;
 setAllOf();
	 
 while(++dec<4){
	for(a=0;a<8;a++)brightness[7-a]=brightness[6-a];
	brightness[0]=cometa[dec];
	led_encode_timeslices( brightness);_delay_ms(100);
 }dec=255;
 
 while(++dec<8){ 
 for(a=0;a<7;a++)brightness[7-a]=brightness[6-a];
 brightness[0]=0;
 led_encode_timeslices( brightness);
 _delay_ms(100);
 }
}

void setAllOf(){
	for (uint8_t chanl=0;chanl<8;chanl++)brightness[chanl]=0;
}

void TickPCM(uint16_t i){
	while(--i!=0){
		_delay_loop_1(20);
	}
}
void init_ports()
{
	tim_init();
	spi_init();
	sei();
}
void showFrames()
{
	send_74hc595_spi(g_timeslice[g_bitpos]);
	TickPCM(ValueTik[g_bitpos++]);	
	g_bitpos = g_bitpos&0x07;
}

void tim_init( void )
{
	TCCR2A |= (1<<WGM21) ; // set the timer to CTC mode.
	TCCR2B |= (1<<CS22|1<<CS21|1<<CS20) ; 
	OCR2A = 1 ; 
	TIMSK2 |= (1 << OCIE2A) ; // Enable the Compare Match interrupt
	g_bitpos = 0 ;
}
// codifica en un array de 8 posiciones los niveles de brillo
// esta codification los envia al registro con modulacion PCM
void led_encode_timeslices( uint8_t intensity[] )
{
	uint8_t portbits = 0;
	uint8_t bitvalue ;
	
	for ( uint8_t bitpos = 0 ; bitpos < 8 ; bitpos++ )
	{
		portbits = 0;
		bitvalue = 1 ;
		for ( uint8_t ledpos = 0 ; ledpos < 8 ; ledpos++ )
		{
			if (intensity[ ledpos ] & (1 << bitpos)) portbits |= bitvalue ;
			bitvalue = bitvalue << 1 ;
		}
		g_timeslice[ bitpos ] = portbits ;
	}
}

// Timer interrupt handler 
ISR( TIMER2_COMPA_vect ){
	
	send_74hc595_spi(g_timeslice[g_bitpos]);		
	OCR2A <<= 1 ;
	if (g_bitpos == 0) {OCR2A = 1;} 
	g_bitpos++;
	g_bitpos &=0x07;	
	TCNT2 = 0;	
}
void spi_init(){
	SHIFT_REGISTER_DDRX |=(DATA |SS | CLOCK);  // confi pines SPI
	SHIFT_REGISTER_PORT &= ~(DATA | SS | CLOCK);
	SPSR |= 1<<SPI2X; 
	SPCR |=(1<<MSTR)|(1<<SPE);// port SPI modo MAESTRO y Habilitamos
}

uint8_t send_74hc595_spi(volatile uint8_t data) {	 
	 SPDR = data;
	 while(!(SPSR & (1<<SPIF)));
	 LATCH();
	 return SPDR;
}