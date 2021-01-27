#ifndef PGE_OS_OS_DISPLAY_H
#define PGE_OS_OS_DISPLAY_H

#include <memory>

namespace pge
{
    class os_Display {
    protected:
        struct os_DisplayImpl;
        std::unique_ptr<os_DisplayImpl> m_impl;
        os_Display();
        ~os_Display();

    public:
        void HandleEvents();
        int  GetWidth() const;
        int  GetHeight() const;
        bool IsCloseRequested() const;
    };
} // namespace pge

#endif