#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "nec_carrier.pio.h"
#include "nec_control.pio.h"
#include "nec_repeat.pio.h"

uint32_t nec_frame(uint8_t addr, uint8_t data);
void nec_repeat(bool enabled);

PIO pio = pio0;
uint carrier_sm = 0, control_sm = 1, repeat_sm = 2;

int main() {
    // Enable UART so we can print status output
    stdio_init_all();
    stdin_uart_init();

    

    //initialize carrier pio
    uint carrier_offset = pio_add_program(pio, &nec_carrier_program);
  //  int carrier_sm = pio_claim_unused_sm(pio, true);
    nec_carrier_program_init(pio, carrier_sm, carrier_offset, 16);

    //initialize control pio
    uint control_offset = pio_add_program(pio, &nec_control_program);
  //  volatile int control_sm = pio_claim_unused_sm(pio, true);
    nec_control_program_init(pio, control_sm, control_offset);

    //initialize repeat pio
    uint repeat_offset = pio_add_program(pio, &nec_repeat_program);
    nec_repeat_program_init(pio, repeat_sm, repeat_offset);

    uint32_t frame;
    char input[256], inputc;
    char *mode_string;

    uint8_t address, data;
    int strindex;

    while(1){
        
        strindex = 0;

        do{
            inputc = uart_getc(uart0);
            printf("%c", inputc);
            input[strindex++] = inputc;
        } while(inputc != '\n' && inputc != '\r');

        input[strindex] = '\0';
        printf("\n");

        address = atoi(strtok(input, ","));
        data = atoi(strtok(NULL, ","));
        mode_string = strtok(NULL, ",");       

        if(address){
            pio_sm_put(pio, control_sm, nec_frame(address, data));
            printf("sending %02x to %02x\n", data, address);
        }
        else{
            printf("Clearing repeat signal\n");
            nec_repeat(false);
        }

        if(mode_string){
            if(mode_string[0] == 'h') {
                printf("Sending repeat signal until next command\n");
                nec_repeat(true);
            }
            if(mode_string[0] == 'r'){
                printf("Ending repeat signal\n");
                nec_repeat(false);
            }
        }
    }

}

uint32_t nec_frame(uint8_t addr, uint8_t data){
    return addr | (addr ^ 0xff) << 8 | (data << 16) | (data ^ 0xff) << 24;
}

void nec_repeat(bool enabled){

    if(enabled) pio_sm_restart(pio, repeat_sm);

    pio_sm_set_enabled(pio, repeat_sm, enabled);
}