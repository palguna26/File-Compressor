#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <filesystem>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

// --------------------- PATH HANDLING ---------------------
string normalizePath(string path) {
    // Remove quotes if present
    if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
        path = path.substr(1, path.size() - 2);
    }
    
    // Replace backslashes with forward slashes
    replace(path.begin(), path.end(), '\\', '/');
    
    // Remove any trailing slashes
    while (!path.empty() && (path.back() == '/' || path.back() == '\\')) {
        path.pop_back();
    }
    
    // Convert to absolute path if relative
    if (!fs::path(path).is_absolute()) {
        path = fs::absolute(fs::path(path)).string();
    }
    
    return path;
}

// --------------------- THREAD-SAFE QUEUE ---------------------
template<typename T>
class SafeQueue {
private:
    queue<T> q;
    mutex mtx;
    condition_variable cv;
public:
    void push(T item) {
        unique_lock<mutex> lock(mtx);
        q.push(item);
        cv.notify_one();
    }

    T pop() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]{ return !q.empty(); });
        T item = q.front();
        q.pop();
        return item;
    }

    bool empty() {
        unique_lock<mutex> lock(mtx);
        return q.empty();
    }
};

// --------------------- HUFFMAN TREE ---------------------
struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

// Build Huffman Tree
HuffmanNode* buildHuffmanTree(const unordered_map<char,int> &freqMap) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> pq;
    for (auto &[c,f] : freqMap) pq.push(new HuffmanNode(c,f));
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    return pq.top();
}

// Generate Huffman Codes
void generateCodes(HuffmanNode* root, string code, unordered_map<char,string> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->ch] = code;
    }
    generateCodes(root->left, code+"0", codes);
    generateCodes(root->right, code+"1", codes);
}

// Compress a chunk
vector<char> compressChunk(const vector<char> &chunk, unordered_map<char,string> &outCodes) {
    unordered_map<char,int> freqMap;
    for (char c : chunk) freqMap[c]++;
    
    HuffmanNode* root = buildHuffmanTree(freqMap);
    unordered_map<char,string> codes;
    generateCodes(root, "", codes);
    outCodes = codes;

    string bitString = "";
    for (char c : chunk) bitString += codes[c];

    // Convert bit string to bytes
    vector<char> compressed;
    for (size_t i=0; i<bitString.size(); i+=8) {
        string byteStr = bitString.substr(i, min((size_t)8, bitString.size()-i));
        while (byteStr.size() < 8) byteStr += '0';
        compressed.push_back(stoi(byteStr, nullptr, 2));
    }
    return compressed;
}

// --------------------- WORKER THREAD ---------------------
void worker(SafeQueue<pair<int, vector<char>>> &inputQueue,
            SafeQueue<pair<int, pair<vector<char>, unordered_map<char,string>>>> &outputQueue) {
    while (true) {
        auto chunkPair = inputQueue.pop();
        if (chunkPair.second.empty()) break; // termination signal
        unordered_map<char,string> codes;
        auto compressed = compressChunk(chunkPair.second, codes);
        outputQueue.push({chunkPair.first, {compressed, codes}});
    }
}

// --------------------- FILE IO ---------------------
vector<char> readFileChunk(ifstream &file, size_t chunkSize) {
    vector<char> buffer(chunkSize);
    file.read(buffer.data(), chunkSize);
    buffer.resize(file.gcount());
    return buffer;
}

