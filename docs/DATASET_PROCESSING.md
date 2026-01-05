# Dataset Processing Guide

## Overview

Melvin provides universal dataset processing tools that automatically detect file types and route data to the correct ports. Feed any dataset (text, audio, image, video, code) and watch the system learn.

## Quick Start

### Basic Usage

```bash
# Process any dataset - auto-detects type
./melvin_feed dataset.txt brain.m
./melvin_feed audio.wav brain.m
./melvin_feed image.jpg brain.m
./melvin_feed code.c brain.m
```

The tool automatically:
- Detects file type (extension + magic bytes)
- Routes to correct port (TEXT/AUDIO/VIDEO)
- Processes with chunked handling for large files
- Shows real-time progress
- Saves checkpoints automatically

## Tools

### `melvin_feed` - Universal Dataset Processor

Processes any dataset through the Melvin pipeline.

**Usage:**
```bash
./melvin_feed <dataset_file> <brain.m> [options]
```

**Options:**
- `--port PORT_ID` - Force port (0x01=text, 0x02=audio, 0x03=video, etc.)
- `--passes N` - Number of passes (default: 1, use 2-3 for deeper hierarchies)
- `--watch` - Watch mode (detailed statistics)
- `--help` - Show help message

**Examples:**
```bash
# Simple processing
./melvin_feed corpus.txt brain.m

# Multiple passes (build deeper hierarchies)
./melvin_feed corpus.txt brain.m --passes 3

# Watch mode (detailed stats)
./melvin_feed corpus.txt brain.m --watch

# Force port (override auto-detection)
./melvin_feed data.bin brain.m --port 0x01
```

**Supported File Types:**

| Type | Extensions | Port |
|------|------------|------|
| Text | .txt, .md, .json, .csv, .xml, .html, .log | 0x01 (TEXT) |
| Audio | .wav, .mp3, .flac, .ogg, .aac, .m4a | 0x02 (AUDIO) |
| Image | .jpg, .jpeg, .png, .gif, .bmp, .tiff, .webp | 0x03 (VIDEO) |
| Video | .mp4, .avi, .mov, .mkv, .webm, .flv | 0x03 (VIDEO) |
| Code | .c, .cpp, .h, .py, .js, .java, .go, .rs, etc. | 0x01 (TEXT) |

**Auto-Detection:**
- First checks file extension
- Falls back to magic bytes (file signature)
- Defaults to TEXT port if unknown

### `melvin_watch` - Real-Time Brain Monitor

Watches brain file and displays real-time statistics.

**Usage:**
```bash
./melvin_watch <brain.m> [--interval SECONDS]
```

**Options:**
- `--interval N` - Refresh interval in seconds (default: 1)
- `--help` - Show help message

**Examples:**
```bash
# Watch with default 1-second refresh
./melvin_watch brain.m

# Watch with 2-second refresh
./melvin_watch brain.m --interval 2
```

**Display:**
- Current time
- Brain file path and size
- Node count (with growth rate)
- Edge count (with growth rate)
- Adaptations count
- Edges/Node ratio
- Growth percentages
- Processing status

## Foundation Building

### Building Text Foundation

```bash
# Process large text corpus
./melvin_feed wikipedia_dump.txt brain.m --passes 3

# Process multiple files
for file in corpus/*.txt; do
    ./melvin_feed "$file" brain.m --passes 2
done
```

### Building Multi-Modal Foundation

```bash
# Text corpus
./melvin_feed text_corpus.txt brain.m --passes 3

# Audio dataset
./melvin_feed audio_samples.wav brain.m --passes 2

# Image dataset
./melvin_feed images.jpg brain.m --passes 2

# System learns cross-modal associations automatically
```

### Progressive Learning

```bash
# Stage 1: Common patterns (top 1000 words)
./melvin_feed top_1000_words.txt brain.m --passes 3

# Stage 2: Common phrases
./melvin_feed top_10000_phrases.txt brain.m --passes 2

# Stage 3: Full corpus (builds on Stage 1 & 2)
./melvin_feed full_corpus.txt brain.m --passes 2
```

## Large File Processing

The tools automatically use chunked processing for files > 100MB:

