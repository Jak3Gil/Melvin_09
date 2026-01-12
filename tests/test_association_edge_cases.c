#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_output(const char *label, uint8_t *output, size_t size) {
    printf("%s (%zu bytes): '", label, size);
    for (size_t i = 0; i < size && i < 100; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("'\n");
}

void test_scenario(const char *scenario_name, MelvinMFile *mfile, const char *test_input) {
    printf("\n--- %s ---\n", scenario_name);
    printf("Input: '%s'\n", test_input);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            print_output("Output", output, output_size);
            free(output);
        }
    } else {
        printf("Output: (no output generated)\n");
    }
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║         ASSOCIATION LEARNING - EDGE CASE TESTS                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // ========================================================================
    // TEST 1: Novel Input (Never Seen Before)
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 1: COMPLETELY NOVEL INPUT\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_novel.m");
    MelvinMFile *mfile1 = melvin_m_create("test_novel.m");
    
    printf("\nTraining with: 'hello world' and 'cat meow'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile1, 0, (uint8_t*)"hello world", 11);
        melvin_in_port_handle_buffer(mfile1, 0, (uint8_t*)"cat meow", 8);
    }
    melvin_m_save(mfile1);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile1), melvin_m_get_edge_count(mfile1));
    
    test_scenario("Novel word 'zebra'", mfile1, "zebra");
    test_scenario("Novel word 'xyz'", mfile1, "xyz");
    test_scenario("Novel numbers '12345'", mfile1, "12345");
    
    melvin_m_close(mfile1);
    
    // ========================================================================
    // TEST 2: Partial Matches (Seen Some, Not All)
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 2: PARTIAL MATCHES\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_partial.m");
    MelvinMFile *mfile2 = melvin_m_create("test_partial.m");
    
    printf("\nTraining with: 'hello world' and 'hello friend'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile2, 0, (uint8_t*)"hello world", 11);
        melvin_in_port_handle_buffer(mfile2, 0, (uint8_t*)"hello friend", 12);
    }
    melvin_m_save(mfile2);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile2), melvin_m_get_edge_count(mfile2));
    
    test_scenario("Shared prefix 'hello'", mfile2, "hello");
    test_scenario("Shared prefix 'hello ' (with space)", mfile2, "hello ");
    test_scenario("Partial 'hel'", mfile2, "hel");
    test_scenario("Partial 'hell'", mfile2, "hell");
    test_scenario("Novel suffix 'hello stranger'", mfile2, "hello stranger");
    
    melvin_m_close(mfile2);
    
    // ========================================================================
    // TEST 3: Mixed Patterns (Pieces from Different Inputs)
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 3: MIXED PATTERNS FROM DIFFERENT INPUTS\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_mixed.m");
    MelvinMFile *mfile3 = melvin_m_create("test_mixed.m");
    
    printf("\nTraining with: 'apple pie', 'apple juice', 'orange juice'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile3, 0, (uint8_t*)"apple pie", 9);
        melvin_in_port_handle_buffer(mfile3, 0, (uint8_t*)"apple juice", 11);
        melvin_in_port_handle_buffer(mfile3, 0, (uint8_t*)"orange juice", 12);
    }
    melvin_m_save(mfile3);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile3), melvin_m_get_edge_count(mfile3));
    
    test_scenario("Shared word 'apple'", mfile3, "apple");
    test_scenario("Shared word 'juice'", mfile3, "juice");
    test_scenario("Novel combo 'orange pie'", mfile3, "orange pie");
    test_scenario("Partial 'app'", mfile3, "app");
    test_scenario("Partial 'jui'", mfile3, "jui");
    
    melvin_m_close(mfile3);
    
    // ========================================================================
    // TEST 4: Substring Confusion
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 4: SUBSTRING CONFUSION\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_substring.m");
    MelvinMFile *mfile4 = melvin_m_create("test_substring.m");
    
    printf("\nTraining with: 'the cat', 'the cathedral', 'cat nap'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile4, 0, (uint8_t*)"the cat", 7);
        melvin_in_port_handle_buffer(mfile4, 0, (uint8_t*)"the cathedral", 13);
        melvin_in_port_handle_buffer(mfile4, 0, (uint8_t*)"cat nap", 7);
    }
    melvin_m_save(mfile4);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile4), melvin_m_get_edge_count(mfile4));
    
    test_scenario("Ambiguous 'the'", mfile4, "the");
    test_scenario("Ambiguous 'cat'", mfile4, "cat");
    test_scenario("Ambiguous 'the c'", mfile4, "the c");
    test_scenario("Ambiguous 'the ca'", mfile4, "the ca");
    test_scenario("Ambiguous 'the cat'", mfile4, "the cat");
    
    melvin_m_close(mfile4);
    
    // ========================================================================
    // TEST 5: Single Character Inputs
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 5: SINGLE CHARACTER INPUTS\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_single.m");
    MelvinMFile *mfile5 = melvin_m_create("test_single.m");
    
    printf("\nTraining with: 'abc', 'def', 'ghi'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile5, 0, (uint8_t*)"abc", 3);
        melvin_in_port_handle_buffer(mfile5, 0, (uint8_t*)"def", 3);
        melvin_in_port_handle_buffer(mfile5, 0, (uint8_t*)"ghi", 3);
    }
    melvin_m_save(mfile5);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile5), melvin_m_get_edge_count(mfile5));
    
    test_scenario("Single char 'a'", mfile5, "a");
    test_scenario("Single char 'd'", mfile5, "d");
    test_scenario("Single char 'g'", mfile5, "g");
    test_scenario("Two chars 'ab'", mfile5, "ab");
    test_scenario("Two chars 'de'", mfile5, "de");
    test_scenario("Novel single 'x'", mfile5, "x");
    
    melvin_m_close(mfile5);
    
    // ========================================================================
    // TEST 6: Overlapping Sequences
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 6: OVERLAPPING SEQUENCES\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_overlap.m");
    MelvinMFile *mfile6 = melvin_m_create("test_overlap.m");
    
    printf("\nTraining with: 'abcd', 'bcde', 'cdef'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile6, 0, (uint8_t*)"abcd", 4);
        melvin_in_port_handle_buffer(mfile6, 0, (uint8_t*)"bcde", 4);
        melvin_in_port_handle_buffer(mfile6, 0, (uint8_t*)"cdef", 4);
    }
    melvin_m_save(mfile6);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile6), melvin_m_get_edge_count(mfile6));
    
    test_scenario("Overlap 'a'", mfile6, "a");
    test_scenario("Overlap 'b'", mfile6, "b");
    test_scenario("Overlap 'c'", mfile6, "c");
    test_scenario("Overlap 'd'", mfile6, "d");
    test_scenario("Overlap 'bc'", mfile6, "bc");
    test_scenario("Overlap 'cd'", mfile6, "cd");
    
    melvin_m_close(mfile6);
    
    // ========================================================================
    // TEST 7: Empty and Whitespace
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 7: EMPTY AND WHITESPACE INPUTS\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_empty.m");
    MelvinMFile *mfile7 = melvin_m_create("test_empty.m");
    
    printf("\nTraining with: 'hello world'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile7, 0, (uint8_t*)"hello world", 11);
    }
    melvin_m_save(mfile7);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile7), melvin_m_get_edge_count(mfile7));
    
    test_scenario("Just space ' '", mfile7, " ");
    test_scenario("Multiple spaces '   '", mfile7, "   ");
    test_scenario("Tab character", mfile7, "\t");
    test_scenario("Newline character", mfile7, "\n");
    
    melvin_m_close(mfile7);
    
    // ========================================================================
    // TEST 8: Longer Context Windows
    // ========================================================================
    printf("\n\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST 8: LONGER CONTEXT WINDOWS\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_long.m");
    MelvinMFile *mfile8 = melvin_m_create("test_long.m");
    
    printf("\nTraining with: 'the quick brown fox jumps'\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile8, 0, (uint8_t*)"the quick brown fox jumps", 25);
    }
    melvin_m_save(mfile8);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile8), melvin_m_get_edge_count(mfile8));
    
    test_scenario("Start 'the'", mfile8, "the");
    test_scenario("Start 'the quick'", mfile8, "the quick");
    test_scenario("Middle 'brown'", mfile8, "brown");
    test_scenario("Middle 'fox'", mfile8, "fox");
    test_scenario("Almost complete 'the quick brown fox'", mfile8, "the quick brown fox");
    test_scenario("Novel middle 'the quick red'", mfile8, "the quick red");
    
    melvin_m_close(mfile8);
    
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                   ALL EDGE CASE TESTS COMPLETE                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
