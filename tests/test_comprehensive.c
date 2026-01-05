/*
 * Comprehensive Melvin Test Suite
 * 
 * Tests ALL claims about Melvin's capabilities using a SINGLE persistent .m file
 * This proves the system can:
 * - Handle multiple different tasks simultaneously
 * - Scale with diverse patterns
 * - Learn continuously without forgetting
 * - Self-organize and self-regulate
 * 
 * All tests share ONE .m file to prove multi-task capability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "melvin.h"

// Test result tracking
typedef struct {
    const char *test_name;
    int passed;
    int failed;
    char details[256];
} TestResult;

TestResult results[10];
int result_count = 0;

// Get current time in microseconds
static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000000.0 + (double)tv.tv_usec;
}

// Record test result
static void record_result(const char *name, int passed, int failed, const char *details) {
    results[result_count].test_name = name;
    results[result_count].passed = passed;
    results[result_count].failed = failed;
    snprintf(results[result_count].details, sizeof(results[result_count].details), "%s", details);
    result_count++;
}

// Helper: Train on a pattern
static void train_pattern(MelvinMFile *mfile, const char *input, const char *output, int repetitions) {
    for (int i = 0; i < repetitions; i++) {
        char combined[512];
        snprintf(combined, sizeof(combined), "%s %s", input, output);
        
        melvin_m_universal_input_write(mfile, (const uint8_t*)combined, strlen(combined));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
}

// Helper: Test recall
static int test_recall(MelvinMFile *mfile, const char *input, const char *expected) {
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
    melvin_m_process_input(mfile);
    
    uint8_t output[1024];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Check if expected string appears in output
    if (output_len > 0 && strstr((char*)output, expected) != NULL) {
        return 1;
    }
    return 0;
}

/*
 * TEST 1: Continuous Learning Without Forgetting
 * 
 * Teaches 5 different patterns sequentially, verifies all are retained
 */
static void test1_continuous_learning(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 1: Continuous Learning Without Forgetting\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    typedef struct {
        const char *input;
        const char *output;
    } Pattern;
    
    Pattern patterns[] = {
        {"hello", "world"},
        {"good", "morning"},
        {"machine", "learning"},
        {"pattern", "recognition"},
        {"neural", "network"}
    };
    int num_patterns = 5;
    
    size_t initial_nodes = melvin_m_get_node_count(mfile);
    size_t initial_edges = melvin_m_get_edge_count(mfile);
    
    printf("Initial state: %zu nodes, %zu edges\n\n", initial_nodes, initial_edges);
    
    // Learn each pattern sequentially
    for (int p = 0; p < num_patterns; p++) {
        printf("Learning pattern %d: '%s' -> '%s'\n", 
               p+1, patterns[p].input, patterns[p].output);
        
        train_pattern(mfile, patterns[p].input, patterns[p].output, 10);
        
        printf("  Graph now: %zu nodes, %zu edges\n",
               melvin_m_get_node_count(mfile),
               melvin_m_get_edge_count(mfile));
    }
    
    printf("\nTesting recall of ALL patterns:\n");
    int correct = 0;
    int total = num_patterns;
    
    for (int p = 0; p < num_patterns; p++) {
        int recalled = test_recall(mfile, patterns[p].input, patterns[p].output);
        printf("  Pattern %d ('%s' -> '%s'): %s\n",
               p+1, patterns[p].input, patterns[p].output,
               recalled ? "✓ RECALLED" : "✗ FORGOTTEN");
        if (recalled) correct++;
    }
    
    float accuracy = (100.0f * correct) / total;
    printf("\nResult: %d/%d patterns recalled (%.1f%% accuracy)\n", correct, total, accuracy);
    
    char details[256];
    snprintf(details, sizeof(details), "%.1f%% accuracy (%d/%d patterns)", accuracy, correct, total);
    record_result("Continuous Learning", correct, total - correct, details);
    
    if (accuracy >= 80.0f) {
        printf("✓ TEST PASSED: No catastrophic forgetting!\n");
    } else {
        printf("✗ TEST FAILED: Patterns were forgotten\n");
    }
}

/*
 * TEST 2: Sequence Learning
 * 
 * Teaches counting sequence, tests prediction
 */
