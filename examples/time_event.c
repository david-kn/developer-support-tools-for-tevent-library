/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Example which sets time events for every 2 seconds until a total end time is reached (10 seconds)
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <tevent.h>
#include <sys/time.h>

struct foo_state
{
    struct timeval endtime;
    int counter;
    TALLOC_CTX *ctx;
};

static void foo(struct tevent_context *ev, struct tevent_timer *tim,
                struct timeval current_time, void *private_data)
{

    printf("--------\n");
    printf("Callback\n");
    struct foo_state *data = talloc_get_type(private_data, struct foo_state);
    struct tevent_timer *time_event;
    struct timeval schedule;
    printf("Data value: %d\n", data->counter);

    data->counter += 1; // increase counter

    // if time has not reached its limit, set another event
    if (tevent_timeval_compare(&current_time, &(data->endtime)) < 0)
    {
        schedule = tevent_timeval_current_ofs(2, 0);
        time_event = tevent_add_timer(ev, data->ctx, schedule, foo, data);
        if (time_event == NULL)
        {
            fprintf(stderr, "MEMORY PROBLEM\n");
            return;
        }
    }
}

int main(void)
{

    printf("INIT\n");

    struct tevent_context *event_ctx;
    TALLOC_CTX *mem_ctx;
    struct tevent_timer *time_event;
    struct timeval schedule;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    struct foo_state *data = talloc(mem_ctx, struct foo_state);

    schedule = tevent_timeval_current_ofs(2, 0);          // +2 second time value
    data->endtime = tevent_timeval_add(&schedule, 60, 0); // one minute time limit
    data->ctx = mem_ctx;
    data->counter = 0;

    // add time event
    time_event = tevent_add_timer(event_ctx, mem_ctx, schedule, foo, data);
    if (time_event == NULL)
    {
        fprintf(stderr, "FAILED\n");
        return EXIT_FAILURE;
    }

    tevent_loop_wait(event_ctx);
    talloc_free(mem_ctx);

    printf("Quit.\n");
    return EXIT_SUCCESS;
}
