#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#define MAX_HOP 514
struct Potato {
    int hop_num;
    int trace[MAX_HOP];
    int count;
};
typedef struct Potato P;