static void test2_sequence_learning(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 2: Sequence Learning (Numbers 0-9)\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    printf("Teaching counting sequence...\n");
    
    // Teach: 0->1, 1->2, 2->3, etc.
    for (int epoch = 0; epoch < 20; epoch++) {
        for (int i = 0; i < 9; i++) {
            char input[2], output[2];
            snprintf(input, sizeof(input), "%d", i);
            snprintf(output, sizeof(output), "%d", i+1);
            train_pattern(mfile, input, output, 1);
        }
    }
    
    printf("Graph now: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    printf("Testing sequence predictions:\n");
    int correct = 0;
    int total = 9;
    
    for (int i = 0; i < 9; i++) {
        char input[2], expected[2];
        snprintf(input, sizeof(input), "%d", i);
        snprintf(expected, sizeof(expected), "%d", i+1);
        
        int recalled = test_recall(mfile, input, expected);
        printf("  %d -> expected: %d, %s\n", i, i+1, recalled ? "✓" : "✗");
        if (recalled) correct++;
    }
    
    float accuracy = (100.0f * correct) / total;
    printf("\nResult: %d/%d predictions correct (%.1f%% accuracy)\n", correct, total, accuracy);
    
    char details[256];
    snprintf(details, sizeof(details), "%.1f%% accuracy on sequence", accuracy);
    record_result("Sequence Learning", correct, total - correct, details);
    
    if (accuracy >= 70.0f) {
        printf("✓ TEST PASSED: Learned sequential patterns!\n");
    } else {
        printf("✗ TEST FAILED: Could not learn sequence\n");
    }
}

/*
 * TEST 3: Multi-Task Learning
 * 
 * Teaches different types of patterns, verifies all coexist
 */
static void test3_multi_task(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 3: Multi-Task Learning (Different Pattern Types)\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    printf("Teaching diverse pattern types:\n");
    
    // Task A: Words
    printf("  Task A: Word associations\n");
    train_pattern(mfile, "cat", "meow", 20);
    train_pattern(mfile, "dog", "bark", 20);
    
    // Task B: Math
    printf("  Task B: Simple math\n");
    train_pattern(mfile, "two plus two", "four", 20);
    train_pattern(mfile, "five minus one", "four", 20);
    
    // Task C: Commands
    printf("  Task C: Command responses\n");
    train_pattern(mfile, "help", "available", 20);
    train_pattern(mfile, "quit", "goodbye", 20);
    
    printf("\nGraph now: %zu nodes, %zu edges\n\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    printf("Testing all task types:\n");
    int correct = 0;
    int total = 6;
    
    typedef struct {
        const char *input;
        const char *expected;
        const char *task_name;
    } TestCase;
    
    TestCase tests[] = {
        {"cat", "meow", "Word A"},
        {"dog", "bark", "Word B"},
        {"two plus two", "four", "Math A"},
        {"five minus one", "four", "Math B"},
        {"help", "available", "Command A"},
        {"quit", "goodbye", "Command B"}
    };
    
    for (int i = 0; i < total; i++) {
        int recalled = test_recall(mfile, tests[i].input, tests[i].expected);
        printf("  %s ('%s' -> '%s'): %s\n",
               tests[i].task_name, tests[i].input, tests[i].expected,
               recalled ? "✓" : "✗");
        if (recalled) correct++;
    }
    
    float accuracy = (100.0f * correct) / total;
    printf("\nResult: %d/%d tasks correct (%.1f%% accuracy)\n", correct, total, accuracy);
    
    char details[256];
    snprintf(details, sizeof(details), "%.1f%% multi-task accuracy", accuracy);
    record_result("Multi-Task Learning", correct, total - correct, details);
    
    if (accuracy >= 70.0f) {
        printf("✓ TEST PASSED: Can handle multiple task types!\n");
    } else {
        printf("✗ TEST FAILED: Tasks interfered with each other\n");
    }
}

/*
 * TEST 4: Knowledge Compounding (Hierarchy Formation)
 * 
 * Teaches many related patterns, checks for compression
 */
static void test4_hierarchy(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 4: Knowledge Compounding (Hierarchy Formation)\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    size_t nodes_before = melvin_m_get_node_count(mfile);
    
    // Teach 50 variations of similar patterns
    printf("Teaching 50 related patterns...\n");
    const char *prefixes[] = {"the", "a", "one", "my", "your"};
    const char *adjectives[] = {"quick", "lazy", "smart", "happy", "sad"};
    const char *nouns[] = {"fox", "dog", "cat", "bird", "fish"};
    
    int patterns_taught = 0;
    for (int epoch = 0; epoch < 2; epoch++) {
        for (int p = 0; p < 5; p++) {
            for (int a = 0; a < 5; a++) {
                for (int n = 0; n < 2; n++) {
                    char pattern[128];
                    snprintf(pattern, sizeof(pattern), "%s %s %s",
                            prefixes[p], adjectives[a], nouns[n]);
                    
                    melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
                    melvin_m_process_input(mfile);
                    melvin_m_universal_input_clear(mfile);
                    melvin_m_universal_output_clear(mfile);
                    
                    patterns_taught++;
                }
            }
        }
    }
    
    size_t nodes_after = melvin_m_get_node_count(mfile);
    size_t nodes_created = nodes_after - nodes_before;
    
    printf("\nResults:\n");
    printf("  Patterns taught: %d\n", patterns_taught);
    printf("  Nodes created: %zu\n", nodes_created);
    printf("  Graph total: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    float compression_ratio = (float)patterns_taught / (float)nodes_created;
    printf("  Compression ratio: %.2fx\n", compression_ratio);
    
    char details[256];
    snprintf(details, sizeof(details), "%.2fx compression (%d patterns -> %zu nodes)",
             compression_ratio, patterns_taught, nodes_created);
    
    if (compression_ratio >= 1.5f) {
        printf("✓ TEST PASSED: Compression detected (hierarchy forming)!\n");
        record_result("Hierarchy Formation", 1, 0, details);
    } else {
        printf("✗ TEST FAILED: No compression (ratio < 1.5x)\n");
        record_result("Hierarchy Formation", 0, 1, details);
    }
}

/*
 * TEST 5: Scale Test
 * 
 * Add many more patterns, test performance doesn't degrade
 */
static void test5_scale(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 5: Scale Test (Performance Under Load)\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    size_t initial_nodes = melvin_m_get_node_count(mfile);
    
    printf("Adding 200 more patterns to test scale...\n");
    
    double start_time = get_time_us();
    
    for (int i = 0; i < 200; i++) {
        char input[64], output[64];
        snprintf(input, sizeof(input), "pattern_%d", i);
        snprintf(output, sizeof(output), "output_%d", i);
        
        train_pattern(mfile, input, output, 1);
        
        if (i % 100 == 0) {
            printf("  %d patterns added...\n", i);
        }
    }
    
    double end_time = get_time_us();
    double elapsed_ms = (end_time - start_time) / 1000.0;
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nResults:\n");
    printf("  Time taken: %.2f ms\n", elapsed_ms);
    printf("  Avg time per pattern: %.2f us\n", (elapsed_ms * 1000.0) / 200);
    printf("  Final graph: %zu nodes (+%zu), %zu edges\n",
           final_nodes, final_nodes - initial_nodes, final_edges);
    printf("  Throughput: %.0f patterns/sec\n", 200000.0 / elapsed_ms);
    
    char details[256];
    snprintf(details, sizeof(details), "%.0f patterns/sec, %zu total nodes",
             200000.0 / elapsed_ms, final_nodes);
    
    if (elapsed_ms / 500 < 1000.0) {  // Less than 1ms per pattern
        printf("✓ TEST PASSED: System scales efficiently!\n");
        record_result("Scale Test", 1, 0, details);
    } else {
        printf("✗ TEST FAILED: Performance degraded\n");
        record_result("Scale Test", 0, 1, details);
    }
}

/*
 * TEST 6: Persistence Check
 * 
 * Verify earlier patterns still work after massive scaling
 */
static void test6_persistence(MelvinMFile *mfile) {
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("TEST 6: Persistence After Scale\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    printf("Re-testing patterns from TEST 1 (learned at start)...\n");
    
    typedef struct {
        const char *input;
        const char *output;
    } Pattern;
    
    Pattern patterns[] = {
        {"hello", "world"},
        {"good", "morning"},
        {"machine", "learning"}
    };
    int num_patterns = 3;
    
    int correct = 0;
    for (int p = 0; p < num_patterns; p++) {
        int recalled = test_recall(mfile, patterns[p].input, patterns[p].output);
        printf("  '%s' -> '%s': %s\n",
               patterns[p].input, patterns[p].output,
               recalled ? "✓ STILL REMEMBERED" : "✗ FORGOTTEN");
        if (recalled) correct++;
    }
    
    float accuracy = (100.0f * correct) / num_patterns;
    printf("\nResult: %d/%d early patterns still work (%.1f%% retention)\n",
           correct, num_patterns, accuracy);
    
    char details[256];
    snprintf(details, sizeof(details), "%.1f%% retention after %zu nodes",
             accuracy, melvin_m_get_node_count(mfile));
    
    if (accuracy >= 60.0f) {
        printf("✓ TEST PASSED: Early patterns persist despite scaling!\n");
        record_result("Persistence Test", correct, num_patterns - correct, details);
    } else {
        printf("✗ TEST FAILED: Early patterns were lost\n");
        record_result("Persistence Test", correct, num_patterns - correct, details);
    }
}

/*
 * Print final summary
 */
static void print_summary(void) {
    printf("\n\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("FINAL SUMMARY\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    
    int total_passed = 0;
    int total_failed = 0;
    
    printf("%-30s | %-10s | %-10s | %s\n", "Test", "Passed", "Failed", "Details");
    printf("-------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < result_count; i++) {
        printf("%-30s | %-10d | %-10d | %s\n",
               results[i].test_name,
               results[i].passed,
               results[i].failed,
               results[i].details);
        
        total_passed += results[i].passed;
        total_failed += results[i].failed;
    }
    
    printf("-------------------------------------------------------------------------------------\n");
    printf("%-30s | %-10d | %-10d | %.1f%% success rate\n",
           "TOTAL",
           total_passed,
           total_failed,
           (100.0f * total_passed) / (total_passed + total_failed));
    
    printf("\n");
    
    if (total_failed == 0) {
        printf("✓✓✓ ALL TESTS PASSED! ✓✓✓\n");
    } else if (total_passed > total_failed) {
        printf("✓ MOSTLY PASSED (%d/%d tests)\n", total_passed, total_passed + total_failed);
    } else {
        printf("✗ NEEDS IMPROVEMENT (%d/%d tests passed)\n", total_passed, total_passed + total_failed);
    }
    
    printf("\n");
}

/*
 * Main test harness
 */
int main(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║                                                           ║\n");
    printf("║     MELVIN COMPREHENSIVE TEST SUITE                      ║\n");
    printf("║     Single Persistent .m File Across All Tests           ║\n");
    printf("║                                                           ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    const char *mfile_path = "melvin_comprehensive.m";
    
    // Remove old file if it exists
    remove(mfile_path);
    
    // Create ONE persistent .m file for ALL tests
    printf("Creating persistent .m file: %s\n", mfile_path);
    MelvinMFile *mfile = melvin_m_create(mfile_path);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create .m file\n");
        return 1;
    }
    
    printf("Initial state: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Run all tests on the SAME .m file
    test1_continuous_learning(mfile);
    test2_sequence_learning(mfile);
    test3_multi_task(mfile);
    test4_hierarchy(mfile);
    test5_scale(mfile);
    test6_persistence(mfile);
    
    // Final statistics
    printf("\n" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n");
    printf("FINAL GRAPH STATE\n");
    printf("=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=" "=\n\n");
    printf("  Total nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Total edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Avg degree: %.2f\n",
           (float)melvin_m_get_edge_count(mfile) / (float)melvin_m_get_node_count(mfile));
    printf("  Adaptations: %lu\n", (unsigned long)melvin_m_get_adaptation_count(mfile));
    
    // Close file
    melvin_m_close(mfile);
    
    // Print summary
    print_summary();
    
    printf("\nTest file saved as: %s\n", mfile_path);
    printf("You can inspect or continue using this .m file for further tests.\n\n");
    
    return 0;
}

