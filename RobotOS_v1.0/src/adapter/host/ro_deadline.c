/* ro_deadline.c — Host Deadline Backend (reuses same logic — no Zephyr deps) */
#include <robotos/ro_deadline.h>
#include <robotos/ro_time.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    bool registered; uint16_t id; const char* name;
    uint32_t budget_us; uint64_t _start_us; volatile uint32_t miss_count;
} slot_t;

static slot_t s_tbl[RO_DEADLINE_MAX_IDS];

static slot_t* find(uint16_t id) {
    for(int i=0;i<RO_DEADLINE_MAX_IDS;i++)
        if(s_tbl[i].registered && s_tbl[i].id==id) return &s_tbl[i];
    return NULL;
}

ro_status_t ro_deadline_register(uint16_t id,const char* name,uint32_t budget_us) {
    if(find(id)) return RO_EINVAL;
    for(int i=0;i<RO_DEADLINE_MAX_IDS;i++) if(!s_tbl[i].registered) {
        s_tbl[i]=(slot_t){.registered=true,.id=id,.name=name,.budget_us=budget_us};
        return RO_OK;
    }
    return RO_ENOMEM;
}

void ro_deadline_begin(uint16_t id) {
    slot_t* s=find(id); if(s) s->_start_us=ro_time_us();
}

void ro_deadline_end(uint16_t id) {
    slot_t* s=find(id);
    if(s && s->_start_us>0) {
        if(ro_time_us()-s->_start_us > s->budget_us) s->miss_count++;
        s->_start_us=0;
    }
}

uint32_t    ro_deadline_miss_count(uint16_t id) { slot_t* s=find(id); return s?s->miss_count:0; }
void        ro_deadline_miss_reset(uint16_t id) { slot_t* s=find(id); if(s) s->miss_count=0; }
const char* ro_deadline_get_name(uint16_t id)   { slot_t* s=find(id); return s?s->name:NULL; }
uint32_t    ro_deadline_get_budget(uint16_t id) { slot_t* s=find(id); return s?s->budget_us:0; }
