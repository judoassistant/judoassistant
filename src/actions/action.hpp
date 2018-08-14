#pragma once

class Action {
public:
    virtual void redo() const = 0;
    virtual void undo() const = 0;
}
