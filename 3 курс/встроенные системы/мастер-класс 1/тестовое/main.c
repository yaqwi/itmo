#include "main.h"
#include "tm1637.h"

volatile uint32_t tickCount; // время в мс
uint32_t last_display_update;
uint16_t counter;
char lastKey;
uint32_t lastScanTime;

uint16_t timerValue = 0; // значение таймера
uint8_t timerRunning = 0;  // запущен ли таймер
uint32_t lastTimerUpdate = 0; // когда в последний раз обновляли таймкр


void osSystickHandler(void) {
  tickCount++;
}


void initGPIO() {
  // Включаем тактирование GPIOA и GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

  // Настраиваем PA5 как выход
  GPIOA->MODER = (GPIOA->MODER & ~(3 << 10)) | (1 << 10);
  GPIOA->OTYPER &= ~(1 << 5);
  GPIOA->OSPEEDR |= (1 << 10);
}

void initUSART2() {
  // Включаем тактирование USART2
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // Настраиваем PA2 и PA3 в альтернативный режим
  GPIOA->MODER = (GPIOA->MODER & ~(0xF << 4)) | (0xA << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF << 8)) | (1 << 8) | (1 << 12);

  // Настраиваем USART2
  USART2->BRR = 417; // 48MHz/115200
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void initSysTick() {
  SysTick->LOAD = 47999; // 1ms при 48MHz
  SysTick->VAL = 0;
  SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}

int _write(int file, uint8_t *ptr, int len) {
  for (int i = 0; i < len; i++) {
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ptr[i];
  }
  return len;
}

// удалила checkTickCount

void updateTimer() {
  if (timerRunning && (tickCount - lastTimerUpdate >= 1000)) { //прошла ли секунда
    lastTimerUpdate = tickCount;

    if (timerValue > 0) {
      timerValue--;
      tm1637_display_number(timerValue);
      printf("Timer: %d\n", timerValue); // обратный отсчет 

      if (timerValue == 0) { // когда доходим до 0
        GPIOA->ODR |= (1 << 5); // включаем лампочку
        timerRunning = 0; // закончили работу
      }
    }
  }
}

int main(void) {
  initGPIO();
  initUSART2();
  initSysTick();
  initKeyboard();
  tm1637_init();
  tm1637_display_number(0); // на таймере должен быть 0

  printf("Hello, %s!\n", "Wokwi Simulation");

  // GPIOA->ODR |= (1 << 5); // Включаем LED (должно быть выключено при запуске)

  while (1) {
    updateTimer();
    scanKeyboard();
  }

  return 0;
}