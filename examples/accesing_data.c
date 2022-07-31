/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * This example shows the difference between data passed with tevent requests. Callback private data and data stored
 * within the request itself.
 *
 * Lines 35-45:     show the different access of data
 * Lines 38, 41:    2 different possibilites for accesing the very same data
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <tevent.h>

struct foo_state
{
    int x;
};

struct testA
{
    int y;
};

static void foo_done(struct tevent_req *req)
{
    // a->x contains 9
    struct foo_state *a = tevent_req_data(req, struct foo_state);

    // b->y contains 10
    struct testA *b = tevent_req_callback_data(req, struct testA);

    // c->y contains 10
    struct testA *c = (struct testA *)tevent_req_callback_data_void(req);

    printf("a->x: %d\n", a->x);
    printf("b->x: %d\n", b->y);
    printf("c->x: %d\n", c->y);
}

struct tevent_req *foo_send(TALLOC_CTX *mem_ctx, struct tevent_context *event_ctx)
{

    printf("_send\n");
    struct tevent_req *req;
    struct foo_state *state;

    req = tevent_req_create(event_ctx, &state, struct foo_state);
    state->x = 10;

    return req;
}

static void run(struct tevent_context *ev, struct tevent_timer *te, struct timeval current_time, void *private_data)
{

    struct tevent_req *req;
    struct testA *tmp = talloc(ev, struct testA);
    tmp->y = 9;
    req = foo_send(ev, ev);

    tevent_req_set_callback(req, foo_done, tmp);
    tevent_req_done(req);
}

int main(int argc, char **argv)
{

    printf("INIT\n");

    struct tevent_context *event_ctx;
    struct testA *data;
    TALLOC_CTX *mem_ctx;
    struct tevent_timer *time_event;

    mem_ctx = talloc_new(NULL); //parent
    if (mem_ctx == NULL)
        return EXIT_FAILURE;

    event_ctx = tevent_context_init(mem_ctx);
    if (event_ctx == NULL)
        return EXIT_FAILURE;

    data = talloc(mem_ctx, struct testA);
    data->y = 10;

    time_event = tevent_add_timer(event_ctx, mem_ctx, tevent_timeval_current(), run, data);
    if (time_event == NULL)
    {
        fprintf(stderr, " FAILED\n");
        return EXIT_FAILURE;
    }

    tevent_loop_once(event_ctx);

    talloc_free(mem_ctx);

    printf("Quit\n");
    return EXIT_SUCCESS;
}
