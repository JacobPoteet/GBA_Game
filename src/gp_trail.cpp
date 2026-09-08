/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_trail.h"

namespace gp
{

void trail::reset(tile_point point)
{
    for (int index = 0; index < capacity; ++index)
    {
        _points[index] = point;
    }

    _head = 0;
    _size = 1;
}

void trail::push(tile_point point)
{
    _head = (_head + 1) % capacity;
    _points[_head] = point;

    if (_size < capacity)
    {
        ++_size;
    }
}

tile_point trail::at(int steps_behind) const
{
    if (_size == 0)
    {
        return tile_point{};
    }

    int steps = steps_behind < 0 ? 0 : steps_behind;

    if (steps > _size - 1)
    {
        steps = _size - 1;
    }

    int index = ((_head - steps) % capacity + capacity) % capacity;
    return _points[index];
}

} // namespace gp
