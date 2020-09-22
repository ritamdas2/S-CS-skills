//Author: Ritam Das

#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINK_GPIO 13


void app_main()
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    //char str[20];
    int num = 0;
    int counter = 0;
    int lednum = 0;

    
    while(1) {
      char buf[5];
      gets(buf);
      if (buf[0] == 's') {
        counter++;
        if (counter % 3 == 0) {
          printf("Switched to Toggle LED Mode!\n");
        }
        if (counter % 3 == 1) {
          printf("Switched to Echo Input Mode!\n");
        }
        if (counter % 3 == 2) {
          printf("Switched to Echo Decimal to Hexadecimal Mode!\n");
        }
    }

    if (counter%3 == 0) {
      // toggle LED mode
      printf("Read: ");
      if (buf[0] != '\0') {
       printf("%s\n", buf);
      }
      if (buf[0] == 't') {
        // add the blinking here
        if (lednum == 0) {
          gpio_set_level(BLINK_GPIO,1);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          lednum = 1;
        }
        else if (lednum == 1) {
          gpio_set_level(BLINK_GPIO,0);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          lednum = 0;
        }
      }
    }

    if (counter%3 == 1) {
      printf("echo: ");
      // gets()
      if (buf[0] != '\0') {
       printf("%s\n", buf);
      }
    }

    if (counter%3 == 2) {
      printf("Enter an integer:\n");
      //gets();
      num = atoi(buf);
      printf("Hex: %X\n", num);
    }
      vTaskDelay(50 / portTICK_RATE_MS);
    }
}
