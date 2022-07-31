/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * This example shows that function event_loop_wait() will last until it is interupted or all of the events have been handled.
 * This loop may last for ever.
 *
 * Lines 37-39: create 3 time events which are scheduled to be triggered 1, 2 and 3 seconds from current time.
 *
 * NOTE: If tevent_loop_wait() is replaced with tevent_loop_once() in this case, the program will process just 1 time event
 * and the other two will remain undone.
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
    int cnt = 1;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    printf("Create time event...\n");

    //create 3 time events - all for the current time
    tim1 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_current(), handler, &cnt);
    tim2 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_current(), handler, &cnt);
    tim3 = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_current(), handler, &cnt);
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
