/* ro_pwm.c — Host PWM Stub */
#include <robotos/ro_pwm.h>
#include <string.h>

#define HOST_PWM_POOL 8
typedef struct ro_pwm { bool in_use; ro_pwm_config_t cfg; uint32_t pulse; } ro_pwm_slot_t;
static ro_pwm_slot_t s_pool[HOST_PWM_POOL];

ro_pwm_t* ro_pwm_get(const ro_pwm_config_t* c) {
    if(!c) return NULL;
    for(int i=0;i<HOST_PWM_POOL;i++) if(!s_pool[i].in_use) {
        memset(&s_pool[i],0,sizeof(s_pool[i]));
        s_pool[i].in_use=true; s_pool[i].cfg=*c;
        return (ro_pwm_t*)&s_pool[i];
    }
    return NULL;
}
void ro_pwm_put(ro_pwm_t* p) { if(p) memset(p,0,sizeof(ro_pwm_slot_t)); }
ro_status_t ro_pwm_set_pulse(ro_pwm_t* p,uint32_t ns) {
    if(!p) return RO_EINVAL; ((ro_pwm_slot_t*)p)->pulse=ns; return RO_OK;
}
ro_status_t ro_pwm_set_period(ro_pwm_t* p,uint32_t per,uint32_t pul) {
    if(!p) return RO_EINVAL;
    ((ro_pwm_slot_t*)p)->cfg.period_ns=per; ((ro_pwm_slot_t*)p)->pulse=pul;
    return RO_OK;
}
ro_status_t ro_pwm_disable(ro_pwm_t* p) { return ro_pwm_set_pulse(p,0); }
