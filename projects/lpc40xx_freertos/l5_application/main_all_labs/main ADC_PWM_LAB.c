// #include "FreeRTOS.h"
// #include "semphr.h"
// #include <stdio.h>

// #include "board_io.h"
// #include "common_macros.h"
// #include "gpio.h"
// #include "gpio_lab.h"
// #include "periodic_scheduler.h"
// #include "sj2_cli.h"
// #include "task.h"

// #include "lpc40xx.h"
// #include "lpc_peripherals.h"
// #include "pwm1.h"
// static SemaphoreHandle_t switch_pressed_signal;
// void sleep_on_sem_task(void *p);
// void configure_your_gpio_interrupt();
// void switch_task(void *task_parameter);
// void gpio_interrupt(void);
// void lab_isr_part2();
// /*========================== LAB_PWM+ADC_PART_0 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// void lab_pwm_part0();
// void pwm_task(void *p);
// void pin_configure_pwm_channel_as_io_pin(uint32_t port_num, uint8_t pin_num, gpio__function_e function);

// /*========================== LAB_PWM+ADC_PART_1 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// #include "adc.h"
// void lab_pwm_part1();
// void pin_configure_adc_channel_as_io_pin();
// /*========================== LAB_PWM+ADC_PART_2 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// #include "queue.h"
// void lab_pwm_part2();
// static QueueHandle_t adc_to_pwm_task_queue;

// void adc_task(void *p) { // adc task is PRODUCER
//   // NOTE: Reuse the code from Part 1
//   adc__initialize();
//   pin_configure_adc_channel_as_io_pin();
//   adc__enable_burst_mode(ADC__CHANNEL_2);
//   int adc_reading = 0; // Note that this 'adc_reading' is not the same variable as the one from adc_task
//   while (1) {
//     // Implement code to send potentiometer value on the queue
//     // a) read ADC input to 'int adc_reading'
//     // b) Send to queue: xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
//     adc_reading = adc__get_channel_reading_with_burst_mode();
//     printf("adc value: %x\n", adc_reading);
//     xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
//     puts("send\n");
//     vTaskDelay(2000);
//   }
// }

// void pwm_task_part2(void *p) { // pwm task is CONSUMER
//   // NOTE: Reuse the code from Part 0
//   pwm1__init_single_edge(1000);

//   pin_configure_pwm_channel_as_io_pin(2, 0, GPIO__FUNCTION_1);
//   pin_configure_pwm_channel_as_io_pin(2, 1, GPIO__FUNCTION_1);
//   pin_configure_pwm_channel_as_io_pin(2, 2, GPIO__FUNCTION_1);
//   int adc_reading = 0;
//   int percent = 0;
//   while (1) {
//     // Implement code to receive potentiometer value from queue
//     int mr0 = LPC_PWM1->MR0;
//     int mr1 = LPC_PWM1->MR1;
//     printf("MR0 is: %d\nMR1 is: %d\n", mr0, mr1);
//     if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, portMAX_DELAY)) {
//       puts("received\n");
//       printf("adc value in queue: %d\n", adc_reading);
//       percent = (double)adc_reading / 4095.0 * 100;
//       printf("percent: %d\n", percent);
//       pwm1__set_duty_cycle(PWM1__2_0, percent - 50);
//       pwm1__set_duty_cycle(PWM1__2_1, percent - 25);
//       pwm1__set_duty_cycle(PWM1__2_2, percent);

//       pwm1__set_duty_cycle(PWM1__2_0, percent - 75);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_2, percent);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_0, percent - 75);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_1, percent);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_2, percent - 75);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_1, percent - 75);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_0, percent);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_0, percent);
//       vTaskDelay(250);
//       pwm1__set_duty_cycle(PWM1__2_0, percent);
//       pwm1__set_duty_cycle(PWM1__2_1, percent);
//       pwm1__set_duty_cycle(PWM1__2_2, percent);
//       vTaskDelay(1000);
//       if (percent > 100) {
//         percent = 0;
//       }
//       percent += 5;
//       vTaskDelay(250);
//     }
//     // We do not need task delay because our queue API will put task to sleep when there is no data in the queue
//     // vTaskDelay(100);
//   }
// }

// int main(void) {

//   // // lab_isr_part2();
//   // // lab_pwm_part0();
//   // lab_pwm_part1();
//   lab_pwm_part2();
// }
// /*========================== LAB_PWM+ADC_PART_2 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// void lab_pwm_part2() {
//   adc_to_pwm_task_queue = xQueueCreate(1, sizeof(int));

//   xTaskCreate(adc_task, "adc_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
//   xTaskCreate(pwm_task_part2, "pmw_task", 1024 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
//   vTaskStartScheduler();
// }

