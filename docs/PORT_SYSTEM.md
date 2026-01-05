# Multi-Modal Port System

## Overview

The port system enables Melvin to handle multiple input/output modalities (text, audio, video, CAN bus, motors, sensors) using the same unified graph architecture—similar to how the brain uses neurons and synapses for all sensory modalities.

## Architecture

### Core Principle: Unified Structure, Specialized Routing

```
Input Ports → Nodes (with port_id) → Edges (cross-modal) → Output Filtering → Output Ports
```

**Like the brain:**
- Visual cortex (specialized) → Neurons (unified) → Motor cortex (specialized)
- All use same neural architecture, different routing

**In Melvin:**
- Text port (specialized) → Nodes (unified) → Text output (specialized)
- All use same node/edge architecture, different port_id

## Port ID Definitions

```c
#define PORT_TEXT    0x01   // Text input/output
#define PORT_AUDIO   0x02   // Audio input/output (PCM samples)
#define PORT_VIDEO   0x03   // Video input/output (frame data)
#define PORT_CANBUS  0x04   // CAN bus input/output
#define PORT_MOTOR   0x05   // Motor control output
#define PORT_SENSOR  0x06   // Sensor input (generic)
```

## How It Works

### 1. Input Processing

```c
// Text input
melvin_in_port_handle_buffer(mfile, PORT_TEXT, text_data, text_size);
// → Creates nodes with port_id = 0x01

// Audio input
melvin_in_port_handle_buffer(mfile, PORT_AUDIO, audio_data, audio_size);
// → Creates nodes with port_id = 0x02

// Video input
melvin_in_port_handle_buffer(mfile, PORT_VIDEO, video_data, video_size);
// → Creates nodes with port_id = 0x03
```

**Result:** Nodes store `port_id` as metadata (where they came from)

### 2. Cross-Modal Learning

**Edges connect freely across ports:**
```
Text node (port_id=0x01) → Audio node (port_id=0x02)
Video node (port_id=0x03) → Motor node (port_id=0x05)
```

**Example: Learning "hello" text → audio association:**
```c
// 1. Process text "hello"
melvin_in_port_handle_buffer(mfile, PORT_TEXT, "hello", 5);

// 2. Process audio waveform for "hello"
melvin_in_port_handle_buffer(mfile, PORT_AUDIO, audio_samples, sample_count);

// 3. Edges form between text nodes and audio nodes (Hebbian learning)
// 4. Next time: text "hello" activates audio nodes through learned edges
```

### 3. Output Filtering

**Only nodes matching output port are generated:**

```c
// Input: Text "hello" (port_id = 0x01)
// Wave propagation: Activates text nodes + audio nodes (via edges)
// Output filtering: Only text nodes (port_id = 0x01) are output
// Result: Text output only, no audio bytes mixed in
```

**Implementation:**
```c
// In generate_from_pattern (melvin.c line ~3271):
if (node->port_id != 0 && output_port != 0 && node->port_id != output_port) {
    continue;  // Filter out incompatible nodes
}
```

### 4. Routing Table

```c
// Set up routing: input_port → output_port
PortRoute routes[] = {
    {PORT_TEXT, PORT_TEXT},      // Text in → Text out
    {PORT_AUDIO, PORT_AUDIO},    // Audio in → Audio out
    {PORT_VIDEO, PORT_MOTOR},    // Video in → Motor out (learned control)
};
melvin_out_port_set_routing_table(routes, 3);
```

## Brain Analogy

### Brain Multi-Modal System

```
Visual Input → Visual Cortex (V1, V2, V4)
  ↓ (neurons)
Association Areas (cross-modal connections)
  ↓ (neurons)
Motor Cortex → Motor Output
```

**Key features:**
- Specialized input areas (visual, auditory, somatosensory)
- Unified neural architecture (all use neurons/synapses)
- Cross-modal connections (visual → auditory learned associations)
- Specialized output areas (motor cortex controls muscles)

### Melvin Multi-Modal System

```
Text Input → Text Nodes (port_id=0x01)
  ↓ (edges)
Cross-Modal Edges (text ↔ audio learned associations)
  ↓ (edges)
Text Output (filtered by port_id)
```

