#include "keyboard.h"
#include <string.h>
#include "main.h"
#include "tm1637.h"

uint32_t startBlock = 0;
uint32_t block = 0;

extern uint8_t digitCount;
extern char enteredCode[5];
extern char code[];
extern char newCode[5];

uint8_t changeCodeStep = 0;


void initKeyboard() {
  // Настройка строк (R1-R4) как выходы с открытым стоком
  // PB7, PB6, PB3, PA10
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (7 * 2))) | (1U << (7 * 2));
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (6 * 2))) | (1U << (6 * 2));
  GPIOB->MODER = (GPIOB->MODER & ~(3U << (3 * 2))) | (1U << (3 * 2));
  GPIOA->MODER = (GPIOA->MODER & ~(3U << (10 * 2))) | (1U << (10 * 2));

  GPIOB->OTYPER |= (1 << 7) | (1 << 6) | (1 << 3);
  GPIOA->OTYPER |= (1 << 10);

  // Настройка столбцов (C1-C3) как входы с подтяжкой к питанию
  // PB10, PB4, PB5
  GPIOB->MODER &= ~(3U << (10 * 2) | 3U << (4 * 2) | 3U << (5 * 2));
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (10 * 2))) | (1U << (10 * 2));
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (4 * 2))) | (1U << (4 * 2));
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (5 * 2))) | (1U << (5 * 2));

  lastKey = '\0';
  lastScanTime = 0;
}

char readKey() {
  const uint8_t rows[] = {7, 6, 10, 3};    // Пины строк
  const uint8_t cols[] = {10, 4, 5};       // Пины столбцов
  const char keymap[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
  };

  for (uint8_t i = 0; i < 4; i++) {
    // Активируем текущую строку
    if (rows[i] != 10) {
      GPIOB->BSRR = (1 << (rows[i] + 16));
    } else {
      GPIOA->BSRR = (1 << (rows[i] + 16));
    }

    // Небольшая задержка для стабилизации
    for (volatile int d = 0; d < 100; d++);

    // Проверяем столбцы
    for (uint8_t j = 0; j < 3; j++) {
      if ((GPIOB->IDR & (1 << cols[j])) == 0) {
        // Деактивируем строку перед возвратом
        if (rows[i] != 10) {
          GPIOB->BSRR = (1 << rows[i]);
        } else {
          GPIOA->BSRR = (1 << rows[i]);
        }
        return keymap[i][j];
      }
    }

    // Деактивируем строку
    if (rows[i] != 10) {
      GPIOB->BSRR = (1 << rows[i]);
    } else {
      GPIOA->BSRR = (1 << rows[i]);
    }
  }

  return '\0';
}


void checkCode() {
  startBlock = tickCount;

  if (strcmp(enteredCode, code) == 0) {
    printf("Correct code! :)\n");

    GPIOA->ODR &= ~(1 << 5);
    GPIOA->ODR |= (1 << 9);

    block = 3000;

  } else {
    printf("Wrong :(\n");

    GPIOA->ODR &= ~(1 << 5);
    GPIOA->ODR |= (1 << 15);

    block = 1500;
  }
}


void changeCode() {
  if (changeCodeStep == 1) {
    if (strcmp(enteredCode, code) == 0) {
      printf("Current code is correct. Enter new code\n");

      changeCodeStep = 2;

      digitCount = 0;
      enteredCode[0] = '\0';
      tm1637_clear();
    } else {
      printf("Wrong current code :(\n");

      changeCodeStep = 0;
      startBlock = tickCount;

      GPIOA->ODR &= ~(1 << 5);
      GPIOA->ODR |= (1 << 15);

      block = 1500;
    }
  }

  else if (changeCodeStep == 2) {
    strcpy(newCode, enteredCode);

    printf("Enter new code again\n");

    changeCodeStep = 3;

    digitCount = 0;
    enteredCode[0] = '\0';
    tm1637_clear();
  }

  else if (changeCodeStep == 3) {
    if (strcmp(enteredCode, newCode) == 0) {
      strcpy(code, newCode);

      printf("Code changed successfully!\n");

      changeCodeStep = 0;
      startBlock = tickCount;

      GPIOA->ODR &= ~(1 << 5);
      GPIOA->ODR |= (1 << 9);

      block = 3000;
    } else {
      printf("Codes do not match :(\n");

      changeCodeStep = 0;
      startBlock = tickCount;

      GPIOA->ODR &= ~(1 << 5);
      GPIOA->ODR |= (1 << 15);

      block = 1500;
    }
  }
}



void scanKeyboard() {
  if (block != 0) {
    if (tickCount - startBlock < block) {
      return;
    }
    block = 0;

    tm1637_clear();

    enteredCode[0] = '\0';
    digitCount = 0;

    GPIOA->ODR &= ~(1 << 9);
    GPIOA->ODR &= ~(1 << 15);
    GPIOA->ODR |= (1 << 5);
  }
  // Сканируем клавиатуру каждые 100мс
  if (tickCount - lastScanTime > 100) {
    lastScanTime = tickCount;
    char currentKey = readKey();

    if (currentKey != '\0' && currentKey != lastKey) {
      if (currentKey == '*' && digitCount == 0 && changeCodeStep == 0) {
        changeCodeStep = 1;
        printf("Change code: enter current code\n");
      }

      else if (currentKey >= '0' && currentKey <= '9') {
        if (digitCount < 4) {
          enteredCode[digitCount] = currentKey;
          digitCount++;
          enteredCode[digitCount] = '\0';
          printf("Code: %s\n", enteredCode);

          displayCode();

          if (digitCount == 4) {
            if (changeCodeStep == 0) {
              checkCode();
            } else {
              changeCode();
            }
          }
        }
      }
      lastKey = currentKey;
    } else if (currentKey == '\0') {
      lastKey = '\0';
    }
  }
}