// Write Huffman metadata to file
void writeMetadata(ofstream &outFile, unordered_map<char,string> &codes) {
    uint32_t mapSize = codes.size();
    outFile.write(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    for (auto &[c, code] : codes) {
        outFile.write(&c, 1);
        uint32_t codeLen = code.size();
        outFile.write(reinterpret_cast<char*>(&codeLen), sizeof(codeLen));
        outFile.write(code.c_str(), codeLen);
    }
}

// --------------------- DECOMPRESSION ---------------------
// Read Huffman metadata from file
unordered_map<string,char> readMetadata(ifstream &inFile) {
    unordered_map<string,char> codes;
    uint32_t mapSize;
    inFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    
    for (uint32_t i = 0; i < mapSize; i++) {
        char c;
        uint32_t codeLen;
        inFile.read(&c, 1);
        inFile.read(reinterpret_cast<char*>(&codeLen), sizeof(codeLen));
        string code(codeLen, '\0');
        inFile.read(&code[0], codeLen);
        codes[code] = c;
    }
    return codes;
}

// Convert bytes to bit string
string bytesToBitString(const vector<char>& bytes) {
    string bits;
    for (unsigned char byte : bytes) {
        for (int i = 7; i >= 0; i--) {
            bits += ((byte >> i) & 1) ? '1' : '0';
        }
    }
    return bits;
}

// Decompress a chunk
vector<char> decompressChunk(const vector<char>& compressedChunk, const unordered_map<string,char>& codes) {
    string bitString = bytesToBitString(compressedChunk);
    vector<char> decompressed;
    string currentCode;
    
    for (char bit : bitString) {
        currentCode += bit;
        if (codes.count(currentCode)) {
            decompressed.push_back(codes.at(currentCode));
            currentCode.clear();
        }
    }
    return decompressed;
}

// Worker thread for decompression
void decompressWorker(SafeQueue<pair<int, pair<vector<char>, unordered_map<string,char>>>>& inputQueue,
                     SafeQueue<pair<int, vector<char>>>& outputQueue) {
    while (true) {
        auto chunk = inputQueue.pop();
        if (chunk.second.first.empty()) break; // termination signal
        auto decompressed = decompressChunk(chunk.second.first, chunk.second.second);
        outputQueue.push({chunk.first, decompressed});
    }
}

// --------------------- MAIN ---------------------
int main() {
    string inputFile, outputFile;
    string mode;
    int NUM_THREADS = 4;
    size_t CHUNK_SIZE = 1024 * 1024; // 1 MB default

    // ------------------ TERMINAL INPUT ------------------
    cout << "Enter mode (c for compress, d for decompress): ";
    getline(cin, mode);
    if (mode != "c" && mode != "d") {
        cerr << "Invalid mode. Use 'c' for compress or 'd' for decompress.\n";
        return 1;
    }

    cout << "Enter input file path: ";
    getline(cin, inputFile);
    inputFile = normalizePath(inputFile);

    cout << "Enter output file path: ";
    getline(cin, outputFile);
    outputFile = normalizePath(outputFile);

    // Validate input file exists
    if (!fs::exists(inputFile)) {
        cerr << "Input file does not exist.\n";
        return 1;
    }

    // Check if input is a directory
    if (fs::is_directory(inputFile)) {
        cerr << "Input path is a directory. Please provide a file path.\n";
        return 1;
    }

    cout << "Enter number of threads (default 4): ";
    string threadsInput;
    getline(cin, threadsInput);
    if (!threadsInput.empty()) NUM_THREADS = stoi(threadsInput);

    cout << "Enter chunk size in MB (default 1): ";
    string chunkInput;
    getline(cin, chunkInput);
    if (!chunkInput.empty()) CHUNK_SIZE = stoi(chunkInput) * 1024 * 1024;

    // Get input file size for statistics
    uintmax_t inputFileSize = fs::file_size(inputFile);

    // ------------------ VALIDATE FILES ------------------
    ifstream inFile(inputFile, ios::binary);
    if (!inFile.is_open()) { cerr << "Cannot open input file\n"; return 1; }
    ofstream outFile(outputFile, ios::binary);
    if (!outFile.is_open()) { cerr << "Cannot open output file\n"; return 1; }

    // Start timing
    auto startTime = high_resolution_clock::now();

    if (mode == "c") {
        cout << "\nStarting compression with " << NUM_THREADS 
             << " threads and chunk size " << CHUNK_SIZE/(1024*1024) << " MB...\n";

        SafeQueue<pair<int, vector<char>>> inputQueue;
        SafeQueue<pair<int, pair<vector<char>, unordered_map<char,string>>>> outputQueue;

        // ------------------ START WORKER THREADS ------------------
        vector<thread> threads;
        for (int i=0; i<NUM_THREADS; i++)
            threads.emplace_back(worker, ref(inputQueue), ref(outputQueue));

        // ------------------ READ FILE AND PUSH CHUNKS ------------------
        int chunkId = 0;
        while (!inFile.eof()) {
            vector<char> chunk = readFileChunk(inFile, CHUNK_SIZE);
            if (!chunk.empty()) {
                inputQueue.push({chunkId, chunk});
                cout << "Processing chunk " << chunkId << " (" 
                     << (float)inFile.tellg() / inputFileSize * 100 << "% complete)\n";
                chunkId++;
            }
        }

        // ------------------ TERMINATE THREADS ------------------
        for (int i=0; i<NUM_THREADS; i++) inputQueue.push({-1, {}});

        // ------------------ JOIN THREADS ------------------
        for (auto &t : threads) t.join();

        // ------------------ WRITE COMPRESSED FILE ------------------
        vector<pair<vector<char>, unordered_map<char,string>>> compressedChunks(chunkId);
        while (!outputQueue.empty()) {
            auto [id, data] = outputQueue.pop();
            compressedChunks[id] = data;
        }

        for (auto &[chunk, codes] : compressedChunks) {
            writeMetadata(outFile, codes);
            uint32_t size = chunk.size();
            outFile.write(reinterpret_cast<char*>(&size), sizeof(size));
            outFile.write(chunk.data(), chunk.size());
        }
    } else { // Decompression mode
        cout << "\nStarting decompression with " << NUM_THREADS << " threads...\n";

        SafeQueue<pair<int, pair<vector<char>, unordered_map<string,char>>>> inputQueue;
        SafeQueue<pair<int, vector<char>>> outputQueue;

        // Start decompression threads
        vector<thread> threads;
        for (int i=0; i<NUM_THREADS; i++)
            threads.emplace_back(decompressWorker, ref(inputQueue), ref(outputQueue));

        // Read and process chunks
        int chunkId = 0;
        while (inFile.peek() != EOF) {
            // Read metadata for this chunk
            auto codes = readMetadata(inFile);
            
            // Read compressed chunk
            uint32_t chunkSize;
            inFile.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
            vector<char> compressedChunk(chunkSize);
            inFile.read(compressedChunk.data(), chunkSize);

            cout << "Decompressing chunk " << chunkId << " (" 
                 << (float)inFile.tellg() / inputFileSize * 100 << "% complete)\n";

            inputQueue.push({chunkId++, {compressedChunk, codes}});
        }

        // Signal threads to terminate
        for (int i=0; i<NUM_THREADS; i++)
            inputQueue.push({-1, {{}, {}}});

        // Wait for threads to finish
        for (auto &t : threads) t.join();

        // Write decompressed chunks in order
        vector<vector<char>> decompressedChunks(chunkId);
        while (!outputQueue.empty()) {
            auto [id, chunk] = outputQueue.pop();
            decompressedChunks[id] = chunk;
        }

        for (const auto &chunk : decompressedChunks) {
            outFile.write(chunk.data(), chunk.size());
        }
    }

    // Calculate and display statistics
    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(endTime - startTime);
    uintmax_t outputFileSize = fs::file_size(outputFile);
    
    cout << "\n-------- Operation Statistics --------\n";
    cout << "Operation: " << (mode == "c" ? "Compression" : "Decompression") << "\n";
    cout << "Input file size: " << inputFileSize << " bytes\n";
    cout << "Output file size: " << outputFileSize << " bytes\n";
    
    if (mode == "c") {
        double ratio = (1.0 - static_cast<double>(outputFileSize) / inputFileSize) * 100;
        cout << "Compression ratio: " << fixed << setprecision(2) << ratio << "%\n";
    }
    
    cout << "Processing time: " << duration.count() / 1000.0 << " seconds\n";
    cout << "Threads used: " << NUM_THREADS << "\n";
    cout << "Chunk size: " << CHUNK_SIZE / (1024 * 1024) << " MB\n";
    cout << "----------------------------------\n\n";
    
    cout << "Operation completed successfully! Output file: " << outputFile << endl;
    return 0;
}
