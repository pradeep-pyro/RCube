#ifndef COMPONENT_H
#define COMPONENT_H

namespace rcube {

namespace internal {

/**
 * Global counter for derived classes (not objects) from Component
 */
struct ComponentCounter {
    static unsigned int counter;
};

} // namespace internal

/**
 * Base class for all components.
 * Pass the derived class as a template (Curiously Recurring Template Pattern);
 * this is to assign a unique ID to each subclass of Component
 */
template <typename Derived>
class Component {
public:
    /**
     * Returns a unique identifier per component type i.e., subclass of Component
     * @return integer id
     */
    static inline unsigned int family() {
        static unsigned int family = internal::ComponentCounter::counter++;
        return family;
    }
};

} // namespace rcube

#endif // COMPONENT_H
