# File Compressor

A multi-threaded file compression and decompression tool that uses Huffman coding for efficient data compression, featuring real-time progress tracking and detailed performance statistics.

## Features

- Complete compression and decompression support
- Parallel processing using multiple worker threads
- Real-time progress tracking with percentage completion
- Detailed compression statistics and performance metrics
- Smart path handling with support for relative/absolute paths
- Robust error handling and input validation
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
   - Operation mode (compress/decompress)
   - Input file path
   - Output file path
   - Number of threads (default: 4)
   - Chunk size in MB (default: 1)

Example Compression:
```bash
./file_compressor
Enter mode (c for compress, d for decompress): c
Enter input file path: input.txt
Enter output file path: compressed.bin
Enter number of threads (default 4): 8
Enter chunk size in MB (default 1): 2
```

Example Decompression:
```bash
./file_compressor
Enter mode (c for compress, d for decompress): d
Enter input file path: compressed.bin
Enter output file path: decompressed.txt
Enter number of threads (default 4): 8
Enter chunk size in MB (default 1): 2
```

The program will display progress information and final statistics:
```
Processing chunk 5 (42.3% complete)
...
-------- Operation Statistics --------
Operation: Compression
Input file size: 1048576 bytes
Output file size: 524288 bytes
Compression ratio: 50.00%
Processing time: 1.23 seconds
Threads used: 8
Chunk size: 2 MB
----------------------------------
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

## Error Handling

The program includes comprehensive error checking for:
- File existence validation
- Directory vs file validation
- File access permissions
- Invalid mode selection
- Path normalization (handles both relative and absolute paths)
- Proper handling of quoted paths and different slash types

## Performance Features

### Progress Tracking
- Real-time progress updates for each chunk
- Percentage completion display
- Active thread status monitoring

### Statistics Reporting
- Input and output file sizes
- Compression/decompression ratio
- Processing time
- Thread utilization
- Chunk size information

## Current Limitations

- Maximum file size depends on available system memory
- Compression effectiveness varies with input data type
- Each chunk requires its own Huffman table
- Decompression requires the complete compressed file

## Future Improvements

- Stream processing for handling larger files
- Alternative compression algorithms
- Directory compression support
- Compression level options
- User interface improvements

- Add decompression functionality
- Implement stream processing for larger files
- Add compression ratio statistics
- Support for different compression algorithms
- Progress bar and status updates