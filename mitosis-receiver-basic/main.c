
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "nrf_drv_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "nrf_gzp.h"
#include "nrf_gzll.h"

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1                           /**< UART RX buffer size. */


#define RX_PIN_NUMBER  25
#define TX_PIN_NUMBER  24
#define CTS_PIN_NUMBER 23
#define RTS_PIN_NUMBER 22
#define HWFC           false


// Define payload length
#define TX_PAYLOAD_LENGTH 4 ///< 4 byte payload length

// ticks for inactive keyboard
#define INACTIVE 100000

// Binary printing
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x10 ? '#' : '.'), \
  (byte & 0x08 ? '#' : '.'), \
  (byte & 0x04 ? '#' : '.'), \
  (byte & 0x02 ? '#' : '.'), \
  (byte & 0x01 ? '#' : '.') 


// Data and acknowledgement payloads
static uint8_t data_payload[NRF_GZLL_CONST_MAX_PAYLOAD_LENGTH];  ///< Placeholder for data payload received from host.
static uint8_t data_buffer[10];

// Debug helper variables
extern nrf_gzll_error_code_t nrf_gzll_error_code;   ///< Error code
static bool init_ok, enable_ok, push_ok, pop_ok, packet_received_left, packet_received_right;
uint32_t left_active = 0;
uint32_t right_active = 0;
uint8_t c;


void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}


