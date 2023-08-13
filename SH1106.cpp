#ifndef SH1106_F
#define SH1106_F

#include <string>
#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"

#include "font.cpp"

#define assertm(exp, msg) assert(((void)msg, exp))

namespace SH1106
{
    spi_inst_t *SPI_PORT = spi0;
    const uint BOUNDRATE = 1000*1000;

    const uint8_t RX_PIN = 0;
    const uint8_t CS_PIN = 1;
    const uint8_t CL_PIN = 2;
    const uint8_t TX_PIN = 3;

    const uint8_t DC_PIN  = 4;
    const uint8_t RES_PIN = 5;

    const uint8_t SCREEN_SIZE_X = 128;
    const uint8_t SCREEN_SIZE_Y = 64;

    bool frame[SCREEN_SIZE_X][SCREEN_SIZE_Y] ;
    void setDC_ToData();
    void setDC_ToCommand();
    void reset();
    void dataSend(uint8_t *data, uint dataSize);
    void commandSend(uint8_t *data, uint dataSize);


    void segmentReMap(bool toRight) {
        uint8_t data[1];
        if (toRight) data[0] = 0b1010'0000;
        else data[0]         = 0b1010'0001;
        commandSend(data, 1);
    }

    void normalReverseMode(bool normal) {
        uint8_t data[1];
        if (normal) data[0]  = 0b1010'0110;
        else data[0]         = 0b1010'0111;
        commandSend(data, 1);
    }

    void setMultiplex(uint8_t multiplex) {
        uint8_t data[2] {0b10101000, multiplex};
        commandSend(data, 2);
    }

    void entireDisplaySwitch(bool on) {
        uint8_t data[1];
        if (on) data[0] = 0b1010'0101;
        else data[0] =    0b1010'0100;
        commandSend(data, 1);

    }

    void setPageAdress(uint8_t address) {
        assertm(address < 8, "Page address to large");
        address = address & 0b0000'1111;
        uint8_t data[1] = {(0b1011'0000 | address)};
        commandSend(data, 1);
    }

    void setCommonOutputScan(bool normal) {
        uint8_t data[1];
        if (normal) data[0] = 0b1100'0000;
        else        data[0] =    0b1100'1000;
        commandSend(data, 1);
    }

    void setOffset(bool offset) {
        uint8_t data[2] {0b1101'0011, offset};
        commandSend(data, 2);
    }

    void setDisplayClockDivider(uint8_t div, uint8_t freqInc) {
        uint8_t data[2] {0b1101'0101, (div | freqInc)};
        commandSend(data, 2);
    }

    void setPrechargePeriod(uint8_t precharge, uint8_t discharge) {
        uint8_t data[2] {0b1101'1001, (precharge | discharge)};
        commandSend(data, 2);
    }

    void DC_DC_CONVERTER(bool on) {
        uint8_t data[2] {0b1010'1101, 0b1000'1010};
        if (on) data[1] = 0b1000'1011;
        commandSend(data, 2);
    }

    void pumpVoltage(bool on) {
        uint8_t data[1] {0b0011'0010};
        commandSend(data, 2); 
    }

    void turnDisplay(bool on) {
        uint8_t data[1];
        if (on) data[0] = 0b1010'1111;
        else    data[0] = 0b1010'1110;
        commandSend(data, 1);
    }
    
    void setColumn(uint8_t collumnId) {
        collumnId += 2;
        uint8_t data[2] {(collumnId & 0b0000'1111), ((collumnId >> 4) | 0b0001'0000)};
        commandSend(data, 2);
    }

    void setLine (uint8_t lineId) {
        uint8_t data[1] {lineId | 0b0100'0000};
        commandSend(data, 1);
    }

    void setContrast(uint8_t contrast) {
        uint8_t data[2] = {0b1000'0001, contrast};
        commandSend(data, 2);
    }



    void resetz() {
        gpio_put(RES_PIN, 1);
        sleep_ms(1);
        gpio_put(RES_PIN, 0);
        sleep_ms(1);
        gpio_put(RES_PIN, 1);
    }
    void transmisionEnd() {
        gpio_put(CS_PIN, 1);
    }

    void transmisionStart() {
        gpio_put(CS_PIN, 0);
    }
    void dataSend(uint8_t *data, uint dataSize) {
        setDC_ToData();
        transmisionStart();
        spi_write_blocking(SPI_PORT, data, dataSize);

        transmisionEnd();
    }

    void commandSend(uint8_t *data, uint dataSize) {
        setDC_ToCommand();
        transmisionStart();
        spi_write_blocking(SPI_PORT, data, dataSize);
        transmisionEnd();
    }
    void setDC_ToData() {
        gpio_put(DC_PIN, 1);
    }

    void setDC_ToCommand() {
        gpio_put(DC_PIN, 0);
    }



    void initSPI() {
        spi_init(SPI_PORT, BOUNDRATE);

        gpio_set_function(RX_PIN  ,  GPIO_FUNC_SPI);
        gpio_set_function(CL_PIN   ,  GPIO_FUNC_SPI);
        gpio_set_function(TX_PIN  ,  GPIO_FUNC_SPI);

        gpio_init(CS_PIN);
        gpio_set_dir(CS_PIN, GPIO_OUT);
        gpio_put(CS_PIN, 1);

    }

    void cleanRam() {
        uint8_t data[200];
        for (int i = 0; i < 200;i++)data[i] = 0;
        for (int p = 0; p < 8; p++) {
            setPageAdress(p);
            setColumn(0);
            setLine(0);
            dataSend(data, 200);
        }
    }
    void sendCommand(uint8_t c) {
        setDC_ToCommand();
        transmisionStart();
        spi_write_blocking(SPI_PORT, &c, 1);
        transmisionEnd();
    }

    void displayFrame() {
        for (int y = 0; y < 8; y++) {
            uint8_t line[128]; 
            setColumn(0);
            setPageAdress(y);
            for (int x = 0; x < 128; x++) {
                line[x]  =    (frame[x][(y*8)  ]
                            | (frame[x][(y*8)+1] << 1)
                            | (frame[x][(y*8)+2] << 2)
                            | (frame[x][(y*8)+3] << 3)
                            | (frame[x][(y*8)+4] << 4)
                            | (frame[x][(y*8)+5] << 5)
                            | (frame[x][(y*8)+6] << 6)
                            | (frame[x][(y*8)+7] << 7));
            }
            dataSend(line, 128);
        printf("frame display end\n");
        }
    }

    void init(bool initSpi) {
        if (initSpi) initSPI();
        FONT::border_x = SCREEN_SIZE_X;
        FONT::border_y = SCREEN_SIZE_Y;

        gpio_init(DC_PIN);
        gpio_set_dir(DC_PIN, GPIO_OUT);
        gpio_put(DC_PIN, 1);

        gpio_init(RES_PIN);
        gpio_set_dir(RES_PIN, GPIO_OUT);
        gpio_put(RES_PIN, 1);
        resetz();
        turnDisplay(false);

        setContrast(125);
        setPageAdress(0);
        setColumn(0);
        setLine(0);
        entireDisplaySwitch(false);

        turnDisplay(true);
        sleep_ms(100);
        cleanRam();
    }

} // namespace SH1106



#endif