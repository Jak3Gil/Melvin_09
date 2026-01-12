# Guide: Using Port Pipeline in Tests

**Date**: Friday, January 9, 2026  
**Purpose**: Minimal changes to use proper input port pipeline

---

## The Minimal Change

### Before (Direct API)
```c
// OLD: Bypass port pipeline
melvin_m_universal_input_write(mfile, data, size);
melvin_m_process_input(mfile);
```

### After (Port Pipeline)
```c
// NEW: Use port pipeline
melvin_in_port_process_device(mfile, port_id, data, size);
```

**That's it!** One function call replaces two, and adds port tracking.

---

## What You Need

### 1. Add Include
```c
#include "melvin_in_port.h"  // Add this
```

### 2. Compile with Port Files
```bash
gcc -o test test.c src/melvin.c src/melvin_in_port.c src/melvin_out_port.c -Isrc
```

---

## Port IDs

Simple convention:
- **Port 0**: Standard input (default)
- **Port 1**: Error/debug input
- **Port 2+**: Custom devices

For most tests, just use **port 0**.

---

## Example: Convert Existing Test

### Original Code
```c
#include "melvin.h"

int main() {
    MelvinMFile *mfile = melvin_m_create("test.m");
    
    // Train
    for (int i = 0; i < 100; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)"hello", 5);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test
    melvin_m_universal_input_write(mfile, (uint8_t*)"hel", 3);
    melvin_m_process_input(mfile);
    
    size_t len = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(len);
    melvin_m_universal_output_read(mfile, output, len);
    
    printf("Output: %.*s\n", (int)len, output);
    free(output);
    
    melvin_m_close(mfile);
    return 0;
}
```

### Ported Code
```c
#include "melvin.h"
#include "melvin_in_port.h"  // ADD THIS

int main() {
    MelvinMFile *mfile = melvin_m_create("test.m");
    
    // Train - CHANGE THIS
    for (int i = 0; i < 100; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello", 5);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test - CHANGE THIS
    melvin_in_port_process_device(mfile, 0, (uint8_t*)"hel", 3);
    
    // Output reading stays the same
    size_t len = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(len);
    melvin_m_universal_output_read(mfile, output, len);
    
    printf("Output: %.*s\n", (int)len, output);
    free(output);
    
    melvin_m_close(mfile);
    return 0;
}
```

**Changes**:
1. Added `#include "melvin_in_port.h"`
2. Replaced `melvin_m_universal_input_write() + melvin_m_process_input()` with `melvin_in_port_process_device()`
3. Output reading unchanged (can keep using direct API)

---

## What the Port Pipeline Does

`melvin_in_port_process_device()` does:
1. Stores port_id in mfile (for routing)
2. Writes data to universal input buffer
3. Calls `melvin_m_process_input()`
4. Returns success/error

**Benefit**: Port ID is tracked, enabling proper routing and device identification.

---

## Do You Need to Change Output?

**No, not required.** You can still use:
```c
melvin_m_universal_output_size()
melvin_m_universal_output_read()
```

**Optional**: Use port output for routing:
```c
melvin_out_port_read_and_route(mfile);  // Reads + routes + clears
```

---

## Summary

**Minimal change for tests**:
1. Add `#include "melvin_in_port.h"`
2. Replace input calls with `melvin_in_port_process_device()`
3. Compile with `melvin_in_port.c` and `melvin_out_port.c`

**That's it!** Your tests now use the proper pipeline.

---

## Test Example

See `tests/test_with_ports.c` for a working example.

**Run it**:
```bash
cd tests
./test_with_ports
```

**Expected output**:
```
Training with port pipeline:
Pattern: 'hello world'

Training complete (100 iterations)

Testing generation:
Input: 'hello '
Output: 'wo wo wo '
Expected: 'world'

Graph: 13 nodes, 25 edges
```

---

**Status**: ✅ Port pipeline working  
**Change**: Minimal (one function call)  
**Benefit**: Proper port tracking, full pipeline tested
