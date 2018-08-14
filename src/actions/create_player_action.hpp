#pragma once

class CreatePlayerAction : public Action {
public:
    CreatePlayerAction(const std::string & firstName, const std::string & lastName, uint8_t age);
    virtual void redo() const;
    virtual void undo() const;

private:
    std::string mFirstName;
    std::string mLastName;
    uint8_t mAge;
    PlayerStore::Id id;
}
