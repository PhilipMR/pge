#include "../include/edit_command.h"

namespace pge
{
    edit_CommandStack::edit_CommandStack()
        : m_undos(0)
    {}

    void
    edit_CommandStack::Add(std::unique_ptr<edit_Command> command)
    {
        diag_LogDebugf("COMMAND ADD -- Undo = %d", m_undos);
        for (size_t i = 0; i < m_undos; ++i) {
            m_stack.pop_back();
        }
        m_stack.push_back(std::move(command));
        m_undos = 0;
    }

    void
    edit_CommandStack::Do(std::unique_ptr<edit_Command> command)
    {
        diag_LogDebugf("COMMAND DO -- Undo = %d", m_undos);
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
        diag_LogDebugf("COMMAND UNDO -- Undo = %d", m_undos);
        if (m_undos == m_stack.size())
            return;
        m_stack[m_stack.size() - (1 + m_undos++)]->Undo();
    }

    void
    edit_CommandStack::Redo()
    {
        diag_LogDebugf("COMMAND REDO -- Undo = %d", m_undos);
        if (m_undos == 0)
            return;
        m_stack[m_stack.size() - m_undos--]->Do();
    }
} // namespace pge