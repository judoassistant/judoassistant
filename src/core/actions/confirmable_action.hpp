#pragma once

class ConfirmableAction {
public:
    virtual ~ConfirmableAction() = default;
    virtual bool doesRequireConfirmation(const TournamentStore &tournament) const = 0;
};

