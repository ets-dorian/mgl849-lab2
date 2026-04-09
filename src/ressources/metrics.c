#define _GNU_SOURCE

#include "shared_data.h"
#include "config.h"
#include "metrics.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>

int cost_of_cmd(char *cmd)
{
    if (strcmp(cmd, "AL1") == 0)       return 1;
    if (strcmp(cmd, "AL2") == 0)       return 2;
    if (strcmp(cmd, "AL3") == 0)       return 3;
    if (strcmp(cmd, "VL1") == 0)       return 6;
    if (strcmp(cmd, "VL2") == 0)       return 9;
    if (strncmp(cmd, "IG", 2) == 0)    return 20;

    return 0;
}
 

int compute_cost(char *cmd)
{
    if (!cmd || cmd[0] == '\0') return 0;
 
    char tmp[CMD_BUF_SIZE];
    strncpy(tmp, cmd, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
 
    int total = 0;
    char *token = strtok(tmp, ";");
    while (token) {
        total += cost_of_cmd(token);
        token = strtok(NULL, ";");
    }
    return total;
}

double compute_efficiency(int prev[NUM_SENSORS], int cur[NUM_SENSORS])
{
    double sum     = 0.0;
    double max_val = -DBL_MAX;
 
    for (int j = 0; j < NUM_SENSORS; j++) {
        double delta = (double)(prev[j] - cur[j]);
        sum += delta;
        if (delta > max_val) max_val = delta;
    }
 
    if (max_val == -DBL_MAX) max_val = 0.0;
 
    return (sum / NUM_SENSORS) + (2.0 * max_val / NUM_SENSORS);
}