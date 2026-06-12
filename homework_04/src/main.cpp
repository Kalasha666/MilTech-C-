#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

const int TICKS_PER_REVOLUTION = 1024;
const float WHEEL_RADIUS_M = 0.3;
const float WHEELBASE_M = 1.0;
const int EXPECTED_FIELD_COUNT = 5;
const int MAX_LINE_LENGTH = 128;

struct WheelTick
{
    long left;
    long right;
};

struct InputData 
{
    long timestamp_ms;
    WheelTick front;
    WheelTick back;
};

struct OutputData 
{
    long timestamp_ms;
    float x;
    float y;
    float theta;
};

float parse_float(const char* text);
int split_line(char line[], char* fields[], int max_fields);
InputData parse_data(char line[]);

int main(int argc, char** argv) {
    // The program expects exactly one argument: a path to telemetry samples.
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";
        return 1;
    }

    std::ifstream input{argv[1]};

    if (!input) {
        std::cerr << "error: failed to open input file: " << argv[1] << '\n';
        return 2;
    }

    char line[MAX_LINE_LENGTH];
    bool any_line = false;
    int line_number = 0;

    while (input.getline(line, MAX_LINE_LENGTH))
    {
        if (line[0] == '\0') continue;

        line_number++;
    }

    int capacity = line_number > 0 ? line_number : 1;
    input.clear();
    input.seekg(0);
    line_number = 0;
    InputData* input_records = capacity > 0 ? new InputData[capacity] : nullptr;

    while (capacity > 0 && input.getline(line, MAX_LINE_LENGTH)) 
    {
        if (line[0] == '\0') 
        {
            std::cout << "empty line, skipping\n";
            continue;
        }
  
        any_line = true;
        const InputData data = parse_data(line);
        std::cout << "timestamp: " << data.timestamp_ms << " front.left: " << data.front.left << " front.right: " << data.front.right
                    << " back.left: " << data.back.left << " back.right: " << data.back.right << "\n";

        input_records[line_number] = data;
        line_number++;
    }

    std::cout << "Input records count: " << line_number << "\n";
    
    if(line_number > 1)
    {
        OutputData* output_records = new OutputData[line_number - 1];
        float x = 0;
        float y = 0;
        float theta = 0;

        for(int i = 1; i < line_number; i++) 
        {
            const InputData& prev = input_records[i - 1];
            const InputData& curr = input_records[i];
            long d_fl = curr.front.left - prev.front.left;
            long d_fr = curr.front.right - prev.front.right;
            long d_bl = curr.back.left - prev.back.left;
            long d_br = curr.back.right - prev.back.right;

            float d_left = (d_fl + d_bl) / 2;
            float d_right = (d_fr + d_br) / 2;

            float distance_per_tick = 2 * M_PI * WHEEL_RADIUS_M / TICKS_PER_REVOLUTION;
            float d_left_m = d_left * distance_per_tick;
            float d_right_m = d_right * distance_per_tick;

            float d_center = (d_left_m + d_right_m) / 2;
            float d_theta = (d_right_m - d_left_m) / WHEELBASE_M;
            x += d_center * cos(theta + d_theta / 2);
            y += d_center * sin(theta + d_theta / 2);
            theta += d_theta;
            output_records[i - 1] = {curr.timestamp_ms, x, y, theta};
            std::cout << curr.timestamp_ms << " " << x << " " << y << " " << theta << "\n";
        }

        delete[] output_records;
    }

    if(input_records) 
    {
        delete[] input_records;
    }
    
    return 0;
}

int split_line(char line[], char* fields[], int max_fields) 
{
    int count = 0;
    char* cursor = line;

    while (*cursor != '\0' && count < max_fields)
    {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') 
        {
            *cursor = '\0';
            ++cursor;
        }

        if (*cursor == '\0') 
        {
            break;
        }

        fields[count] = cursor;
        ++count;

        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n' && *cursor != '\r') 
        {
            ++cursor;
        }
    }

    return count;
}

float parse_float(const char* text) 
{
    if (text[0] == '+') 
    {
        ++text;
    }

    return std::strtod(text, nullptr);
}

InputData parse_data(char line[]) 
{
    char* fields[EXPECTED_FIELD_COUNT] = {};
    const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);
    (void)field_count;
    InputData data{};

    data.timestamp_ms = (field_count > 0) ? parse_float(fields[0]) : 0;    
    data.front.left = (field_count > 1) ? parse_float(fields[1]) : 0;
    data.front.right = (field_count > 2) ? parse_float(fields[2]) : 0;
    data.back.left = (field_count > 3) ? parse_float(fields[3]) : 0;
    data.back.right = (field_count > 4) ? parse_float(fields[4]) : 0;
        
    return data;
}