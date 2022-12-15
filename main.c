#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "nec_carrier.pio.h"
#include "nec_control.pio.h"

uint32_t nec_frame(uint8_t addr, uint8_t data);

int main() {
    // Enable UART so we can print status output
    stdio_init_all();
    stdin_uart_init();

    PIO pio = pio0;

    //initialize carrier pio
    uint carrier_offset = pio_add_program(pio, &nec_carrier_program);
    int carrier_sm = pio_claim_unused_sm(pio, true);
    nec_carrier_program_init(pio, carrier_sm, carrier_offset, 14);

    //initialize control pio
    uint control_offset = pio_add_program(pio, &nec_control_program);
    volatile int control_sm = pio_claim_unused_sm(pio, true);
    nec_control_program_init(pio, control_sm, control_offset);

    uint32_t frame;
    char input[256], inputc;
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


       pio_sm_put(pio, control_sm, nec_frame(address, data));
      printf("sending %02x to %02x\n", data, address);
    }

}

uint32_t nec_frame(uint8_t addr, uint8_t data){
    return addr | (addr ^ 0xff) << 8 | (data << 16) | (data ^ 0xff) << 24;
}
