#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "led_matrix.h"
#include "config.h"

int led_matrix_init(led_matrix_t *matrix) {
    if (!matrix) return -1;

    matrix->framebuffer_fd = -1;
    matrix->framebuffer_ptr = 0;
    matrix->framebuffer_size = 0;

    matrix->framebuffer_fd = open(MATRIX_FB_PATH, O_RDWR);
    if (matrix->framebuffer_fd < 0) {
        printf("Error: cannot open framebuffer device.\n");
        return -1;
    }

    if (ioctl(matrix->framebuffer_fd, FBIOGET_FSCREENINFO, &matrix->fixed_info)) {
        printf("Error reading fixed information.\n");
        close(matrix->framebuffer_fd);
        matrix->framebuffer_fd = -1;
        return -1;
    }

    if (ioctl(matrix->framebuffer_fd, FBIOGET_VSCREENINFO, &matrix->variable_info)) {
        printf("Error reading variable information.\n");
        close(matrix->framebuffer_fd);
        matrix->framebuffer_fd = -1;
        return -1;
    }

    matrix->framebuffer_size = MATRIX_HEIGHT * LINE_LENGTH_BYTES;
    matrix->framebuffer_ptr = (char *)mmap(
        0,
        matrix->framebuffer_size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        matrix->framebuffer_fd,
        0
    );

    if (matrix->framebuffer_ptr == MAP_FAILED) {
        printf("Error: failed to map framebuffer device to memory.\n");
        close(matrix->framebuffer_fd);
        matrix->framebuffer_fd = -1;
        matrix->framebuffer_ptr = 0;
        return -1;
    }

    return 0;
}

void led_matrix_destroy(led_matrix_t *matrix){
    if (!matrix) return;

    if (matrix->framebuffer_fd >= 0) {
        close(matrix->framebuffer_fd);
    }

    matrix->framebuffer_fd = -1;
    matrix->framebuffer_ptr = 0;
    matrix->framebuffer_size = 0;
}

int led_matrix_draw(led_matrix_t *matrix, led_color_t color){
    int x, y, location;

    for (y = 0; y < MATRIX_SIZE; y++) {
        for (x = 0; x < MATRIX_SIZE; x++) {
            location = (x + matrix->variable_info.xoffset) * (matrix->variable_info.bits_per_pixel / 8)
                     + (y + matrix->variable_info.yoffset) * matrix->fixed_info.line_length;
        
            *((uint16_t *)(matrix->framebuffer_ptr + location)) = color;
        }
    }

    return 0;
}

led_color_t led_color_from_alarm_level(alarm_level_t level) {
    switch (level) {
    case ALARM_L:
        return COLOR_GREEN;
    case ALARM_M:
        return COLOR_ORANGE;
    case ALARM_H:
        return COLOR_RED;
    default:
        return COLOR_WHITE;
    }
}

int led_matrix_draw_gas(led_matrix_t *matrix, int gas_idx, led_color_t color) {
    if (!matrix || !matrix->framebuffer_ptr) return -1;
    if (gas_idx < 0 || gas_idx > 2) return -1;

    int col_start;
    if (gas_idx == 0) col_start = 0;
    else if (gas_idx == 1) col_start = 3;
    else col_start = 6;
    int col_end = col_start + 1;

    int x, y, location;
    for (y = 0; y < MATRIX_SIZE; y++) {
        for (x = col_start; x <= col_end; x++) {
            location = (x + matrix->variable_info.xoffset) * (matrix->variable_info.bits_per_pixel / 8)
                     + (y + matrix->variable_info.yoffset) * matrix->fixed_info.line_length;
            *((uint16_t *)(matrix->framebuffer_ptr + location)) = color;
        }
    }

    return 0;
}

int led_matrix_set_gas_level(led_matrix_t *matrix, int gas_idx, alarm_level_t level) {
    led_color_t color = led_color_from_alarm_level(level);
    return led_matrix_draw_gas(matrix, gas_idx, color);
}