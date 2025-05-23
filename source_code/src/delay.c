#include <delay.h>

static volatile uint32_t clock_ticks;

void clock_tick(void) {
  clock_ticks++;
}

uint32_t read_cycle_counter(void) {
  return dwt_read_cycle_counter();
}

uint32_t get_us_counter(void){
  return dwt_read_cycle_counter() / (SYSCLK_FREQUENCY_HZ / MICROSECONDS_PER_SECOND);
}

void delay(uint32_t ms) {
  uint32_t awake = clock_ticks + ms;

  while (awake > clock_ticks) {
  };
}

uint32_t get_clock_ticks(void) {
  return clock_ticks;
}

void delay_us(uint32_t us) {
  uint32_t initial_cycles = dwt_read_cycle_counter();
  uint32_t sleep_cycles = (uint32_t)(SYSCLK_FREQUENCY_HZ * ((float)us / (float)MICROSECONDS_PER_SECOND));

  while (dwt_read_cycle_counter() - initial_cycles <= sleep_cycles) {
  };
}