// /*========================== LAB_PWM+ADC_PART_1 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// void lab_pwm_part1() {

//   adc__initialize();
//   // adc__enable_burst_mode(2);
//   fprintf(stderr, "Step1 \n");
//   LPC_IOCON->P0_25 &= ~(1 << 7);                                // set bit 7 to 0 to enable ANALOG function
//   pin_configure_pwm_channel_as_io_pin(0, 25, GPIO__FUNCTION_1); // set pin P0__25 to function AD MODE

//   fprintf(stderr, "Step2 \n");
//   adc__enable_burst_mode(ADC__CHANNEL_2);
//   fprintf(stderr, "Step3 \n");

//   fprintf(stderr, "Step5 \n");
//   while (1) {
//     double adc_value = (double)adc__get_channel_reading_with_burst_mode() / 4095.0 * 3.3;
//     printf("Volatage: %.2f\n", adc_value);
//     delay__ms(500);
//   }
// }
// void pin_configure_adc_channel_as_io_pin() {
//   LPC_IOCON->P0_25 &= ~(1 << 7);                                // set bit 7 to 0 to enable ANALOG function
//   pin_configure_pwm_channel_as_io_pin(0, 25, GPIO__FUNCTION_1); // set pin P0__25 to function AD MODE
// }
// /*========================== LAB_PWM+ADC_PART_0 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// void lab_pwm_part0() {
//   xTaskCreate(pwm_task, "pwm_task", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   vTaskStartScheduler();
// }
// void pwm_task(void *p) {
//   pwm1__init_single_edge(1000);

//   pin_configure_pwm_channel_as_io_pin(2, 0, GPIO__FUNCTION_1);
//   pin_configure_pwm_channel_as_io_pin(2, 1, GPIO__FUNCTION_1);
//   pin_configure_pwm_channel_as_io_pin(2, 2, GPIO__FUNCTION_1);

//   uint8_t percent = 0;
//   while (1) {
//     pwm1__set_duty_cycle(PWM1__2_0, percent - 75);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_2, percent);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_0, percent - 75);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_1, percent);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_2, percent - 75);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_1, percent - 75);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_0, percent);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_0, percent);
//     vTaskDelay(250);
//     pwm1__set_duty_cycle(PWM1__2_0, percent);
//     pwm1__set_duty_cycle(PWM1__2_1, percent);
//     pwm1__set_duty_cycle(PWM1__2_2, percent);
//     vTaskDelay(1000);
//     if (percent > 100) {
//       percent = 0;
//     }
//     percent += 5;
//     vTaskDelay(250);
//   }
// }
// // this api can be used for pwm and adc lab to configure the pin function
// void pin_configure_pwm_channel_as_io_pin(uint32_t port_num, uint8_t pin_num, gpio__function_e function) {
//   gpio__construct_with_function(port_num, pin_num, function);
// }

// /*========================== LAB_ISR_PART_2 =====================
// *
// *
// *
// *
// *
// *
// *
// =================================================================*/
// void lab_isr_part2() {
//   switch_pressed_signal = xSemaphoreCreateBinary(); // Create your binary semaphore
//   configure_your_gpio_interrupt();
//   NVIC_EnableIRQ(GPIO_IRQn); // Enable interrupt gate for the GPIO
//   lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio_interrupt, NULL);
//   xTaskCreate(sleep_on_sem_task, "sem", (512U * 4) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
//   vTaskStartScheduler();
// }

// void configure_your_gpio_interrupt() {
//   GPIO__set_as_input(0, 30);
//   LPC_IOCON->P0_30 |= (1 << 3); // 2nd way to pull down register
//   LPC_IOCON->P0_30 &= ~(1 << 4);
//   LPC_GPIOINT->IO0IntEnF |= (1 << 30);
// }
// void gpio_interrupt(void) {
//   fprintf(stderr, "hello isr\n");
//   xSemaphoreGiveFromISR(switch_pressed_signal, NULL);
//   LPC_GPIOINT->IO0IntClr |= (1 << 30);
//   fprintf(stderr, "gave semaphore\n");
// }

// void sleep_on_sem_task(void *p) {
//   while (1) {
//     // Use xSemaphoreTake with forever delay and blink an LED when you get the signal
//     if (xSemaphoreTake(switch_pressed_signal, 1000)) {
//       fprintf(stderr, "Took semaphore\n");
//       GPIO__set_high(1, 18);
//       vTaskDelay(100);
//       GPIO__set_low(1, 18);
//       vTaskDelay(100);
//     } else {
//       puts("sleeping...\n");
//     }
//   }
// }
