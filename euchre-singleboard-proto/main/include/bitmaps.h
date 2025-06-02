#ifndef BITMAPS_H
#define BITMAPS_H

#include <stdlib.h>

#define BITMAP_DIM 8

const uint8_t HEART[BITMAP_DIM] = {
    0b01100110,
    0b11111110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00001000
};

const uint8_t DIAMOND[BITMAP_DIM] = {
    0b00001000,
    0b00011100,
    0b00111110,
    0b01111111,
    0b00111110,
    0b00011100,
    0b00001000,
    0b00000000
};

const uint8_t CLUB[8] = {
    0b00011000,
    0b00111100,
    0b00111100,
    0b00011000,
    0b01111110,
    0b00011000,
    0b00111100,
    0b00000000
};

const uint8_t SPADE[8] = {
    0b00001000,
    0b00011100,
    0b00111110,
    0b01111111,
    0b00111110,
    0b00011100,
    0b00011100,
    0b00011100
};

const uint8_t BLANK[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};

#endif // BITMAPS_H