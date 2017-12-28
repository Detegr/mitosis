/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0x40000
  GZP_PARAMS (rw) : ORIGIN = 0x00001000, LENGTH = 0x400
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x4000
}

SECTIONS
{
/* Keep these in-sync with ../../../config/nrf_gzp_config.h */
  GZP_PARAMS_STORAGE_ADR = 0x00001000;
  GZP_PARAMS_DB_ADR 0x00001000:
  {

  } > GZP_PARAMS
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf5x_common.ld"