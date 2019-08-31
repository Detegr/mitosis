#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <nrf.h>
#include "mitosis-aes-ecb.h"

bool mitosis_aes_ecb_init(mitosis_aes_ecb_context_t* state) {
    if(state == NULL) {
        return false;
    }

    NRF_ECB->ECBDATAPTR = (uint32_t) state;
    return true;
}

bool mitosis_aes_ecb_encrypt(mitosis_aes_ecb_context_t* state) {
    if(state == NULL) {
        return false;
    }

    if(NRF_ECB->ECBDATAPTR != (uint32_t) state) {
        NRF_ECB->ECBDATAPTR = (uint32_t) state;
    }

    uint32_t wait_counter = 0x1000000;

    NRF_ECB->EVENTS_ERRORECB = 0;
    NRF_ECB->EVENTS_ENDECB = 0;
    NRF_ECB->TASKS_STARTECB = 1;
    while(!(NRF_ECB->EVENTS_ENDECB | NRF_ECB->EVENTS_ERRORECB))
    {
        wait_counter--;
        if(wait_counter == 0)
        {
            return false;
        }
    }
    NRF_ECB->EVENTS_ENDECB = 0;
    if(NRF_ECB->EVENTS_ERRORECB) {
        NRF_ECB->EVENTS_ERRORECB = 0;
        return false;
    }
    return true;
}
