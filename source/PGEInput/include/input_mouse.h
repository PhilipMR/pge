#ifndef PGE_INPUT_INPUT_MOUSE_H
#define PGE_INPUT_INPUT_MOUSE_H

#include <math_vec2.h>

namespace pge
{
    enum class input_MouseButton
    {
        LEFT,
        MIDDLE,
        RIGHT,
        NUM_BUTTONS
    };

    void      input_MouseClearState();
    void      input_MouseClearDelta();
    bool      input_MouseButtonDown(input_MouseButton button);
    bool      input_MouseButtonPressed(input_MouseButton button);
    bool      input_MouseButtonReleased(input_MouseButton button);
    math_Vec2 input_MousePosition();
    math_Vec2 input_MouseDelta();

} // namespace pge

#endif