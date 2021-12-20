#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>     // delay macros
#include <avr/interrupt.h>

#include "module.h"
#include "moduleLED.h"
#include "moduleIR.h"
#include "CAN.h"

volatile uint8_t cell_colour[4] = {0, 0, 0, 0};

cell_num_t cell_id[4] = {CELL_00, CELL_01, CELL_02, CELL_03};

IR_message_t IR_message_tx[4];

void setup(){
    cell_colour[0] = (configuration[0]);
    cell_colour[1] = (configuration[1]);
    cell_colour[2] = (configuration[2]);
    cell_colour[3] = (configuration[3]);
}

void loop(){
	for(int i = 0; i < 4; ++i){
        switch(cell_colour[i]){
            case 1:
                set_LED_with_brightness(cell_id[i], RED, HIGH);
                IR_message_tx[i].type = 1;
                IR_message_tx[i].data[0] = 22; //grey colour
                IR_message_tx[i].data[1] = 1; //obstacle true
                set_IR_message(&IR_message_tx[i], i);
                break;
            case 2:
                set_LED_with_brightness(cell_id[i], BLUE, HIGH);
                IR_message_tx[i].type = 1;
                IR_message_tx[i].data[0] = 22; //grey colour
                IR_message_tx[i].data[1] = 0; //obstacle false
                set_IR_message(&IR_message_tx[i], i);
                break;
            case 3:
                set_LED_with_brightness(cell_id[i], WHITE, HIGH);
                IR_message_tx[i].type = 1;
                IR_message_tx[i].data[0] = 21; //white colour
                IR_message_tx[i].data[1] = 0; //obstacle false
                set_IR_message(&IR_message_tx[i], i);
                break;
            case 4:
                set_LED_with_brightness(cell_id[i], LED_OFF, HIGH);
                IR_message_tx[i].type = 1;
                IR_message_tx[i].data[0] = 20; //black colour
                IR_message_tx[i].data[1] = 0; //obstacle false
                set_IR_message(&IR_message_tx[i], i);
                break;
            default:
                break;
           }
	}
}


int main() {
    module_init();

    module_start(setup, loop);

    return 0;
}
