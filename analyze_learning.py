#!/usr/bin/env python3
"""
Analyze Melvin Learning Rate Test Results
Computes error rate trends, learning curves, and statistics
"""

import csv
import sys
import re

def parse_results(filename):
    """Parse CSV results file"""
    results = []
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            results.append({
                'iterations': int(row['iterations']),
                'output': row['output'],
                'correct': row['correct'] == 'YES',
                'error_rate': float(row['error_rate']),
                'edges': int(row['edge_count']),
                'nodes': int(row['node_count']),
                'hierarchies': int(row['hierarchy_count'])
            })
    return results

def analyze_learning(results):
    """Analyze learning trends"""
    print("=" * 70)
    print("MELVIN LEARNING RATE ANALYSIS")
    print("=" * 70)
    print()
    
    # Group by training ranges
    early = [r for r in results if r['iterations'] <= 5]
    mid = [r for r in results if 6 <= r['iterations'] <= 20]
    late = [r for r in results if r['iterations'] > 20]
    
    print("ERROR RATE BY TRAINING PHASE:")
    print("-" * 70)
    if early:
        avg_error = sum(r['error_rate'] for r in early) / len(early)
        correct_pct = sum(1 for r in early if r['correct']) / len(early) * 100
        print(f"  Early (1-5 iterations):     {avg_error:5.1f}% avg error, {correct_pct:5.1f}% correct (n={len(early)})")
    
    if mid:
        avg_error = sum(r['error_rate'] for r in mid) / len(mid)
        correct_pct = sum(1 for r in mid if r['correct']) / len(mid) * 100
        print(f"  Mid (6-20 iterations):       {avg_error:5.1f}% avg error, {correct_pct:5.1f}% correct (n={len(mid)})")
    
    if late:
        avg_error = sum(r['error_rate'] for r in late) / len(late)
        correct_pct = sum(1 for r in late if r['correct']) / len(late) * 100
        print(f"  Late (21-100 iterations):    {avg_error:5.1f}% avg error, {correct_pct:5.1f}% correct (n={len(late)})")
    
    print()
    print("LEARNING TREND:")
    print("-" * 70)
    if len(results) >= 2:
        first_error = results[0]['error_rate']
        last_error = results[-1]['error_rate']
        trend = last_error - first_error
        
        # Find best performance
        best = min(results, key=lambda r: r['error_rate'])
        worst = max(results, key=lambda r: r['error_rate'])
        
        print(f"  Initial error rate:  {first_error:5.1f}%")
        print(f"  Final error rate:    {last_error:5.1f}%")
        print(f"  Best error rate:     {best['error_rate']:5.1f}% (at {best['iterations']} iterations)")
        print(f"  Worst error rate:    {worst['error_rate']:5.1f}% (at {worst['iterations']} iterations)")
        print(f"  Overall trend:       {trend:+.1f}% ({'IMPROVING' if trend < -5 else 'DEGRADING' if trend > 5 else 'STABLE'})")
    
    print()
    print("GRAPH GROWTH:")
    print("-" * 70)
    if results:
        first_nodes = results[0]['nodes']
        last_nodes = results[-1]['nodes']
        first_edges = results[0]['edges']
        last_edges = results[-1]['edges']
        
        print(f"  Nodes:  {first_nodes} → {last_nodes} (growth: {last_nodes - first_nodes:+d})")
        print(f"  Edges:  {first_edges} → {last_edges} (growth: {last_edges - first_edges:+d})")
        
        # Find when structure changes significantly
        for i, r in enumerate(results):
            if i > 0 and (r['nodes'] != results[i-1]['nodes'] or r['edges'] != results[i-1]['edges']):
                print(f"  Structure change at {r['iterations']} iterations: {results[i-1]['nodes']} nodes → {r['nodes']} nodes")
    
    print()
    print("OUTPUT ANALYSIS:")
    print("-" * 70)
    expected_len = 6  # " world"
    for r in results:
        output_len = len(r['output'])
        if output_len != expected_len:
            print(f"  {r['iterations']:3d} iterations: output length {output_len} (expected {expected_len}) - '{r['output']}'")
    
    print()
    print("LEARNING CURVE (Error Rate vs Training Iterations):")
    print("-" * 70)
    print("  Iterations | Error Rate | Correct | Output")
    print("  " + "-" * 66)
    for r in results:
        correct_mark = "✓" if r['correct'] else "✗"
        output_display = r['output'][:20] if len(r['output']) <= 20 else r['output'][:17] + "..."
        print(f"  {r['iterations']:10d} | {r['error_rate']:9.1f}% | {correct_mark:7s} | {output_display}")
    
    print()
    print("CONCLUSION:")
    print("-" * 70)
    
    # Determine if learning is happening
    if mid and late:
        mid_avg = sum(r['error_rate'] for r in mid) / len(mid)
        late_avg = sum(r['error_rate'] for r in late) / len(late)
        
        if late_avg < mid_avg - 2:
            print("  ✓ Learning is IMPROVING: Error rate decreases with more training")
        elif late_avg > mid_avg + 2:
            print("  ✗ Learning is DEGRADING: Error rate increases with more training")
            print("     Possible causes: Overfitting, hierarchy formation issues, or context matching problems")
        else:
            print("  → Learning is STABLE: Error rate remains consistent")
    
    # Check for optimal training point
    if results:
        best = min(results, key=lambda r: r['error_rate'])
        if best['error_rate'] < 5.0:
            print(f"  → Optimal training point: {best['iterations']} iterations (error: {best['error_rate']:.1f}%)")
    
    print()

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: ./analyze_learning.py <results.csv>")
        sys.exit(1)
    
    results = parse_results(sys.argv[1])
    analyze_learning(results)

