/* ro_i2c.c — Host I2C Stub */
#include <robotos/ro_i2c.h>
#include <string.h>

typedef struct ro_i2c_bus { bool in_use; } ro_i2c_slot_t;
static ro_i2c_slot_t s_pool[4];

ro_i2c_bus_t* ro_i2c_get(const char* l) {
    if(!l) return NULL;
    for(int i=0;i<4;i++) if(!s_pool[i].in_use) {
        s_pool[i].in_use=true; return (ro_i2c_bus_t*)&s_pool[i];
    }
    return NULL;
}
void ro_i2c_put(ro_i2c_bus_t* b) { if(b) memset(b,0,sizeof(ro_i2c_slot_t)); }
ro_status_t ro_i2c_write(ro_i2c_bus_t* b,uint16_t a,const uint8_t* d,uint32_t l) {
    (void)b;(void)a;(void)d;(void)l; return RO_OK;
}
ro_status_t ro_i2c_read(ro_i2c_bus_t* b,uint16_t a,uint8_t* d,uint32_t l) {
    (void)b;(void)a;(void)d;(void)l; return RO_OK;
}
ro_status_t ro_i2c_write_read(ro_i2c_bus_t* b,uint16_t a,
    const uint8_t* w,uint32_t wl,uint8_t* r,uint32_t rl) {
    (void)b;(void)a;(void)w;(void)wl;(void)r;(void)rl; return RO_OK;
}
