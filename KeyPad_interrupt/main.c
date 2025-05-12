/*
 * KeyPad_Interrupt.c
 *
 * Created: 2025-04-30 오후 5:36:24
 * Author : Jiheon Choi
 * Info   : Electronics Engineering Embedded system
 *			2021146036
 *			a88217281@gmail.com
 */ 

#define F_CPU 14.7456E6
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ADD 0x0A
#define SUB 0x0B
#define MUL 0x0C
#define DIV 0x0D
#define RESET 0x0E
#define RESULT 0x0F
#define NON_CLICK 0x88
#define EMPTY INT16_MAX
#define DEBOUNCING 200
#define BAR 10
#define NEG 11

volatile uint8_t scan_line = 0xf7;
uint8_t *key_num;
// add = a, sub = b, mul = c, div = d;
/* 실습 보드 기준
 1 2 3 sub
 4 5 6 mult
 7 8 9 div
 reset 0 result add
*/
/* proteus 
 7 8 9 div	
 4 5 6 mult
 1 2 3 sub
 reset 0 result add
*/
ISR(INT4_vect){
	uint8_t line = (~scan_line) & 0x0f;
	cli();
	switch (line)
	{
		case 0x01: *key_num = 0x1; break;
		case 0x02: *key_num = 0x4; break;
		case 0x04: *key_num = 0x7; break;
		case 0x08: *key_num = RESET; break;
		default: *key_num = NON_CLICK;
	}
	_delay_ms(DEBOUNCING); // 프로테우스에서 채터링 발생 시간이 맞지 않으므로 인위적인 delay
	sei();
}
ISR(INT5_vect){
	uint8_t line = (~scan_line) & 0x0f;
	cli();
	switch (line)
	{
		case 0x01: *key_num = 0x2; break;
		case 0x02: *key_num = 0x5; break;
		case 0x04: *key_num = 0x8; break;
		case 0x08: *key_num = 0x0; break;
		default: *key_num = NON_CLICK;
	}
	_delay_ms(DEBOUNCING); // 프로테우스에서 채터링 발생 시간이 맞지 않으므로 인위적인 delay
	sei();
}
ISR(INT6_vect){
	uint8_t line = (~scan_line) & 0x0f;
	cli();
	switch (line)
	{
		case 0x01: *key_num = 0x3; break;
		case 0x02: *key_num = 0x6; break;
		case 0x04: *key_num = 0x9; break;
		case 0x08: *key_num = RESULT; break;
		default: *key_num = NON_CLICK;
	}
	_delay_ms(DEBOUNCING); // 프로테우스에서 채터링 발생 시간이 맞지 않으므로 인위적인 delay
	sei();
}
ISR(INT7_vect){
	uint8_t line = (~scan_line) & 0x0f;
	cli();
	switch (line)
	{
		case 0x01: *key_num = SUB; break;
		case 0x02: *key_num = MUL; break;
		case 0x04: *key_num = DIV; break;
		case 0x08: *key_num = ADD; break;
		default: *key_num = NON_CLICK;
	}
	_delay_ms(DEBOUNCING); // 프로테우스에서 채터링 발생 시간이 맞지 않으므로 인위적인 delay
	sei();
}
// 0b 1011 1111 
uint8_t num_data[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90, 0xbf, 0x7f,0xff};
uint8_t sel_FND[4] = {0x80, 0x40, 0x20, 0x10};

void PORT_INIT(void){
	DDRC = 0xC0;
	DDRD = 0xf0;	// FND Select
	DDRB = 0xff;	// FND print
	DDRE = 0x0f;	// KeySwitch set // Upper nibble input key, Low nibble scan line
	EIMSK = (1<<INT4) | (1<<INT5) | (1<<INT6) | (1<<INT7);
	EICRB = (1<<ISC71) | (1<<ISC70) | (1<<ISC61) | (1<<ISC60) | (1<<ISC51) | (1<<ISC50) | (1<<ISC41) | (1<<ISC40);// INT4~7 rising edge trigger
	sei();			// Interrupt permit
}


