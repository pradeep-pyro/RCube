#include "RCube/Core/Arch/System.h"
namespace rcube {

void ComponentMask::set(size_t pos, bool flag) {
    bits.set(pos, flag);
}

void ComponentMask::reset(size_t pos) {
    bits.reset(pos);
}

bool ComponentMask::match(const ComponentMask &other) {
    return (bits & other.bits) == other.bits;
}

bool ComponentMask::equal(const ComponentMask &other) {
    return bits == other.bits;
}

std::string ComponentMask::to_string() const {
    return bits.to_string();
}

bool operator==(const ComponentMask &lhs, const ComponentMask &rhs) {
    return lhs.bits == rhs.bits;
}

} // namespace rcube
