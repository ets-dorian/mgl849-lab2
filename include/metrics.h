#ifndef METRICS_H
#define METRICS_H

#include "config.h"

/*
 * cost_of_cmd - Compute the cost of a command.
 * @cmd: Command to compute the cost of.
 * Returns the cost of the command.
 */
int cost_of_cmd(char *cmd);

/*
 * compute_cost - Compute the total cost of a sequence of commands.
 * @cmd: Sequence of commands to compute the cost of.
 * Returns the total cost of the sequence.
 */
int compute_cost(char *cmd);

/*
 * compute_efficiency - Compute the efficiency of a sequence of commands.
 /*
 /* E_i = (1/N) * Σ_j (prev_j - cur_j)                                
 /*      + (2/N) * max_j (prev_j - cur_j)
 /*   
 /* Un delta positif = la concentration a baissé = action efficace.
 /* Un delta négatif = la fuite empire malgré la réaction.
 * @prev: Previous sensor readings.
 * @cur: Current sensor readings.
 * Returns the efficiency of the sequence.
 */
double compute_efficiency(int prev[NUM_SENSORS], int cur[NUM_SENSORS]);

#endif