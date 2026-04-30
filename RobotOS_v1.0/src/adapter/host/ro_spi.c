/* ro_spi.c — Host SPI Stub */
#include <robotos/ro_spi.h>
#include <string.h>

typedef struct ro_spi_bus { bool in_use; ro_spi_config_t cfg; } ro_spi_slot_t;
static ro_spi_slot_t s_pool[4];

ro_spi_bus_t* ro_spi_get(const ro_spi_config_t* c) {
    if(!c) return NULL;
    for(int i=0;i<4;i++) if(!s_pool[i].in_use) {
        s_pool[i].in_use=true; s_pool[i].cfg=*c;
        return (ro_spi_bus_t*)&s_pool[i];
    }
    return NULL;
}
void ro_spi_put(ro_spi_bus_t* b) { if(b) memset(b,0,sizeof(ro_spi_slot_t)); }
ro_status_t ro_spi_transceive(ro_spi_bus_t* b,const uint8_t* t,uint8_t* r,uint32_t l) {
    (void)b;(void)t;(void)r;(void)l; return RO_OK;
}
