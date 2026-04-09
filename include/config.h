#ifndef CONFIG_H
#define CONFIG_H

/*
 * Default server configuration
 */
#define DEFAULT_SERVER_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT_RECEIVER 1231
#define DEFAULT_SERVER_PORT_SENDER 1232

/*
 * Default metric file path
 */
#define METRIC_FILE_PATH   "metrics.log"

/*
 * Default command buffer size
 */
#define CMD_BUF_SIZE 32

/*
 * Number of sensors
 */
#define NUM_SENSORS 3

/*
 * Matrix configuration
 */
#define MATRIX_FB_PATH "/dev/fb0"
#define MATRIX_SIZE 8
#define MATRIX_WIDTH   8
#define MATRIX_HEIGHT  8
#define BYTES_PER_PIXEL    2   /* RGB565 */
#define LINE_LENGTH_BYTES  16

/*
 * Blink delay in microseconds for the LED matrix alarm display
 */
#define BLINK_DELAY_US 300000

#endif