/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Example of queue - request are put into the queue and they are not triggered before the previous request from the queue is
 * set as done (or freed).
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <tevent.h>

struct computation_state
{
    int local_var;
    int x;
};

struct juststruct
{
    TALLOC_CTX *ctx;
    struct tevent_context *ev;
    int y;
};

int created = 0;

static void timer_handler(struct tevent_context *ev, struct tevent_timer *te,
                          struct timeval current_time, void *private_data)
{
    // time event which after all sets request as done. Following item from the queue  may be invoked.
    struct tevent_req *req = private_data;
    struct computation_state *stateX = tevent_req_data(req, struct computation_state);

    /*
     * Processing some stuff
     */

    printf("time_handler\n");

    tevent_req_done(req);
    talloc_free(req);
    printf("Request #%d set as done.\n", stateX->x);
}

static void trigger(struct tevent_req *req, void *private_data)
{

    struct juststruct *priv = tevent_req_callback_data(req, struct juststruct);
    struct computation_state *in = tevent_req_data(req, struct computation_state);
    struct timeval schedule;
    struct tevent_timer *tim;
    schedule = tevent_timeval_current_ofs(1, 0);
    printf("Processing request #%d\n", in->x);

    if (in->x % 3 == 0)
    { // just example; third request does not contain any further operation and will be finished right away.
        tim = NULL;
    }
    else
    {
        tim = tevent_add_timer(priv->ev, req, schedule, timer_handler, req);
    }

    if (tim == NULL)
    {
        tevent_req_done(req);
        talloc_free(req);
        printf("Request #%d set as done.\n", in->x);
    }
}

struct tevent_req *foo_send(TALLOC_CTX *mem_ctx, struct tevent_context *ev, const char *name, int num)
{

    struct tevent_req *req;
    struct computation_state *state;
    struct computation_state *in;
    struct tevent_timer *tim;

    printf("foo_send\n");
    req = tevent_req_create(mem_ctx, &state, struct computation_state);
    if (!req)
    { // check for appropriate allocation
        tevent_req_error(req, 1);
        return NULL;
    }

    // exemplary filling of variables
    state->local_var = 1;
    state->x = num;

    return req;
}

static void foo_done(struct tevent_req *req)
{

    enum tevent_req_state state;
    uint64_t err;
    if (tevent_req_is_error(req, &state, &err))
    {
        printf("ERROR WAS SET %d\n", state);
        return;
    }
    else
    {
        /*
         * processing some stuff
         */
        printf("Callback is done...\n");
    }
}

int main(int argc, char **argv)
{

    printf("INIT\n");

    TALLOC_CTX *mem_ctx;
    struct tevent_req *req[6];
    struct tevent_req *tmp;
    struct tevent_context *ev;
    struct tevent_queue *fronta = NULL;
    int ret;
    int i = 0;

    const char *const names[] = {
        "first", "second", "third", "fourth", "fifth"};

    mem_ctx = talloc_new(NULL); //parent
    talloc_parent(mem_ctx);
    ev = tevent_context_init(mem_ctx);
    if (ev == NULL)
    {
        fprintf(stderr, "MEMORY ERROR\n");
        return EXIT_FAILURE;
    }

    // setting up queue
    fronta = tevent_queue_create(mem_ctx, "test_queue");
    tevent_queue_stop(fronta);
    tevent_queue_start(fronta);
    if (tevent_queue_running(fronta))
    {
        printf("Queue is runnning (length: %d)\n", tevent_queue_length(fronta));
    }
    else
    {
        printf("Queue is not runnning\n");
    }

    struct juststruct *data = talloc(ev, struct juststruct);
    data->ctx = mem_ctx;
    data->ev = ev;

    // create 4 requests
    for (i = 1; i < 5; i++)
    {
        req[i] = foo_send(mem_ctx, ev, names[i], i);
        tmp = req[i];
        if (req[i] == NULL)
        {
            fprintf(stderr, "CHYBA!! %d \n", ret);
            break;
        }
        tevent_req_set_callback(req[i], foo_done, data);
        created++;
    }

    // add item to a queue
    tevent_queue_add(fronta, ev, req[1], trigger, data);
    tevent_queue_add(fronta, ev, req[2], trigger, data);
    tevent_queue_add(fronta, ev, req[3], trigger, data);
    tevent_queue_add(fronta, ev, req[4], trigger, data);

    printf("Queue length: %d\n", tevent_queue_length(fronta));
    printf("----------------------------\n");
    while (tevent_queue_length(fronta) > 0)
    {
        printf("_______________________\n");
        tevent_loop_once(ev);
        printf("Queue: %d items left\n", tevent_queue_length(fronta));
    }
    printf("----------------------------\n");
    //tevent_loop_wait(ev);

    talloc_free(mem_ctx);
    printf("FINISH\n");

    return EXIT_SUCCESS;
}
