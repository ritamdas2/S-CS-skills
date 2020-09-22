#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"


#define BLINK_GPIO_1 13
#define BLINK_GPIO_2 12
#define BLINK_GPIO_3 15
#define BLINK_GPIO_4 14

char *decimal_to_binary(int n)
{
   int c, dec, count;
   char *pointer;

   count = 0;
   pointer = (char*)malloc(4+1);

   if (pointer == NULL)
      exit(EXIT_FAILURE);
   for (c = 3 ; c >= 0 ; c--)
   {
      dec = n >> c;
      if (dec & 1)
         *(pointer+count) = 1 + '0';
      else
         *(pointer+count) = 0 + '0';
      count++;
   }
   *(pointer+count) = '\0';
   return pointer;
}

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_reset_pin(BLINK_GPIO_1);
    gpio_reset_pin(BLINK_GPIO_2);
    gpio_reset_pin(BLINK_GPIO_3);
    gpio_reset_pin(BLINK_GPIO_4);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_3, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO_4, GPIO_MODE_OUTPUT);
    
    int i;
    char *binaryNum;
    while(1){
      for (i=0;i<=15;i++){
        binaryNum = decimal_to_binary(i);
        gpio_set_level(BLINK_GPIO_1, binaryNum[0]-'0');
        gpio_set_level(BLINK_GPIO_2, binaryNum[1]-'0');
        gpio_set_level(BLINK_GPIO_3, binaryNum[2]-'0');
        gpio_set_level(BLINK_GPIO_4, binaryNum[3]-'0');
        free(binaryNum);
        vTaskDelay(1000 / portTICK_RATE_MS);
      }
    }
}