void Reset_Val_arr(uint8_t *arr){
	uint8_t i = 0;
	for(i=0;i<4;i++){
		*(arr+i) = NON_CLICK;
		PORTE = sel_FND[i];
		PORTB = 0xff;
		_delay_ms(10);
	}
	
	
}
void Set_Val_arr(uint8_t *arr, int16_t num){
	if(num > 9999){
		*(arr+3) = BAR;
		*(arr+2) = BAR;
		*(arr+1) = BAR;
		*(arr)	 = BAR;
		PORTC = 0x30;
		_delay_ms(200); 
		PORTC = 0xF0;
	}
	else if(num < 0){
		*(arr+3) = NEG;
		*(arr+2) = NEG;
		*(arr+1) = NEG;
		*(arr)	 = NEG;
		PORTC = 0x70;
		_delay_ms(200);
		PORTC = 0xF0;
	}
	else{
		*(arr+3) = (uint8_t)num/1000;
		num %= 1000;
		*(arr+2) = (uint8_t)num/100;
		num %= 100;
		*(arr+1) = (uint8_t)num/10;
		*arr = (uint8_t)num%10;
		if(*(arr+3) == 0)							*(arr+3) = NON_CLICK;
		if(*(arr+3) == NON_CLICK && *(arr+2) == 0)	*(arr+2) = NON_CLICK;
		if(*(arr+2) == NON_CLICK && *(arr+1) == 0)	*(arr+1) = NON_CLICK;
		PORTC = 0xB0;
		_delay_ms(200);
		PORTC = 0xF0;
	}
}
int16_t Set_number(uint8_t *arr){
	int16_t num = 0;
	if(*(arr+3) >= 0 && *(arr+3) <= 9)	num =  (int16_t)*(arr+3)*1000;
	if(*(arr+2) >= 0 && *(arr+2) <= 9)	num += (int16_t)*(arr+2)*100;
	if(*(arr+1) >= 0 && *(arr+1) <= 9)	num += (int16_t)*(arr+1)*10;
	if(*(arr) >= 0	 && *(arr) <= 9)	num += (int16_t)*(arr);
	Reset_Val_arr(arr);
	return num;
}
int16_t Calculate(int16_t *num1, int16_t *num2, uint8_t *op){
	int16_t res = EMPTY;
	if(*num1 != EMPTY && *num2 != EMPTY && (*op >= ADD && *op <= DIV)){
		switch (*op)
		{
			case ADD:
				res = *num1 + *num2;
			break;
			case SUB:
				res = *num1 - *num2;
			break;
			case MUL:
				res = *num1 * *num2;
			break;
			case DIV:
				if(*num1 != 0 && *num2 != 0){
					res = *num1 / *num2;
				}
				else res = EMPTY;
			break;
		}
	}
	*num1 = EMPTY;
	*num2 = EMPTY;
	*op = NON_CLICK;
	return res;
}

int main(void){
	uint8_t i = 0;
	uint8_t key_in = NON_CLICK;
	uint8_t val[4] = {NON_CLICK,NON_CLICK,NON_CLICK,NON_CLICK};
	key_num = &key_in;
	volatile uint8_t *MY_FND = &PORTD;
	volatile uint8_t *key_scan_chk = &PORTE;
	uint8_t op = NON_CLICK;
	int16_t num1 =EMPTY, num2 = EMPTY, result = EMPTY;
	PORT_INIT();
	PORTC = 0xFF;
	while(1){
		if(key_in>=0 && key_in <=9) {
			if(result != EMPTY) {
				Reset_Val_arr(val);
				result = EMPTY;
			}
			val[3] = val[2];
			val[2] = val[1];
			val[1] = val[0];
			val[0] = key_in;
			key_in = NON_CLICK;
		}
		else if(key_in == ADD || key_in == SUB || key_in == MUL || key_in == DIV){
			num1 = Set_number(val);
			op=key_in;
			key_in = NON_CLICK;
		}
		if(key_in == RESULT){
			num2 = Set_number(val);
			result = Calculate(&num1, &num2, &op);
			Set_Val_arr(val,result);
			key_in = NON_CLICK;
		}
		else if(key_in == RESET){
			Reset_Val_arr(val);
			num1 = EMPTY;
			num2 = EMPTY;
			result = EMPTY;
			op = NON_CLICK;
			key_in = NON_CLICK;
			
		}
		for(i=0;i<4;i++){
			*key_scan_chk = scan_line;
			if(val[i] != NON_CLICK){
				*MY_FND = sel_FND[i];	
				PORTB = num_data[val[i]];
				_delay_ms(10); // 출력용 delay (잔상효과)
			}
			else {
				*MY_FND = sel_FND[i];
				PORTB = 0xff;
			}
			_delay_us(100);	// scan line 간에 delay
			scan_line = scan_line >> 1;
		}
		scan_line = 0xf7;
	}
}
