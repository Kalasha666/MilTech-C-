#include "telemetry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.
int frame_count = 0;
int frame_valid_count = 0;

int calculate_total_frames_count(const char* path)
{
    std::ifstream input{path};

    if (!input) {
        std::cerr << "error: failed to open input file: " << path << '\n';
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    frame_count = 0;

    while (input.getline(line, MAX_LINE_LENGTH))
    {
        if (line[0] == '\0') continue;

        frame_count++;
    }

    input.clear();
    input.seekg(0);

    return frame_count;
}

int calculate_valid_frames_count(const Frame frames[], int count)
{
    frame_valid_count = 0;

    for (int i = 0; i < count; ++i) {
        if (frames[i].valid) {
            ++frame_valid_count;
        }
    }

    return frame_valid_count;
}

int split_line(char line[], char* fields[], int max_fields) {
    int count = 0;
    char* cursor = line;

    while (*cursor != '\0' && count < max_fields) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') {
            break;
        }

        fields[count] = cursor;
        ++count;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' &&
               *cursor != '\r') {
            ++cursor;
        }
    }

    return count;
}

bool parse_long(const char* text, long& value) {
    char* end = nullptr;
    value = std::strtol(text, &end, 10);

    if (end == text)
    {
        std::cerr << "error: invalid fields value: " << end << "\n";
        value = 0;

        return false;
    }

    return true;
}

bool parse_int(const char* text, int& value) {

    char* end = nullptr;
    value = std::strtol(text, &end, 10);

    if (end == text)
    {
        std::cerr << "error: invalid fields value: " << end << "\n";
        value = 0;
        return false;
    }
    else
    {
        value = static_cast<int>(value);
    }

    return true;
}

bool parse_double(const char* text, double& value) {
    char* end = nullptr;
    value = std::strtod(text, &end);

    if (end == text) {
        std::cerr << "error: invalid fields value: " << end << "\n";
        value = 0.0;
        return false;
    }

    return true;
}

Frame parse_frame(char line[]) {
    char* fields[EXPECTED_FIELD_COUNT] = {};
    const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);
    Frame frame{};
    frame.valid = true;
    bool timestamp_ms_result = parse_long(fields[0], frame.timestamp_ms);
    bool seq_result = parse_int(fields[1], frame.seq);
    bool voltage_v_result = parse_double(fields[2], frame.voltage_v);

    if(voltage_v_result && frame.voltage_v < 0.0)
    {
        std::cerr << "error: voltage isn't correct : " << frame.voltage_v << '\n';
        frame.valid = false;
    }

    bool current_a_result = parse_double(fields[3], frame.current_a);
    bool temperature_c_result = parse_double(fields[4], frame.temperature_c);

    if(temperature_c_result && (frame.temperature_c < -40.0 || frame.temperature_c > 120.0))
    {
        std::cerr << "error: temperature isn't correct : " << frame.temperature_c << '\n';
        frame.valid = false;
    }

    bool gps_fix_result = parse_int(fields[5], frame.gps_fix);

    if(frame.gps_fix != 0 && frame.gps_fix != 1)
    {
        std::cerr << "error: gps isn't correct : " << frame.gps_fix << '\n';
        frame.valid = false;
    }

    bool satellites_result = parse_int(fields[6], frame.satellites);

    if(satellites_result && frame.satellites < 0)
    {
        std::cerr << "error: satellites isn't correct : " << frame.satellites << '\n';
        frame.valid = false;
    }

    if(!timestamp_ms_result || !seq_result || !voltage_v_result || !current_a_result || !temperature_c_result || !gps_fix_result || !satellites_result)
    {
        frame.valid = false;
    }

    return frame;
}

double compute_frame_rate_hz(const Frame frames[], int count) {
    const long elapsed_ms = frames[count - 1].timestamp_ms - frames[0].timestamp_ms;

    return static_cast<double>((count - 1) * 1000 / elapsed_ms);
}

int read_frames(const char* path, Frame frames[], int max_frames) {
    std::ifstream input{path};

    if (!input) {
        std::cerr << "error: failed to open input file: " << path << '\n';
        return 0;
    }

    int count = 0;
    char line[MAX_LINE_LENGTH];

    while (input.getline(line, MAX_LINE_LENGTH)) {

        if (line[0] == '\0') {
            std::cerr << "error: line is empty: " << "\n";
            continue;
        }

        char* fields[EXPECTED_FIELD_COUNT] = {};
        char copy[MAX_LINE_LENGTH];
        const int field_count = split_line(std::strcpy(copy, line), fields, EXPECTED_FIELD_COUNT);

        if (field_count < EXPECTED_FIELD_COUNT) {
            std::cerr << "error: invalid fields count: " << field_count << "\n";
            continue;
        }

        if (count < max_frames) {
            frames[count] = parse_frame(line);
            ++count;
        }
    }

    if(count > 1)
        for (int i = 1; i < count; ++i) {
            if (frames[i].timestamp_ms <= frames[i - 1].timestamp_ms) {
                std::cerr << "error: timestamp_ms doesn't grow at index: [" << i << "] - [" << i-1 << "]" << "\n";
                frames[i].valid = false;
            }

            if(frames[i].seq - frames[i - 1].seq != 1) {
                std::cerr << "error: seq doesn't grow by 1 at index: [" << i << "] - [" << i-1 << "]" << "\n";
                frames[i].valid = false;
            }
        }

    return count;
}

Summary summarize(const Frame frames[], int count) {
    Summary summary{};
    summary.frames_total = count;
    summary.frames_valid = calculate_valid_frames_count(frames, count);
    summary.voltage_min = frames[0].voltage_v;
    summary.voltage_max = frames[0].voltage_v;
    summary.low_voltage_frames = 0;

    double temperature_sum = 0.0;

    for (int i = 0; i < count; ++i) {

        if(!frames[i].valid) {
            continue;
        }

        if (frames[i].voltage_v < summary.voltage_min) {
            summary.voltage_min = frames[i].voltage_v;
        }

        if (frames[i].voltage_v > summary.voltage_max) {
            summary.voltage_max = frames[i].voltage_v;
        }

        temperature_sum += frames[i].temperature_c;

        if (frames[i].voltage_v < 22.0) {
            ++summary.low_voltage_frames;
        }
    }

    summary.temperature_avg = static_cast<double>(temperature_sum) / frame_valid_count;
    summary.frame_rate_hz = compute_frame_rate_hz(frames, count);

    return summary;
}

void print_summary(const Summary& summary) {
    std::cout << "frames_total " << summary.frames_total << '\n';
    std::cout << "frames_valid " << summary.frames_valid << '\n';
    std::cout << "voltage_min " << summary.voltage_min << '\n';
    std::cout << "voltage_max " << summary.voltage_max << '\n';
    std::cout << "temperature_avg " << summary.temperature_avg << '\n';
    std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
    std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
