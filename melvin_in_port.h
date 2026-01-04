/*
 * Melvin: Input Port Handler Header
 */

#ifndef MELVIN_IN_PORT_H
#define MELVIN_IN_PORT_H

#include "melvin.h"
#include <stdint.h>
#include <stddef.h>

/* Port frame serialization */
size_t port_frame_serialize(const PortFrame *frame, uint8_t *buffer, size_t buffer_size);
int port_frame_deserialize(const uint8_t *buffer, size_t buffer_size, PortFrame *frame);

/* Main input handler */
int melvin_in_port_process_device(MelvinMFile *mfile, 
                                   uint8_t port_id,
                                   const uint8_t *raw_data,
                                   size_t data_size);

/* Device-specific handlers */
int melvin_in_port_handle_text_file(MelvinMFile *mfile, 
                                     uint8_t port_id, 
                                     const char *filename);
int melvin_in_port_handle_text_file_chunked(MelvinMFile *mfile,
                                             uint8_t port_id,
                                             const char *filename,
                                             size_t chunk_size);
int melvin_in_port_handle_stdin(MelvinMFile *mfile, uint8_t port_id);
int melvin_in_port_handle_buffer(MelvinMFile *mfile, 
                                  uint8_t port_id,
                                  const uint8_t *buffer,
                                  size_t buffer_size);

#endif /* MELVIN_IN_PORT_H */

