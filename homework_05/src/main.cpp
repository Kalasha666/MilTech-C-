#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "telemetry.hpp"


int main(int argc, char** argv) {
    // The executable expects exactly one telemetry log path.
    if (argc != 2) {
        std::cerr << "usage: telemetry_check <input_path>\n";
        return 1;
    }

    int count = calculate_total_frames_count(argv[1]);
    Frame* frames = count > 0 ? new Frame[count] : nullptr;
    read_frames(argv[1], frames, count);
    Summary summary = summarize(frames, count);
    delete[] frames;
    print_summary(summary);

    return 0;
}