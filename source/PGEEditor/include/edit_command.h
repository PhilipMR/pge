#ifndef PGE_EDITOR_EDIT_COMAND_H
#define PGE_EDITOR_EDIT_COMAND_H

#include <vector>
#include <memory>

namespace pge
{
    class edit_Command {
    public:
        virtual ~edit_Command() = default;
        virtual void Do()       = 0;
        virtual void Undo()     = 0;
    };

    class edit_CommandStack {
        std::vector<std::unique_ptr<edit_Command>> m_stack;
        int                                        m_undos;

    public:
        edit_CommandStack();
        void Clear();
        void Add(std::unique_ptr<edit_Command> command);
        void Do(std::unique_ptr<edit_Command> command);
        void Undo();
        void Redo();
    };
} // namespace pge

#endif