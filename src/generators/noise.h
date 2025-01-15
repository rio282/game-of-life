#ifndef GAMEOFLIFE_NOISE_H
#define GAMEOFLIFE_NOISE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define P 256

int permutation[2 * P];
float grad[8][2] = {{1,  1},
                    {-1, 1},
                    {1,  -1},
                    {-1, -1},
                    {1,  0},
                    {-1, 0},
                    {0,  1},
                    {0,  -1}};

int hash(int i) {
    return permutation[i & 255];
}

float dot(const float g[], float x, float y) {
    return g[0] * x + g[1] * y;
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float perlin(float x, float y) {
    int X = (int) floorf(x) & 255;
    int Y = (int) floorf(y) & 255;

    float fx = x - floorf(x);
    float fy = y - floorf(y);

    float u = fx * fx * (3.0f - 2.0f * fx);
    float v = fy * fy * (3.0f - 2.0f * fy);

    int aa = hash(X + hash(Y));
    int ab = hash(X + hash(Y + 1));
    int ba = hash(X + 1 + hash(Y));
    int bb = hash(X + 1 + hash(Y + 1));

    float x1 = lerp(dot(grad[aa], fx, fy), dot(grad[ba], fx - 1, fy), u);
    float x2 = lerp(dot(grad[ab], fx, fy - 1), dot(grad[bb], fx - 1, fy - 1), u);

    return lerp(x1, x2, v);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc51-cpp"
#pragma ide diagnostic ignored "cert-msc50-cpp"

void InitPermutationTable() {

    for (int i = 0; i < P; ++i) {
        permutation[i] = i;
    }

    for (int i = 0; i < P; ++i) {
        int j = rand() % P;
        int temp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = temp;
    }

}

#pragma clang diagnostic pop

void GenerateBinaryNoise(int width, int height, bool grid[], float threshold) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float value = perlin((float) x * 0.1f, (float) y * 0.1f);
            const int idx = y * width + x;
            grid[idx] = value > threshold;
        }
    }
}

#endif //GAMEOFLIFE_NOISE_H
