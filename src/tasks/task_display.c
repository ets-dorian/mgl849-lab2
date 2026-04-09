#define _POSIX_C_SOURCE 200809L
#include "led_matrix.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "shared_data.h"

#define BLINK_INTERVAL_MS 500

void* task_display(void* arg) {
    
    SharedData *shared = (SharedData *)arg;
    led_matrix_t led_matrix;
    
    if (led_matrix_init(&led_matrix) != 0) {
       fprintf(stderr, "Init matrix failed\n");
       return NULL;
    }
    
    printf("Led thread booted\n");

    alarm_t levels[3] = {NONE, NONE, NONE};
    int blink_on = 1;

    while(1){
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += BLINK_INTERVAL_MS / 1000;
        ts.tv_nsec += (BLINK_INTERVAL_MS % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000L;
        }

        int ret = sem_timedwait(&shared->sem_new_alarm, &ts);
        if (ret == 0) {
            pthread_mutex_lock(&shared->lock);
            levels[0] = shared->alarm_levels[0];
            levels[1] = shared->alarm_levels[1];
            levels[2] = shared->alarm_levels[2];
            pthread_mutex_unlock(&shared->lock);
        } else if (errno == ETIMEDOUT) {
            blink_on = !blink_on;
        } else {
            if (errno != EINTR) {
                fprintf(stderr, "sem_timedwait error: %s\n", strerror(errno));
            }
            continue;
        }

        for (int i = 0; i < 3; ++i) {
            int res;
            if (blink_on) {
                res = led_matrix_set_gas_level(&led_matrix, i, levels[i]);
            } else {
                res = led_matrix_draw_gas(&led_matrix, i, COLOR_BLACK);
            }
            if (res == -1){
                fprintf(stderr, "Can't draw matrix led\n");
            }
        }
    }

    return NULL;
}