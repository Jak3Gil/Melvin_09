/*
 * Melvin: Emergent Intelligence System
 * 
 * Header file with shared structures and public interface
 * for melvin.c, melvin_in_port.c, and melvin_out_port.c
 */

#ifndef MELVIN_H
#define MELVIN_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Graph Graph;
typedef struct MFile MFile;

/* ============================================================================
 * PORT FRAME STRUCTURE (CAN Bus-Style Data Packaging)
 * ============================================================================ */

/* PortFrame: External data packaging
 * - Used by melvin_in_port.c and melvin_out_port.c
 * - Format: [port_id (1)] + [timestamp (8)] + [data_size (4)] + [data (N)]
 */
typedef struct PortFrame {
    uint8_t port_id;        // Port identifier (e.g., 5 = USB mic, 6 = USB camera)
    uint64_t timestamp;     // When data was captured
    uint32_t data_size;     // Size of actual device data
    uint8_t data[];         // Raw device data (flexible array member)
} PortFrame;

/* ============================================================================
 * PUBLIC INTERFACE (melvin.c)
 * ============================================================================ */

/* Type alias for compatibility (MFile is defined in melvin.c) */
typedef MFile MelvinMFile;

/* .m file management */
MelvinMFile* melvin_m_load(const char *filename);
MelvinMFile* melvin_m_create(const char *filename);
int melvin_m_save(MelvinMFile *mfile);
void melvin_m_close(MelvinMFile *mfile);

/* Universal input operations */
void melvin_m_universal_input_write(MelvinMFile *mfile, 
                                     const uint8_t *data, 
                                     size_t size);
size_t melvin_m_universal_input_size(MelvinMFile *mfile);
void melvin_m_universal_input_clear(MelvinMFile *mfile);

/* Processing */
int melvin_m_process_input(MelvinMFile *mfile);
uint8_t melvin_m_get_last_input_port_id(MelvinMFile *mfile);

/* Universal output operations */
size_t melvin_m_universal_output_size(MelvinMFile *mfile);
size_t melvin_m_universal_output_read(MelvinMFile *mfile, 
                                      uint8_t *buffer, 
                                      size_t buffer_size);
void melvin_m_universal_output_clear(MelvinMFile *mfile);

/* Error feedback operations (Error-Based Learning) */
void melvin_m_feedback_error(MelvinMFile *mfile, float error_signal);
// error_signal: 0.0 = completely wrong, 1.0 = completely correct
// Adjusts weights of edges used in last output generation
// Combines with frequency-based (Hebbian) learning for dual learning system

/* Self-supervised learning operations */
void melvin_m_strengthen_continuation(MelvinMFile *mfile, const uint8_t *sequence, 
                                       size_t prefix_len, size_t total_len);
// Strengthens edges in the continuation (sequence[prefix_len:]) during training
// Gives correct path a head start (like supervised learning)

/* Statistics (for monitoring) */
size_t melvin_m_get_node_count(MelvinMFile *mfile);
size_t melvin_m_get_edge_count(MelvinMFile *mfile);
uint64_t melvin_m_get_adaptation_count(MelvinMFile *mfile);

#endif /* MELVIN_H */

