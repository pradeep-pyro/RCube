#pragma once

namespace rcube
{

/**
 * Helper class to manage ping-pong buffers (or any objects)
 */
template <class T> class PingPongHelper
{
    T objects_[2];
    int index_ = 0;

  public:
    T &first()
    {
        return objects_[index_];
    }
    const T &first() const
    {
        return objects_[index_];
    }
    T &second()
    {
        return objects_[1 - index_];
    }
    const T &second() const
    {
        return objects_[1 - index_];
    }
    void increment()
    {
        index_ = 1 - index_;
    }
};

} // namespace rcube
