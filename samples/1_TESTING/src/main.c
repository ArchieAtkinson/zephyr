#include <stdint.h>
#include <zephyr/smf.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smf_testing);

/* Forward declaration of state table */
static const struct smf_state demo_states[];

/* List of demo states */
enum demo_state { S0, S1, S2 };

/* User defined object */
struct s_object {
        /* This must be first */
        struct smf_ctx ctx;

        /* Other state specific data add here */
} s_obj;

/* State S0 */
static void s0_entry(void *o)
{
    LOG_INF("%s", __func__);
}
static void s0_run(void *o)
{   
    LOG_INF("%s", __func__);
}
static void s0_exit(void *o)
{
    LOG_INF("%s", __func__);
}

/* State S1 */
static void s1_run(void *o)
{   
    LOG_INF("%s", __func__);
}
static void s1_exit(void *o)
{
    LOG_INF("%s", __func__);
}

/* State S2 */
static void s2_entry(void *o)
{
    LOG_INF("%s", __func__);
}
static void s2_run(void *o)
{
    LOG_INF("%s", __func__);
}

/* Populate state table */
static const struct smf_state demo_states[] = {
	[S0] = SMF_CREATE_STATE(s0_entry, s0_run, s0_exit),
	/* State S1 does not have an entry action */
	[S1] = SMF_CREATE_STATE(NULL, s1_run, s1_exit),
	/* State S2 does not have an exit action */
	[S2] = SMF_CREATE_STATE(s2_entry, s2_run, NULL),
};
static bool s0_2_s1_guard(void *o)
{
    LOG_INF("%s", __func__);
    return true;
};

static bool s0_2_s1_action(void *o)
{
    LOG_INF("%s", __func__);
    return true;
};

typedef enum {
    SMF_EVENT1,
    SMF_EVENT2,
    SMF_EVENT3,
} smf_demo_t;


#define SMF_EVENT(_event_type) #_event_type

#define SMF_EVENT_CREATE(_name, _event_type) \
    struct _event_type _name; \
    _name.header.event_type = #_event_type \

#define SMF_EVENT_SEND(_name) (&_name.header)

struct foo_event{
    struct smf_event_header header;
    int a;
};

const struct smf_transition demo_trans[] = {
    SMF_CREATE_TRANS(&demo_states[S0], &demo_states[S1], SMF_EVENT(foo_event), s0_2_s1_guard, s0_2_s1_action),
};

void main(void)
{   
    SMF_EVENT_CREATE(foo, foo_event);
    foo.a = 5;

    bool run_once = true;
        int32_t ret;
        smf_init_trans(SMF_CTX(&s_obj), demo_trans, 1);
        /* Set initial state */
        smf_set_initial(SMF_CTX(&s_obj), &demo_states[S0]);

        /* Run the state machine */
        while(1) {
                /* State machine terminates if a non-zero value is returned */
                ret = smf_run_state(SMF_CTX(&s_obj));
                if (ret) {
                        /* handle return code and terminate state machine */
                        break;
                }
                if(run_once){
                    smf_process_event(SMF_CTX(&s_obj), SMF_EVENT_SEND(foo));
                    run_once = false;
                }
                
                k_msleep(1000);
        }
}



    