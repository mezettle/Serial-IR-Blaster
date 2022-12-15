#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "nec_carrier.pio.h"
#include "nec_control.pio.h"


int main() {
    // Enable UART so we can print status output
    stdio_init_all();

    //initialize carrier pio
    uint carrier_offset = pio_add_program(pio0, &nec_carrier_program);
    int carrier_sm = pio_claim_unused_sm(pio0, true);
    nec_carrier_program_init(pio0, carrier_sm, carrier_offset, 16);

    //initialize control pio
    uint control_offset = pio_add_program(pio0, &nec_control_program);
    int control_sm = pio_claim_unused_sm(pio0, true);
    nec_control_program_init(pio0, control_sm, control_offset);

    uint32_t test_com = 0b01000000101111110001011111101000;

    while(1){
        test_com = 0b01000000101111110001011111101000;
        busy_wait_ms(1000);
        pio_sm_put(pio0, control_sm, test_com);
        printf("testing %x\n", test_com);
    }

}

