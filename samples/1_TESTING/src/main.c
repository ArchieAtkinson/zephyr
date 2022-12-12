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
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S2]);
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
    smf_set_state(SMF_CTX(&s_obj), &demo_states[S0]);
}

/* Populate state table */
static const struct smf_state demo_states[] = {
	[S0] = SMF_CREATE_STATE(s0_entry, s0_run, s0_exit),
	/* State S1 does not have an entry action */
	[S1] = SMF_CREATE_STATE(NULL, s1_run, s1_exit),
	/* State S2 does not have an exit action */
	[S2] = SMF_CREATE_STATE(s2_entry, s2_run, NULL),
};





static bool s1_2_s2_guard(void *o)
{
    LOG_INF("%s", __func__);
    return true;
};

static bool s1_2_s2_action(void *o)
{
    LOG_INF("%s", __func__);
    return true;
};


struct smf_transition demo_trans[] = {{
	.current_state = &demo_states[S0],
	.future_state = &demo_states[S1],
	.event = SMF_EVENT1,
	.guard = s1_2_s2_guard,
	.action = s1_2_s2_action,
}};

void smf_init(struct smf_ctx * ctx, struct smf_transition *trans_table, int32_t table_size)
{
    ctx->transition_table = trans_table;
    ctx->table_size = table_size;
}

void smf_process_event(struct smf_ctx * ctx, smf_event_t event)
{
    __ASSERT_NO_MSG(event < FINAL && event >= 0);
    int i;
    if (ctx->transition_table) {
        for (i = 0; i < ctx->table_size; i++) {
            bool current_state_check = ctx->transition_table->current_state == ctx->current;
            bool event_check = event == ctx->transition_table->event;
            bool guard = ctx->transition_table->guard(ctx);
            if (current_state_check && event_check && guard) {
                smf_set_state(SMF_CTX(&s_obj), ctx->transition_table->future_state);
            }
        }
    }
}

void main(void)
{
        int32_t ret;
        smf_init(SMF_CTX(&s_obj), demo_trans, 1);
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
                smf_process_event(SMF_CTX(&s_obj), SMF_EVENT1);
                k_msleep(1000);
        }
}