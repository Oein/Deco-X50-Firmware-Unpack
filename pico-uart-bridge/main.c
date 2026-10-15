/*
 * USB CDC <-> UART bridge for the RP2040 using PIO, so any GPIO can be used.
 *
 *   GP5 (TX) -> voltage divider -> Deco RX
 *   GP6 (RX) <- Deco TX (1.8 V)
 *
 * The baud rate follows the host's line coding (default 115200 8N1).
 */
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "tusb.h"
#include "uart.pio.h"

#define PIN_TX       5
#define PIN_RX       6
#define DEFAULT_BAUD 115200

static PIO const pio = pio0;
static uint sm_tx, sm_rx;
static volatile uint32_t pending_baud;

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *coding)
{
	(void)itf;
	if (coding->bit_rate)
		pending_baud = coding->bit_rate;
}

static void set_baud(uint32_t baud)
{
	float div = uart_pio_clkdiv(baud);

	pio_sm_set_clkdiv(pio, sm_tx, div);
	pio_sm_set_clkdiv(pio, sm_rx, div);
	pio_sm_clkdiv_restart(pio, sm_tx);
	pio_sm_clkdiv_restart(pio, sm_rx);
}

int main(void)
{
	uint8_t buf[64];

	sm_tx = pio_claim_unused_sm(pio, true);
	sm_rx = pio_claim_unused_sm(pio, true);
	uart_tx_program_init(pio, sm_tx, pio_add_program(pio, &uart_tx_program),
			     PIN_TX, DEFAULT_BAUD);
	uart_rx_program_init(pio, sm_rx, pio_add_program(pio, &uart_rx_program),
			     PIN_RX, DEFAULT_BAUD);

	tusb_init();

	for (;;) {
		uint32_t n = 0;

		tud_task();

		if (pending_baud) {
			set_baud(pending_baud);
			pending_baud = 0;
		}

		/* UART -> USB; bytes are dropped if no host is reading */
		while (n < sizeof(buf) && !pio_sm_is_rx_fifo_empty(pio, sm_rx))
			buf[n++] = (uint8_t)(pio_sm_get(pio, sm_rx) >> 24);
		if (n && tud_cdc_connected()) {
			tud_cdc_write(buf, n);
			tud_cdc_write_flush();
		}

		/* USB -> UART */
		while (tud_cdc_available() && !pio_sm_is_tx_fifo_full(pio, sm_tx)) {
			uint8_t c;

			tud_cdc_read(&c, 1);
			pio_sm_put(pio, sm_tx, c);
		}
	}
}