int main(void)
{
    uint32_t err_code, length;
    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          UART_BAUDRATE_BAUDRATE_Baud1M
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_error_handle,
                         APP_IRQ_PRIORITY_LOW,
                         err_code);

    APP_ERROR_CHECK(err_code);

    // Initialize Gazell
    nrf_gzll_init(NRF_GZLL_MODE_HOST);

    // Initialize Gazell Pairing library
    gzp_init();
    gzp_pairing_enable(true);
  
    // Enable Gazell to start sending over the air
    nrf_gzll_enable();

    // main loop
    while (true)
    {
        gzp_host_execute();

        if (gzp_id_req_received())
        {
            // Accept all pairing requests received.
            // The pairing library's proximity detection will (help) prevent
            // unwanted devices from pairing.
            gzp_id_req_grant();
        }

        if (gzp_crypt_user_data_received())
        {
            if (gzp_crypt_user_data_read(data_payload, (uint8_t*) &length))
            {
                // unpacking received packet
                if (data_payload[0]) // left-hand
                {
                    data_buffer[0] = ((data_payload[1] & 1<<3) ? 1:0) << 0 |
                                     ((data_payload[1] & 1<<4) ? 1:0) << 1 |
                                     ((data_payload[1] & 1<<5) ? 1:0) << 2 |
                                     ((data_payload[1] & 1<<6) ? 1:0) << 3 |
                                     ((data_payload[1] & 1<<7) ? 1:0) << 4;

                    data_buffer[2] = ((data_payload[2] & 1<<6) ? 1:0) << 0 |
                                     ((data_payload[2] & 1<<7) ? 1:0) << 1 |
                                     ((data_payload[1] & 1<<0) ? 1:0) << 2 |
                                     ((data_payload[1] & 1<<1) ? 1:0) << 3 |
                                     ((data_payload[1] & 1<<2) ? 1:0) << 4;

                    data_buffer[4] = ((data_payload[2] & 1<<1) ? 1:0) << 0 |
                                     ((data_payload[2] & 1<<2) ? 1:0) << 1 |
                                     ((data_payload[2] & 1<<3) ? 1:0) << 2 |
                                     ((data_payload[2] & 1<<4) ? 1:0) << 3 |
                                     ((data_payload[2] & 1<<5) ? 1:0) << 4;

                    data_buffer[6] = ((data_payload[3] & 1<<5) ? 1:0) << 1 |
                                     ((data_payload[3] & 1<<6) ? 1:0) << 2 |
                                     ((data_payload[3] & 1<<7) ? 1:0) << 3 |
                                     ((data_payload[2] & 1<<0) ? 1:0) << 4;

                    data_buffer[8] = ((data_payload[3] & 1<<1) ? 1:0) << 1 |
                                     ((data_payload[3] & 1<<2) ? 1:0) << 2 |
                                     ((data_payload[3] & 1<<3) ? 1:0) << 3 |
                                     ((data_payload[3] & 1<<4) ? 1:0) << 4;
                }
                else // right-hand
                {
                    data_buffer[1] = ((data_payload[1] & 1<<7) ? 1:0) << 0 |
                                     ((data_payload[1] & 1<<6) ? 1:0) << 1 |
                                     ((data_payload[1] & 1<<5) ? 1:0) << 2 |
                                     ((data_payload[1] & 1<<4) ? 1:0) << 3 |
                                     ((data_payload[1] & 1<<3) ? 1:0) << 4;

                    data_buffer[3] = ((data_payload[1] & 1<<2) ? 1:0) << 0 |
                                     ((data_payload[1] & 1<<1) ? 1:0) << 1 |
                                     ((data_payload[1] & 1<<0) ? 1:0) << 2 |
                                     ((data_payload[2] & 1<<7) ? 1:0) << 3 |
                                     ((data_payload[2] & 1<<6) ? 1:0) << 4;

                    data_buffer[5] = ((data_payload[2] & 1<<5) ? 1:0) << 0 |
                                     ((data_payload[2] & 1<<4) ? 1:0) << 1 |
                                     ((data_payload[2] & 1<<3) ? 1:0) << 2 |
                                     ((data_payload[2] & 1<<2) ? 1:0) << 3 |
                                     ((data_payload[2] & 1<<1) ? 1:0) << 4;

                    data_buffer[7] = ((data_payload[2] & 1<<0) ? 1:0) << 0 |
                                     ((data_payload[3] & 1<<7) ? 1:0) << 1 |
                                     ((data_payload[3] & 1<<6) ? 1:0) << 2 |
                                     ((data_payload[3] & 1<<5) ? 1:0) << 3;

                    data_buffer[9] = ((data_payload[3] & 1<<4) ? 1:0) << 0 |
                                     ((data_payload[3] & 1<<3) ? 1:0) << 1 |
                                     ((data_payload[3] & 1<<2) ? 1:0) << 2 |
                                     ((data_payload[3] & 1<<1) ? 1:0) << 3;
                }
            }
        }

        // checking for a poll request from QMK
        if (app_uart_get(&c) == NRF_SUCCESS && c == 's')
        {
            // sending data to QMK, and an end byte
            nrf_drv_uart_tx(data_buffer,10);
            app_uart_put(0xE0);

            // debugging help, for printing keystates to a serial console
            /*
            for (uint8_t i = 0; i < 10; i++)
            {
                app_uart_put(data_buffer[i]);
            }
            printf(BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN " " \
                   BYTE_TO_BINARY_PATTERN "\r\n", \
                   BYTE_TO_BINARY(data_buffer[0]), \
                   BYTE_TO_BINARY(data_buffer[1]), \
                   BYTE_TO_BINARY(data_buffer[2]), \
                   BYTE_TO_BINARY(data_buffer[3]), \
                   BYTE_TO_BINARY(data_buffer[4]), \
                   BYTE_TO_BINARY(data_buffer[5]), \
                   BYTE_TO_BINARY(data_buffer[6]), \
                   BYTE_TO_BINARY(data_buffer[7]), \
                   BYTE_TO_BINARY(data_buffer[8]), \
                   BYTE_TO_BINARY(data_buffer[9]));   
            nrf_delay_us(100);
            */
        }
        // allowing UART buffers to clear
        nrf_delay_us(10);
        
        // if no packets recieved from keyboards in a few seconds, assume either
        // out of range, or sleeping due to no keys pressed, update keystates to off
        left_active++;
        right_active++;
        if (left_active > INACTIVE)
        {
            data_buffer[0] = 0;
            data_buffer[2] = 0;
            data_buffer[4] = 0;
            data_buffer[6] = 0;
            data_buffer[8] = 0;
            left_active = 0;
        }
        if (right_active > INACTIVE)
        {
            data_buffer[1] = 0;
            data_buffer[3] = 0;
            data_buffer[5] = 0;
            data_buffer[7] = 0;
            data_buffer[9] = 0;
            right_active = 0;
        }
    }
}
