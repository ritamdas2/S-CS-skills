/* Infrared IR/UART beacons for crawler capture the flag!
   November 2019 -- Emily Lam

   RMT Pulse          -- pin 26 -- A0
   UART Transmitter   -- pin 25 -- A1
   UART Receiver      -- pin 34 -- A2

   Hardware interrupt (button to change color)  -- pin 4  -- A5
   Button to send to information to other ESP   -- pin 27 -- A10
   ID Indicator       -- pin 13 -- Onboard LED

   Red LED            -- pin 15
   Green LED          -- pin 32
   Blue LED           -- Pin 14

   Features:
   - Sends UART payload -- | START | myColor | myID | Checksum? |
   - Outputs 38kHz using RMT for IR transmission
   - Onboard LED blinks device ID (myID)
   - Button press to change device ID
   - RGB LED shows traffic light state (red, green, yellow)
   - Timer controls traffic light state (r - 10s, g - 10s, y - 2s)
*/

//Ritam Das 11-13-2020

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "soc/uart_reg.h" // Little said on piazza to add this

// RMT definitions
#define RMT_TX_CHANNEL 1                                 // RMT channel for transmitter
#define RMT_TX_GPIO_NUM 25                               // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV 100                                  // RMT counter clock divider
#define RMT_TICK_10_US (80000000 / RMT_CLK_DIV / 100000) // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US 9500                       // RMT receiver timeout value(us)

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1

#define BUTTON_GPIO 27

// LED Output pins definitions
#define BLUEPIN 32
#define GREENPIN 15
#define REDPIN 33
#define ONBOARD 13

// #define TIMER_DIVIDER 16                             //  Hardware timer clock divider
// #define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER) // to seconds
// #define TIMER_INTERVAL_2_SEC (2)
// #define TIMER_INTERVAL_10_SEC (10)
// #define TEST_WITH_RELOAD 1 // Testing will be done with auto reload

// Default ID/color
#define ID 3
#define COLOR 'R'

// Variables for my ID, minVal and status plus string fragments
char start = 0x1B;
char myID = (char)ID;
char myColor = (char)COLOR;
int len_out = 4;

// Mutex (for resources), and Queues (for button)
SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;
// static xQueueHandle timer_queue;

// flag for sending data
int sendFlag = 0;

// System tags
static const char *TAG_SYSTEM = "system"; // For debug logs

// Button 1 interrupt handler -- add to queue
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Utilities ///////////////////////////////////////////////////////////////////

// Checksum
char genCheckSum(char *p, int len)
{
  char temp = 0;
  for (int i = 0; i < len; i++)
  {
    temp = temp ^ p[i];
  }
  // printf("%X\n",temp);

  return temp;
}
bool checkCheckSum(uint8_t *p, int len)
{
  char temp = (char)0;
  bool isValid;
  for (int i = 0; i < len - 1; i++)
  {
    temp = temp ^ p[i];
  }
  // printf("Check: %02X ", temp);
  if (temp == p[len - 1])
  {
    isValid = true;
  }
  else
  {
    isValid = false;
  }
  return isValid;
}

// Init Functions //////////////////////////////////////////////////////////////
// RMT tx init
static void rmt_tx_init()
{
  rmt_config_t rmt_tx;
  rmt_tx.channel = RMT_TX_CHANNEL;
  rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;
  // Carrier Frequency of the IR receiver
  rmt_tx.tx_config.carrier_freq_hz = 38000;
  rmt_tx.tx_config.carrier_level = 1;
  rmt_tx.tx_config.carrier_en = 1;
  // Never idle -> aka ontinuous TX of 38kHz pulses
  rmt_tx.tx_config.idle_level = 1;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = 0;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}

// Configure UART
static void uart_init()
{
  // Basic configs
  uart_config_t uart_config = {
      .baud_rate = 1200, // Slow BAUD rate
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1, UART_SIGNAL_RXD_INV); // ****** changed 2nd arg from UART_INVERSE_RXD

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs
static void led_init()
{
  gpio_pad_select_gpio(BLUEPIN);
  gpio_pad_select_gpio(GREENPIN);
  gpio_pad_select_gpio(REDPIN);
  gpio_pad_select_gpio(ONBOARD);
  gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(ONBOARD, GPIO_MODE_OUTPUT);
}

// Button 1 interrupt init
static void button_init()
{
  gpio_config_t io_conf;
  //interrupt of rising edge
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  //bit mask of the pins, use GPIO4 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);
  gpio_intr_enable(GPIO_INPUT_IO_1);
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *)GPIO_INPUT_IO_1);
  //create a queue to handle gpio event from isr
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  //start gpio task

  // other button init
  gpio_pad_select_gpio(BUTTON_GPIO);
  gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
}

