#define REPEAT_SLEEP 50

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

    while(!stdio_usb_connected()) busy_wait_ms(500);

    //initialize carrier pio
    uint carrier_offset = pio_add_program(pio, &nec_carrier_program);
    nec_carrier_program_init(pio, carrier_sm, carrier_offset, 16);

    //initialize control pio
    uint control_offset = pio_add_program(pio, &nec_control_program);
    nec_control_program_init(pio, control_sm, control_offset);

    //initialize repeat pio
    uint repeat_offset = pio_add_program(pio, &nec_repeat_program);
    nec_repeat_program_init(pio, repeat_sm, repeat_offset);

    uint32_t frame;
    char input[256];     //input buffer and input character
    char *mode_string;          //pointer for mode token
    int strindex;               //index of current character being read in
    int inputc;

    uint8_t address, data;      //bytes for address/data to be converted into an NEC frame

    while(1){
        
        strindex = 0;

        //read a string to the input buffer
        do{
            while((inputc = getchar_timeout_us(10)) < 0);
            putchar(inputc);
            input[strindex++] = inputc;
        } while(inputc != '\n' && inputc != '\r');
        input[strindex] = '\0';
        printf("\n");


        //grab NEC frame info and mode
        address = atoi(strtok(input, ","));
        data = atoi(strtok(NULL, ","));
        mode_string = strtok(NULL, ",");       

        //given a valid address, use the NEC_control pio sm to send the frame
        if(address){
            pio_sm_put(pio, control_sm, nec_frame(address, data));
            printf("Sending %02x to %02x\n", data, address);
        }
        //with a null address, clear a repeat signal
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
                printf("Repeating same command until next return\n");
                nec_repeat(false);
                do{
                    inputc = getchar_timeout_us(0);
                    sleep_ms(REPEAT_SLEEP);
                    pio_sm_put(pio, control_sm, nec_frame(address, data));
                } while(inputc != '\r' && inputc != '\n');
                printf("Ending repeating command\n");
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