```bash
# Process 40GB file (automatically chunked)
./melvin_feed large_dataset.txt brain.m

# Shows progress every 100 chunks or 100MB
# Processing: 45.2% (1.1 GB / 2.45 GB, 1123 chunks)
```

**Features:**
- Memory-efficient (1MB chunks by default)
- Progress tracking
- Checkpoint saving
- Handles files of any size

## Batch Processing

### Process Entire Directory

```bash
# Process all text files
find datasets/ -name "*.txt" | while read file; do
    ./melvin_feed "$file" brain.m
done

# Process with multiple passes
find datasets/ -name "*.txt" | while read file; do
    ./melvin_feed "$file" brain.m --passes 2
done
```

### Process Multiple Types

```bash
# Text files
for file in datasets/text/*.txt; do
    ./melvin_feed "$file" brain.m
done

# Audio files
for file in datasets/audio/*.wav; do
    ./melvin_feed "$file" brain.m
done

# Image files
for file in datasets/images/*.jpg; do
    ./melvin_feed "$file" brain.m
done
```

## Monitoring

### Watch While Processing

**Terminal 1:**
```bash
./melvin_feed large_dataset.txt brain.m --watch
```

**Terminal 2:**
```bash
./melvin_watch brain.m
```

### Watch Multiple Brains

```bash
# Terminal 1: Watch brain1.m
./melvin_watch brain1.m

# Terminal 2: Watch brain2.m
./melvin_watch brain2.m
```

## Output Example

```
$ ./melvin_feed corpus.txt brain.m --passes 3 --watch

Auto-detected: corpus.txt -> Text (Port 0x01)
Loaded existing brain: brain.m (1234 nodes, 5678 edges)
╔══════════════════════════════════════════════════════════╗
║ Melvin Dataset Processor                                  ║
╠══════════════════════════════════════════════════════════╣
║ File:    corpus.txt                                     ║
║ Size:    2.45 GB                                        ║
║ Port:    0x01 (Text                                    ) ║
║ Passes:  3                                              ║
╚══════════════════════════════════════════════════════════╝

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Pass 1/3
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Processing: 45.2% (1.1 GB / 2.45 GB, 1123 chunks)

Progress:
  Nodes:      1,234,567 (+45,321)
  Edges:      2,345,678 (+123,456)
  Adaptations: 1,234 (+1)
  Rate:       12.5 MB/s
  Time:       88.3 seconds
  ✓ Checkpoint saved

Statistics:
  Node growth:    3.78%
  Edge growth:    5.56%
  Edges/Node:     1.90

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Complete
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Final: 1,234,567 nodes, 2,345,678 edges
```

## Tips

1. **Multiple Passes**: Use 2-3 passes to build deeper hierarchies (10:1 compression per level)

2. **Watch Mode**: Use `--watch` for detailed statistics during processing

3. **Checkpoints**: System auto-saves after each pass (safe to interrupt)

4. **Large Files**: Automatically uses chunked processing (no manual configuration needed)

5. **Port Override**: Use `--port` to force a specific port if auto-detection fails

6. **Batch Processing**: Process multiple files in sequence to build comprehensive foundation

7. **Real-Time Monitoring**: Use `melvin_watch` in separate terminal to monitor progress

## Troubleshooting

**File type not detected:**
```bash
# Force port manually
./melvin_feed data.bin brain.m --port 0x01
```

**Large file processing slow:**
- Normal for first pass (building patterns)
- Subsequent passes are faster (using existing hierarchies)
- Use `--passes 1` for initial processing, add passes later

**Memory issues:**
- System uses chunked processing automatically
- No manual memory management needed
- Checkpoints allow resuming if interrupted

## Integration with Pipeline

The tools are fully compatible with the existing pipeline:

- Uses `melvin_in_port_handle_text_file()` for small files
- Uses `melvin_in_port_handle_text_file_chunked()` for large files
- Sets `last_input_port_id` automatically
- Nodes get `port_id` metadata
- Output filtering works automatically
- Cross-modal learning enabled

## Summary

The dataset processing tools provide:
- ✅ Universal file type detection
- ✅ Automatic port routing
- ✅ Progress monitoring
- ✅ Large file support (chunked)
- ✅ Multi-pass processing
- ✅ Real-time statistics
- ✅ Checkpoint saving
- ✅ Simple command-line interface

Feed any dataset and watch Melvin learn!

