#ifndef MODES_H
#define MODES_H

#include <stdexcept>

/**
 *
 */
enum class Blend {
    Multiply = 0,
    Add = 1,
    Mix = 2
};

/**
 *
 */
enum class Draw {
    Front = 0,
    Back = 1,
    Both = 2
};

enum class Medium {
    Air, Water, Ice, Glass, Diamond
};

constexpr double refractiveIndex(Medium m) {
    // http://hyperphysics.phy-astr.gsu.edu/hbase/Tables/indrf.html
    switch(m) {
    case Medium::Air:
        return 1.00029;
    case Medium::Water:
        return 1.33;
    case Medium::Ice:
        return 1.309;
    case Medium::Glass:
        return 1.52;
    case Medium::Diamond:
        return 2.417;
    }
}

#endif // CONSTANTS_H