////////////////////////////////////////////////////////////////////////////////

// Tasks ///////////////////////////////////////////////////////////////////////
// Button task -- rotate through myIDs
void button_task()
{
  uint32_t io_num;
  while (1)
  {

    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
    {
      xSemaphoreTake(mux, portMAX_DELAY);
      if (myID == 3)
      {
        myID = 1;
      }
      else
      {
        myID++;
      }

      if (myColor == 'R')
      {
        myColor = 'G';
      }
      else if (myColor == 'G')
      {
        myColor = 'Y';
      }
      else if (myColor == 'Y')
      {
        myColor = 'R';
      }
      xSemaphoreGive(mux);
      printf("Button pressed. LED color: %c\n", myColor);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Button 2 task -- send data through IR
void button_2_task()
{
  int button_flag;
  while (1)
  {
    button_flag = gpio_get_level(BUTTON_GPIO);
    if (!button_flag)
    {
      printf("button 2 pressed\n");
      sendFlag = 1;
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    sendFlag = 0;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}


// Receives task -- looks for Start byte then stores received values
void recv_task()
{
  // Buffer for input data
  uint8_t *data_in = (uint8_t *)malloc(BUF_SIZE);
  while (1)
  {
    if (sendFlag)
    {
      char *data_out = (char *)malloc(len_out);
      xSemaphoreTake(mux, portMAX_DELAY);
      data_out[0] = start;
      data_out[1] = (char)myColor;
      data_out[2] = (char)myID;
      data_out[3] = genCheckSum(data_out, len_out - 1);

      // ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_out, len_out, ESP_LOG_INFO);

      for (int i = 0; i < 5; i++)
      {
        uart_write_bytes(UART_NUM_1, data_out, len_out);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
      sendFlag = 0;
      printf("data sucessfully sent\n");
      xSemaphoreGive(mux);
    }
    else
    {
      int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
      if (len_in > 0)
      {
        if (data_in[0] == start)
        {
          if (checkCheckSum(data_in, len_out))
          {

            ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in, len_out, ESP_LOG_INFO);
            // change led data for current esp
            myColor = (char)data_in[1];
            myID = (char)data_in[2];
            printf("data successfully recieved!\n");
          }
        }
      }
      else
      {
        // printf("Nothing received.\n");
      }
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
  free(data_in);
}


// LED task to light LED based on traffic state
void led_task()
{
  while (1)
  {
    switch ((int)myColor)
    {
    case 'R': // Red
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(REDPIN, 1);
      gpio_set_level(BLUEPIN, 0);
      // printf("Current state: %c\n",status);
      break;
    case 'Y': // Yellow            (actually just blue)
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(REDPIN, 0);
      gpio_set_level(BLUEPIN, 1);
      // printf("Current state: %c\n",status);
      break;
    case 'G': // Green
      gpio_set_level(GREENPIN, 1);
      gpio_set_level(REDPIN, 0);
      gpio_set_level(BLUEPIN, 0);
      // printf("Current state: %c\n",status);
      break;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// LED task to blink onboard LED based on ID
void id_task()
{
  while (1)
  {
    for (int i = 0; i < (int)myID; i++)
    {
      gpio_set_level(ONBOARD, 1);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      gpio_set_level(ONBOARD, 0);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


void app_main()
{

  // Mutex for current values when sending
  mux = xSemaphoreCreateMutex();

  // // Create a FIFO queue for timer-based events
  // timer_queue = xQueueCreate(10, sizeof(timer_event_t));

  // Create task to handle timer-based events
  // xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

  // Initialize all the things
  rmt_tx_init();
  uart_init();
  led_init();
  // alarm_init();
  button_init();

  // Create tasks for receive, send, set gpio, and button
  xTaskCreate(recv_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL);
  //xTaskCreate(send_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(led_task, "set_traffic_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(id_task, "set_id_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(button_task, "button_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
  xTaskCreate(button_2_task, "button_2_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
}
