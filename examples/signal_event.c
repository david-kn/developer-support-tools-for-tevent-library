/*
 * David Koňař (xkonar07@stud.fit.vutbr.cz)
 *
 * Example which set signal event. Catches SIGINT signal and prints message to stdout. To test catching of signal
 * use command 'kill -SIGINT signal_event'.
 *
 * NOTE: to kill this process, use either e.g. gnome system monitor (or some alternative) or command 'killall -SIGKILL signal_event'.
 */

#include <stdio.h>
#include <tevent.h>
#include <signal.h>


static void handler(struct tevent_context *ev,
                    struct tevent_signal *se,
                    int signum,
                    int count,
                    void *siginfo,
                    void *private_data) {

        /*
         *  Do something usefull
         */

        printf("handling signal...\n");
        exit(EXIT_SUCCESS);
}

int main (void) {
    printf("INIT\n");

    struct tevent_context *event_ctx;
    TALLOC_CTX *mem_ctx;
    struct tevent_signal *sig;

    mem_ctx = talloc_new(NULL); //parent
    if(mem_ctx == NULL) {
        fprintf(stderr, "FAILED\n");
        return EXIT_FAILURE;
    }
    event_ctx = tevent_context_init(mem_ctx);
    if(event_ctx == NULL) {
        fprintf(stderr, "FAILED\n");
        return EXIT_FAILURE;
    }

    if(tevent_signal_support(event_ctx)) {
        // create signal event
        sig = tevent_add_signal(event_ctx, mem_ctx, SIGINT, 0, handler, NULL);
        if(sig == NULL) {
                fprintf(stderr, "FAILED\n");
                return EXIT_FAILURE;
        }
        tevent_loop_wait(event_ctx);
    }

    talloc_free(mem_ctx);
    printf("Quit.\n");
    return EXIT_SUCCESS;
}
