/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * This example shows that function event_loop_once() process only one event and then the program continue. In this case it
 * means that it will quit and the two rest time events will remain undone.
 *
 * Lines 40-43: create 3 time events which are scheduled to be triggered 1, 2 and 3 seconds from current time.
 *
 * NOTE: If tevent_loop_once() is replaced with tevent_loop_wait() in this case, the program will last for 3 seconds but it
 * would process all 3 time events.
 */

#include <stdio.h>
#include <tevent.h>

static void handler(struct tevent_context *ev, struct tevent_timer *tim,
                    struct timeval current_time, void *private_data)
{
    int *num = (int *)private_data;
    printf("\thandler was called %d time.\n", *num);
    (*num)++;
}

int main(int argc, char **argv)
{

    printf("INIT\n");

    TALLOC_CTX *mem_ctx;
    struct tevent_context *event_ctx;
    struct tevent_timer *tim1, *tim2, *tim3;
    struct timeval now;
    int cnt = 1;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    printf("Create time event...\n");
    now = tevent_timeval_current();

    // create time event with delay: 1,2 and 3 seconds
    tim1 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_add(&now, 1, 0), handler, &cnt);
    tim2 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_add(&now, 2, 0), handler, &cnt);
    tim3 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_add(&now, 3, 0), handler, &cnt);

    if (tim1 == NULL || tim2 == NULL || tim3 == NULL)
    {
        fprintf(stderr, " FAILED\n");
        return EXIT_FAILURE;
    }

    printf("tevent_loop_wait()\n");
    printf("------------------\n");

    tevent_loop_once(event_ctx);

    talloc_free(mem_ctx);

    printf("------------------\n");
    printf("QUIT\n");

    return EXIT_SUCCESS;
}
