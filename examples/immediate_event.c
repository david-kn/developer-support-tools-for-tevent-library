/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Example which set 2 immediate events which are triggered as soon as tevent loop is reached
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <tevent.h>
#include <sys/time.h>

 struct info_struct {
     int counter;
 };

 static void foo(struct tevent_context *ev, struct tevent_immediate *im, void *private_data) {

        printf("--------\n");
        printf("Callback\n");
        struct info_struct *data = talloc_get_type(private_data, struct info_struct);
        printf("Data value: %d\n", data->counter);
}

int main (void) {

    printf("INIT\n");

    struct tevent_context *event_ctx;
    TALLOC_CTX *mem_ctx;
    struct tevent_immediate *im, *im2;
    struct timeval schedule;

    mem_ctx = talloc_new(NULL); //parent
    event_ctx = tevent_context_init(mem_ctx);

    struct info_struct *data = talloc(mem_ctx, struct info_struct);
    struct info_struct *data2 = talloc(mem_ctx, struct info_struct);


    //  setting up private data
    data->counter = 1;
    data2->counter = 2;

    // first immediate event
    im = tevent_create_immediate(mem_ctx);
    if(im == NULL) {
        fprintf(stderr, "FAILED\n");
        return EXIT_FAILURE;
    }
    tevent_schedule_immediate(im, event_ctx, foo, data);

    // second immediate event
    im2 = tevent_create_immediate(mem_ctx);
    if(im == NULL) {
        fprintf(stderr, "FAILED\n");
        return EXIT_FAILURE;
    }
    tevent_schedule_immediate(im2, event_ctx, foo, data2);

    tevent_loop_wait(event_ctx);
    talloc_free(mem_ctx);

    printf("Quit.\n");
    return EXIT_SUCCESS;
}
