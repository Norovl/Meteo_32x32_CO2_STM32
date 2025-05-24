#include "WS2812Controller.h"

WS2812Controller::WS2812Controller(uint16_t numLeds, uint8_t pin)
  : num_leds(numLeds), dataPin(pin), mPinMask(1 << pin) {
  leds_ = new uint8_t[num_leds * 3];

  // Настройка GPIO (выбранный пин как выход)
  RCC_APB2ENR |= (1 << 2);  // Включение тактирования GPIOA
  GPIOA->CRL = (GPIOA->CRL & ~(0xF << (dataPin * 4))) | (0x3 << (dataPin * 4));
  clear();
}

WS2812Controller::~WS2812Controller() {
  delete[] leds_;
}

void WS2812Controller::EnableCycleCounter() {
  DEMCR |= (1 << 24);  // DEMCR_TRCENA
  DWT_CYCCNT = 0;
  DWT[0] |= 1;  // DWT_CTRL_CYCCNTENA
}

void WS2812Controller::write8Bits(uint8_t b) {
  for (uint32_t i = 8; i > 0; i--) {
    while (DWT_CYCCNT < next_mark)
      ;
    GPIOA->ODR |= mPinMask;  // HIGH

    next_mark += (b & 0x80) ? T1H : T0H;
    while (DWT_CYCCNT < next_mark)
      ;

    GPIOA->ODR &= ~mPinMask;  // LOW
    next_mark += (b & 0x80) ? T1L : T0L;
    b <<= 1;
  }
}

void WS2812Controller::setColor(uint16_t led, uint8_t red, uint8_t green, uint8_t blue) {
  if (led >= num_leds) return;
  uint16_t i = led * 3;
  leds_[i++] = green;
  leds_[i++] = red;
  leds_[i] = blue;
}

void WS2812Controller::setPixel(int16_t Num, uint32_t Col) {
  setColor(Num, (Col >> 16) & 0xFF, (Col >> 8) & 0xFF, Col & 0xFF);
}

void WS2812Controller::clear() {
  for (uint16_t i = 0; i < num_leds * 3; i++) {
    leds_[i] = 0;
  }
}

void WS2812Controller::show() {

  // Отключаем все глобальные прерывания
  asm volatile("cpsid i");

  EnableCycleCounter();
  next_mark = T1H;
  for (uint16_t i = 0; i < num_leds * 3; i++) {
    write8Bits(leds_[i]);
  }

  GPIOA->ODR &= ~mPinMask;  // LOW
  while (DWT_CYCCNT < next_mark + 600000 / 14)
    ;

  // Включаем прерывания обратно
  asm volatile("cpsie i");
}