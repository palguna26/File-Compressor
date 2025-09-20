# File Compressor

A multi-threaded file compression tool that uses Huffman coding for efficient data compression.

## Features

- Parallel compression using multiple worker threads
- Configurable chunk size for processing large files
- Custom thread-safe queue implementation
- Huffman coding for optimal compression
- Binary file support

## Requirements

- C++ compiler with C++17 support
- MinGW-w64 (for Windows users)
- Standard Template Library (STL)
- Support for multi-threading (`<thread>`, `<mutex>`)

## Build Instructions

Using g++:

```bash
g++ -std=c++17 main.cpp -o file_compressor
```

## Usage

1. Run the compiled executable
2. Follow the prompts to enter:
   - Input file path
   - Output file path
   - Number of threads (default: 4)
   - Chunk size in MB (default: 1)

Example:
```bash
./file_compressor
Enter input file path: input.txt
Enter output file path: compressed.bin
Enter number of threads (default 4): 8
Enter chunk size in MB (default 1): 2
```

## How It Works

### 1. Multi-threaded Architecture
- Uses a thread-safe queue for distributing work
- Configurable number of worker threads
- Chunk-based parallel processing

### 2. Compression Algorithm
- **Huffman Coding Implementation:**
  - Character frequency analysis
  - Dynamic Huffman tree construction
  - Variable-length code generation
  - Efficient bit-packing

### 3. File Processing
- Input file is split into configurable chunks
- Each chunk is processed independently
- Maintains chunk order for final output
- Stores compression metadata per chunk

## Technical Implementation

### Key Components

1. **SafeQueue Template Class**
   - Thread-safe operations
   - Condition variables for synchronization
   - Generic implementation for different data types

2. **Huffman Tree**
   - Custom node structure
   - Priority queue-based construction
   - Efficient code generation

3. **File I/O**
   - Binary file handling
   - Metadata storage
   - Chunk-based processing

## Performance Considerations

- **Threading:** Performance scales with number of CPU cores
- **Memory Usage:** Controlled by chunk size configuration
- **Compression Ratio:** Depends on input data patterns
- **Disk I/O:** Buffered reading and writing for efficiency

## Current Limitations

- Compression-only implementation (no decompression yet)
- Maximum file size depends on available system memory
- Compression effectiveness varies with input data type
- Each chunk requires its own Huffman table

## Future Improvements

- Add decompression functionality
- Implement stream processing for larger files
- Add compression ratio statistics
- Support for different compression algorithms
- Progress bar and status updates