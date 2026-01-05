# Critical Fixes Needed for Output Generation

## 1. By-Value Return Ownership (CRITICAL)

Current code has shallow copy issue:
```c
WaveBPTTState new_state = wave_propagate_with_bptt(...);
*current_wave_state = new_state;  // Shallow copy of pointers!
memset(&new_state, 0, sizeof(WaveBPTTState));
```

Fix: Use tmp->move->zero pattern:
```c
WaveBPTTState tmp = wave_propagate_with_bptt(...);
if (current_wave_state != state) {
    free_wave_bptt_state(current_wave_state);
    free(current_wave_state);
}
current_wave_state = malloc(sizeof(WaveBPTTState));
*current_wave_state = tmp;
memset(&tmp, 0, sizeof(tmp));  // tmp no longer owns
```

## 2. Entropy Stop Condition

Replace stability counter with effective support N_eff = 1 / sum(p_i^2)

## 3. Normalization Consistency

Ensure weights are normalized: p_i = w_i / sum_w

## 4. Port Payload Protection

Add guards to prevent port node payload overwrite during wave propagation

## 5. Distribution Size Explosion

Add adaptive capping based on local activation threshold

