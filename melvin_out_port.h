/*
 * Melvin: Output Port Handler Header
 */

#ifndef MELVIN_OUT_PORT_H
#define MELVIN_OUT_PORT_H

#include "melvin.h"
#include <stdint.h>
#include <stddef.h>

/* Port routing structure */
typedef struct PortRoute {
    uint8_t input_port;
    uint8_t output_port;
} PortRoute;

/* Routing table management */
void melvin_out_port_set_routing_table(const PortRoute *routes, size_t count);
uint8_t melvin_out_port_get_output_port(uint8_t input_port);

/* Main output handler */
int melvin_out_port_read_and_route(MelvinMFile *mfile);

/* Device-specific handlers */
/* Note: melvin_out_port_send_to_device is internal, use melvin_out_port_read_and_route */
int melvin_out_port_handle_text_file(uint8_t port_id, 
                                      const char *filename,
                                      const uint8_t *data,
                                      size_t size);
int melvin_out_port_handle_stdout(const uint8_t *data, size_t size);
int melvin_out_port_display_output(MelvinMFile *mfile);

/* Error feedback (Error-Based Learning) */
float melvin_out_port_compare_output(const uint8_t *output, size_t output_size,
                                     const uint8_t *expected, size_t expected_size);
int melvin_out_port_process_with_feedback(MelvinMFile *mfile,
                                          const uint8_t *expected_output,
                                          size_t expected_size);

#endif /* MELVIN_OUT_PORT_H */

