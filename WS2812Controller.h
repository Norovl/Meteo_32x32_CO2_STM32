#ifndef WS2812CONTROLLER_H
#define WS2812CONTROLLER_H

#include <stdint.h>
//#include <Arduino.h>

class WS2812Controller {
private:
    uint8_t* leds_;  // Буфер данных светодиодов
    uint32_t next_mark;
    uint8_t dataPin;      // Пин для управления лентой
    uint32_t mPinMask;    // Маска для быстрого доступа к пину
    uint16_t num_leds;

    // Определения регистров
    typedef struct {
        volatile uint32_t CRL;
        volatile uint32_t CRH;
        volatile uint32_t IDR;
        volatile uint32_t ODR;
    } GPIO_TypeDef;

    #define GPIOA ((GPIO_TypeDef *)0x40010800UL)
    #define RCC_APB2ENR (*(volatile uint32_t *)0x40021018)
    #define DWT_BASE 0xE0001000UL
    #define DWT ((volatile uint32_t *)DWT_BASE)
    #define DWT_CYCCNT (DWT[1])
    #define DEMCR (*(volatile uint32_t *)(0xE000EDFCUL))


    // Тайминги для WS2811 (в тактах при 72 МГц)
    const uint32_t T0H = 30+5;  // 0.4 мкс
    const uint32_t T1H = 55+5;  // 0.8 мкс
    const uint32_t T0L = 60+5;  // 0.85 мкс
    const uint32_t T1L = 35+5;  // 0.45 мкс

    void EnableCycleCounter();
    void write8Bits(uint8_t b);

public:
    WS2812Controller(uint16_t numLeds, uint8_t pin);
    ~WS2812Controller();
    void setColor(uint16_t led, uint8_t red, uint8_t green, uint8_t blue);
    void setPixel(int16_t Num, uint32_t Col);
    void clear();
    void show();
};

#endif