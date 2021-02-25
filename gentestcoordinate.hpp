#ifndef __GENTESTCOORDINATE_HPP__
#define __GENTESTCOORDINATE_HPP__

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include "Angel-yjc.hpp"

void gentestcoordinate() {
    std::srand(time(0));
    std::vector<int> random_call;
    random_call.push_back(0);

    // randomly generate 9 integers between [45, 135), [135, 225), ...
    for (int i = 0; i < 9; ++i) {
        random_call.push_back(45 + rand() % 89 + i * 90);
    }
    sort(random_call.begin(), random_call.end());
    random_call.push_back(900);

    std::vector<std::vector<float>> position;
    // generate 11 coordinates, including the starting point and the ending
    // point
    for (int i = 0; i < 11; ++i) {
        float radian = random_call[i] * M_PI / 1800.0;
        position.push_back(
            std::vector<float>{(1 - cos(radian)) * 1000, -sin(radian) * 1000});
    }

    // calculate 10 velocity vectors between those 11 coordinates
    std::vector<std::vector<float>> velocity;
    for (int i = 0; i < 10; ++i) {
        velocity.push_back(
            std::vector<float>{(position[i + 1][0] - position[i][0]) /
                                   (random_call[i + 1] - random_call[i]),
                               (position[i + 1][1] - position[i][1]) /
                                   (random_call[i + 1] - random_call[i])});
    }

    // adding noise to the position
    for (int i = 1; i < 11; ++i) {
        position[i][0] += (float)rand() / (float)(RAND_MAX / 50.0) - 25.0;
        position[i][1] += (float)rand() / (float)(RAND_MAX / 50.0) - 25.0;
    }

    // adding noise to the velocity
    for (int i = 0; i < 10; ++i) {
        velocity[i][0] += (float)rand() / (float)(RAND_MAX / 0.5) - 0.25;
        velocity[i][1] += (float)rand() / (float)(RAND_MAX / 0.5) - 0.25;
    }

    std::ofstream coordinatestream;
    coordinatestream.clear();
    coordinatestream.open("data.txt");
    for (int i = 0; i < 10; ++i) {
        coordinatestream << position[i][0] << ' ' << position[i][1] << ' '
                         << velocity[i][0] << ' ' << velocity[i][1] << ' '
                         << random_call[i] << std::endl;
    }
    coordinatestream.close();
}

#endif  // __GENTESTCOORDINATE_HPP__