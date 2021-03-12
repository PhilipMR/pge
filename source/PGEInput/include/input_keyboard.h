#ifndef PGE_INPUT_INPUT_KEYBOARD_H
#define PGE_INPUT_INPUT_KEYBOARD_H

namespace pge
{
    enum class input_KeyboardKey
    {
        BACKSPACE = 8,

        ENTER = 13,

        SHIFT = 16,
        CTRL = 17,

        SPACE = 32,

        ARROW_LEFT = 37,
        ARROW_UP,
        ARROW_RIGHT,
        ARROW_DOWN,

        NUM0 = 48,
        NUM1, NUM2, NUM3,
        NUM4, NUM5, NUM6,
        NUM7, NUM8, NUM9,

        A = 65,
        B, C, D, E,
        F, G, H, I,
        J, K, L, M,
        N, O, P, Q,
        R, S, T, U,
        V, W, X, Y,
        Z,

    };

    void input_KeyboardClearState();
    void input_KeyboardClearDelta();
    bool input_KeyboardDown(input_KeyboardKey key);
    bool input_KeyboardPressed(input_KeyboardKey key);
    bool input_KeyboardReleased(input_KeyboardKey key);
}

#endif