**Key features:**
- Specialized input ports (text, audio, video, CAN bus)
- Unified node/edge architecture (all use same structure)
- Cross-modal edges (text → audio learned associations)
- Specialized output filtering (port_id routing)

## Example Use Cases

### 1. Text-to-Speech (Reading → Speaking)

```c
// Training: Process text + audio together
melvin_in_port_handle_buffer(mfile, PORT_TEXT, "hello", 5);
melvin_in_port_handle_buffer(mfile, PORT_AUDIO, audio_waveform, audio_size);
// → Edges form: text nodes → audio nodes

// Usage: Text input generates audio output
PortRoute routes[] = {{PORT_TEXT, PORT_AUDIO}};
melvin_out_port_set_routing_table(routes, 1);
melvin_in_port_handle_buffer(mfile, PORT_TEXT, "hello", 5);
// → Output: audio bytes (through learned text→audio edges)
```

### 2. Vision-Based Motor Control (Seeing → Acting)

```c
// Training: Process video + motor commands together
melvin_in_port_handle_buffer(mfile, PORT_VIDEO, red_light_frame, frame_size);
melvin_in_port_handle_buffer(mfile, PORT_MOTOR, brake_command, cmd_size);
// → Edges form: video nodes → motor nodes

// Usage: Video input generates motor output
PortRoute routes[] = {{PORT_VIDEO, PORT_MOTOR}};
melvin_out_port_set_routing_table(routes, 1);
melvin_in_port_handle_buffer(mfile, PORT_VIDEO, red_light_frame, frame_size);
// → Output: motor command (through learned video→motor edges)
```

### 3. Multi-Modal Sensor Fusion

```c
// Process multiple inputs simultaneously
melvin_in_port_handle_buffer(mfile, PORT_VIDEO, camera_frame, frame_size);
melvin_in_port_handle_buffer(mfile, PORT_CANBUS, can_data, can_size);
melvin_in_port_handle_buffer(mfile, PORT_SENSOR, lidar_data, lidar_size);
// → All nodes activate, edges form across modalities
// → System learns sensor fusion patterns automatically
```

## Implementation Details

### Node Structure

```c
typedef struct Node {
    uint8_t id[9];
    uint8_t *payload;
    size_t payload_size;
    
    uint8_t port_id;  // NEW: Where this node came from (0 = unknown)
    
    float activation_strength;
    float weight;
    // ... other fields ...
} Node;
```

### Port ID Inheritance

**Hierarchy nodes inherit port_id:**
```c
// When creating hierarchy from "he" + "llo" → "hello"
hierarchy->port_id = node1->port_id;  // Inherit from first node
```

**Blank nodes have port_id = 0:**
```c
// Blank nodes bridge different modalities
blank_node->port_id = 0;  // Can connect to any port
```

### File Persistence

Port IDs are saved/loaded with nodes:
```c
// Save: Write port_id after context_trace_len
write(fd, &node->port_id, sizeof(uint8_t));

// Load: Read port_id after context_trace_len
read(fd, &node->port_id, sizeof(uint8_t));
```

## Benefits

1. **Unified Architecture:** Same node/edge structure for all modalities
2. **Cross-Modal Learning:** Edges form naturally between any nodes
3. **Format Safety:** Output filtering prevents format mismatches
4. **Biological Plausibility:** Mirrors brain's multi-modal processing
5. **Scalability:** Add new ports without changing core

## Testing

Run the port system test:
```bash
./tests/test_port_system
```

Expected output:
```
✓ Nodes store port_id metadata
✓ Cross-modal edges form naturally
✓ Output filtering by port_id works
✓ Port system persists across save/load
✓ Multi-modal routing enabled
```

## Future Extensions

1. **Learned Routing:** System learns which ports to route to (not just static table)
2. **Format Compatibility:** Nodes learn which ports they're compatible with (error feedback)
3. **Multi-Output:** Generate outputs for multiple ports simultaneously
4. **Port Hierarchies:** Meta-ports that combine multiple input ports

## Summary

The port system enables multi-modal intelligence using the same biological principles:
- **Specialized input/output** (like sensory cortices)
- **Unified processing** (like neurons/synapses)
- **Cross-modal learning** (like association areas)
- **Adaptive routing** (like motor cortex)

All modalities use the same graph, same learning rules, same mechanisms—just like the brain.

