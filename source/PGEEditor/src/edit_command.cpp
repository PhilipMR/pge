#include "../include/edit_command.h"

namespace pge
{
    edit_CommandStack::edit_CommandStack()
        : m_cursor(-1)
    {}

    void
    edit_CommandStack::Add(std::unique_ptr<edit_Command> command)
    {
        if (!m_stack.empty()) {
            for (size_t i = m_stack.size() - 1; i > m_cursor; i--) {
                m_stack.pop_back();
            }
        }
        m_stack.push_back(std::move(command));
        m_cursor = m_stack.size() - 1;
    }

    void
    edit_CommandStack::Do(std::unique_ptr<edit_Command> command)
    {
        if (!m_stack.empty()) {
            for (size_t i = m_stack.size() - 1; i > m_cursor; i--) {
                m_stack.pop_back();
            }
        }
        command->Do();
        m_stack.push_back(std::move(command));
        m_cursor = m_stack.size() - 1;
    }

    void
    edit_CommandStack::Undo()
    {
        if (m_cursor >= 0) {
            m_stack[m_cursor--]->Undo();
        }
    }

    void
    edit_CommandStack::Redo()
    {
        if (m_stack.empty())
            return;
        if (m_cursor < (int)m_stack.size() - 1) {
            m_stack[++m_cursor]->Do();
        }
    }
} // namespace pge