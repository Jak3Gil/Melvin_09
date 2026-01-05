#!/bin/bash
# Production Test: Complex Growing .m File
# Tests all 6 mechanisms using melvin_in_port with persistent .m file
# Then analyzes using melvin_out_port

set -e

BRAIN_FILE="test_production_brain.m"
PORT_ID=1

echo "=========================================="
echo "Melvin Production System Test"
echo "Testing complex growing .m file"
echo "=========================================="
echo ""

# Clean up previous test
rm -f "$BRAIN_FILE" test_production_*.txt test_production_output.txt

# Create test input files that will trigger all mechanisms
echo "Creating test inputs..."

# Test 1: Repeated patterns (triggers hierarchy formation)
cat > test_production_input1.txt << 'EOF'
cat cat cat
dog dog dog
cat dog cat dog
EOF

# Test 2: Similar patterns (triggers blank node generalization)
cat > test_production_input2.txt << 'EOF'
similar1 similar2 similar3
similar4 similar5 similar6
pattern1 pattern2 pattern3
EOF

# Test 3: Long repeated sequences (triggers payload growth)
cat > test_production_input3.txt << 'EOF'
repeat repeat repeat repeat repeat
again again again again again
EOF

# Test 4: Complex patterns (triggers all mechanisms)
cat > test_production_input4.txt << 'EOF'
hello world hello world
the quick brown fox
hello world hello world
the quick brown fox
EOF

# Test 5: Novel patterns (tests adaptation)
cat > test_production_input5.txt << 'EOF'
novel pattern here
completely new data
different structure
EOF

echo "✓ Test inputs created"
echo ""

# Check if we need to compile the port handlers
if [ ! -f melvin_in_port.o ] || [ ! -f melvin_out_port.o ]; then
    echo "Compiling port handlers..."
    gcc -O3 -c melvin_in_port.c -o melvin_in_port.o
    gcc -O3 -c melvin_out_port.c -o melvin_out_port.o
    echo "✓ Port handlers compiled"
    echo ""
fi

# Create a test program that uses the port handlers
cat > test_production_runner.c << 'EOFC'
#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain.m> <input_file> [port_id]\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    const char *input_file = argv[2];
    uint8_t port_id = (argc > 3) ? (uint8_t)atoi(argv[3]) : 1;
    
    // Open or create .m file
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        mfile = melvin_m_create(brain_path);
        if (!mfile) {
            fprintf(stderr, "Error: Failed to create/open brain file\n");
            return 1;
        }
    }
    
    // Process input using melvin_in_port
    int result = melvin_in_port_handle_text_file(mfile, port_id, input_file);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to process input file\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    // Save the brain after processing
    melvin_m_save(mfile);
    
    // Get statistics
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
    
    printf("Processed: %s\n", input_file);
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    printf("  Adaptations: %llu\n", (unsigned long long)adaptations);
    
    // Check for output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("  Output size: %zu bytes\n", output_size);
    }
    
    melvin_m_close(mfile);
    return 0;
}
EOFC

# Compile test runner
echo "Compiling test runner..."
gcc -O3 -c melvin.c -o melvin_lib.o
gcc -O3 -o test_production_runner test_production_runner.c melvin_in_port.o melvin_out_port.o melvin_lib.o -lm -lpthread
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile test runner"
    exit 1
fi
echo "✓ Test runner compiled"
echo ""

# Process all inputs sequentially (persistent .m file grows)
echo "=========================================="
echo "Processing inputs (persistent .m file)..."
echo "=========================================="
echo ""

for i in 1 2 3 4 5; do
    echo "--- Input $i ---"
    ./test_production_runner "$BRAIN_FILE" "test_production_input${i}.txt" "$PORT_ID"
    
    # Show .m file size growth
    if [ -f "$BRAIN_FILE" ]; then
        SIZE=$(stat -f%z "$BRAIN_FILE" 2>/dev/null || stat -c%s "$BRAIN_FILE" 2>/dev/null || echo "0")
        echo "  Brain file size: $SIZE bytes"
    fi
    echo ""
done

echo "=========================================="
echo "Final Brain Statistics"
echo "=========================================="
echo ""

if [ -f "$BRAIN_FILE" ]; then
    FINAL_SIZE=$(stat -f%z "$BRAIN_FILE" 2>/dev/null || stat -c%s "$BRAIN_FILE" 2>/dev/null || echo "0")
    echo "Final brain file size: $FINAL_SIZE bytes"
    echo ""
    
    # Get final statistics
    ./test_production_runner "$BRAIN_FILE" "test_production_input1.txt" "$PORT_ID" 2>&1 | grep -E "(Nodes|Edges|Adaptations)" | tail -3
    echo ""
fi

# Create output analyzer
cat > test_production_analyzer.c << 'EOFC'
#include "melvin.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m>\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    
    // Open .m file
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to open brain file\n");
        return 1;
    }
    
    printf("==========================================\n");
    printf("Brain Analysis\n");
    printf("==========================================\n");
    printf("\n");
    
    // Statistics
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
    
    printf("Statistics:\n");
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    printf("  Adaptations: %llu\n", (unsigned long long)adaptations);
    printf("\n");
    
    // Check for output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("Output:\n");
        printf("  Size: %zu bytes\n", output_size);
        
        // Read and display output
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("  Content: ");
            for (size_t i = 0; i < output_size && i < 100; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("%c", output[i]);
                } else {
                    printf("\\x%02x", output[i]);
                }
            }
            if (output_size > 100) {
                printf("... (truncated)");
            }
            printf("\n");
            free(output);
        }
        printf("\n");
    } else {
        printf("No output generated (pure thinking mode)\n");
        printf("\n");
    }
    
    // Display output using melvin_out_port
    printf("==========================================\n");
    printf("Output Port Analysis\n");
    printf("==========================================\n");
    printf("\n");
    
    // Set up routing (input port 1 -> output port 1)
    PortRoute route = {1, 1};
    melvin_out_port_set_routing_table(&route, 1);
    
    // Read and route output
    int result = melvin_out_port_read_and_route(mfile);
    if (result == 0) {
        printf("Output routed successfully\n");
    } else {
        printf("No output to route (or routing not configured)\n");
    }
    
    // Display output
    melvin_out_port_display_output(mfile);
    
    melvin_m_close(mfile);
    return 0;
}
EOFC

# Compile analyzer
echo "Compiling output analyzer..."
gcc -O3 -o test_production_analyzer test_production_analyzer.c melvin_out_port.o melvin_lib.o -lm -lpthread
if [ $? -ne 0 ]; then
    echo "Error: Failed to compile analyzer"
    exit 1
fi
echo "✓ Analyzer compiled"
echo ""

# Analyze the brain
echo "=========================================="
echo "Analyzing Brain with melvin_out_port"
echo "=========================================="
echo ""

./test_production_analyzer "$BRAIN_FILE" > test_production_analysis.txt 2>&1
cat test_production_analysis.txt

echo ""
echo "=========================================="
echo "Test Complete"
echo "=========================================="
echo ""
echo "Files created:"
echo "  - $BRAIN_FILE (persistent brain)"
echo "  - test_production_analysis.txt (analysis output)"
echo ""
echo "Check the brain file size to see growth:"
if [ -f "$BRAIN_FILE" ]; then
    ls -lh "$BRAIN_FILE"
fi

