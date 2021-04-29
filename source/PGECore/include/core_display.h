#ifndef PGE_CORE_CORE_DISPLAY_H
#define PGE_CORE_CORE_DISPLAY_H

#include <memory>

namespace pge
{
    class core_Display {
    protected:
        struct core_DisplayImpl;
        std::unique_ptr<core_DisplayImpl> m_impl;
        core_Display();
        ~core_Display();

    public:
        void HandleEvents();
        int  GetWidth() const;
        int  GetHeight() const;
        bool IsCloseRequested() const;
    };
} // namespace pge

#endif