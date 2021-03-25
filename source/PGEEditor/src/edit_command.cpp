#include "../include/edit_command.h"

namespace pge
{
    edit_CommandStack::edit_CommandStack()
        : m_undos(0)
    {}

    void
    edit_CommandStack::Add(std::unique_ptr<edit_Command> command)
    {
        for (size_t i = 0; i < m_undos; ++i) {
            m_stack.pop_back();
        }
        m_stack.push_back(std::move(command));
        m_undos = 0;
    }

    void
    edit_CommandStack::Do(std::unique_ptr<edit_Command> command)
    {
        for (size_t i = 0; i < m_undos; ++i) {
            m_stack.pop_back();
        }
        command->Do();
        m_stack.push_back(std::move(command));
        m_undos = 0;
    }

    void
    edit_CommandStack::Undo()
    {
        if (m_undos == m_stack.size())
            return;
        m_stack[m_stack.size() - (1 + m_undos++)]->Undo();
    }

    void
    edit_CommandStack::Redo()
    {
        if (m_undos == 0)
            return;
        m_stack[m_stack.size() - m_undos--]->Do();
    }
} // namespace pge