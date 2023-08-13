//#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "SH1106.cpp"

int main() {
    stdio_init_all();
    printf("start");
    sleep_ms(1000);


    SH1106::init(true);

    while(true);
    return 0;
}