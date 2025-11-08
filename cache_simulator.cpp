#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;

const int MAX_SIZE = 1000;

class Entry {
public:
    Entry() : valid(false), tag(0), ref(0) {}
    ~Entry() {}
    
    void set_tag(unsigned _tag) { tag = _tag; }
    unsigned get_tag() { return tag; }
    
    void set_valid(bool _valid) { valid = _valid; }
    bool get_valid() { return valid; }
    
    void set_ref(int _ref) { ref = _ref; }
    int get_ref() { return ref; }
    
private:
    bool valid;
    unsigned tag;
    int ref;
};

class Cache {
public:
    Cache(int _num_entries, int _assoc);
    ~Cache();
    
    int get_index(unsigned long addr);
    unsigned get_tag(unsigned long addr);
    bool hit(ofstream& outfile, unsigned long addr);
    void update(unsigned long addr);
    
private:
    int assoc;
    unsigned num_entries;
    int num_sets;
    int offset_bits;
    int index_bits;
    Entry **entries;
    int global_counter;
    int find_lru_way(int set_index);
    void update_lru(int set_index, int way);
};

Cache::Cache(int _num_entries, int _assoc) {
    num_entries = _num_entries;
    assoc = _assoc;
    num_sets = num_entries / assoc;
    global_counter = 0;
    index_bits = (int)log2(num_sets);
    offset_bits = 0;
    entries = new Entry*[num_sets];
    for (int i = 0; i < num_sets; i++) {
        entries[i] = new Entry[assoc];
        for (int j = 0; j < assoc; j++) {
            entries[i][j].set_valid(false);
            entries[i][j].set_ref(0);
        }
    }
}

Cache::~Cache() {
    for (int i = 0; i < num_sets; i++) {
        delete[] entries[i];
    }
    delete[] entries;
}

int Cache::get_index(unsigned long addr) {
    unsigned long mask = (1 << index_bits) - 1;
    return (addr >> offset_bits) & mask;
}

unsigned Cache::get_tag(unsigned long addr) {
    return addr >> (index_bits + offset_bits);
}

int Cache::find_lru_way(int set_index) {
    int lru_way = 0;
    int min_ref = entries[set_index][0].get_ref();
    for (int i = 1; i < assoc; i++) {
        if (entries[set_index][i].get_ref() < min_ref) {
            min_ref = entries[set_index][i].get_ref();
            lru_way = i;
        }
    }
    return lru_way;
}

void Cache::update_lru(int set_index, int way) {
    global_counter++;
    entries[set_index][way].set_ref(global_counter);
}

bool Cache::hit(ofstream& outfile, unsigned long addr) {
    int set_index = get_index(addr);
    unsigned addr_tag = get_tag(addr);
    for (int way = 0; way < assoc; way++) {
        if (entries[set_index][way].get_valid() && 
            entries[set_index][way].get_tag() == addr_tag) {
            update_lru(set_index, way);
            outfile << addr << " : HIT" << endl;
            return true;
        }
    }
    outfile << addr << " : MISS" << endl;
    update(addr);
    return false;
}

void Cache::update(unsigned long addr) {
    int set_index = get_index(addr);
    unsigned addr_tag = get_tag(addr);
    int victim_way = -1;
    for (int way = 0; way < assoc; way++) {
        if (!entries[set_index][way].get_valid()) {
            victim_way = way;
            break;
        }
    }
    if (victim_way == -1) {
        victim_way = find_lru_way(set_index);
    }
    entries[set_index][victim_way].set_valid(true);
    entries[set_index][victim_way].set_tag(addr_tag);
    update_lru(set_index, victim_way);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << endl;
        cerr << "   ./cache_simulator num_entries associativity filename" << endl;
        return 1;
    }
    
    unsigned num_entries = atoi(argv[1]);
    unsigned assoc = atoi(argv[2]);
    string input_filename = argv[3];
    if (num_entries == 0 || assoc == 0 || num_entries % assoc != 0) {
        cerr << "Error: Invalid cache parameters" << endl;
        cerr << "num_entries must be divisible by associativity" << endl;
        return 1;
    }
    ifstream input;
    input.open(input_filename);
    if (!input.is_open()) {
        cerr << "Could not open input file " << input_filename << ". Exiting ..." << endl;
        return 1;
    }
    unsigned long* nums = new unsigned long[MAX_SIZE];
    int count = 0;
    while (!input.eof() && count < MAX_SIZE) {
        input >> nums[count];
        if (input.fail()) break;
        count++;
    }
    input.close();
    Cache cache(num_entries, assoc);
    ofstream output;
    output.open("cache_sim_output");
    for (int i = 0; i < count; i++) {
        cache.hit(output, nums[i]);
    }
    output.close();
    delete[] nums;
    return 0;
}
