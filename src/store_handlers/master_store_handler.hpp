#pragma once

#include <QObject>
#include <QString>
#include <fstream>

#include "core.hpp"

#include "serialize.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"
#include "stores/player_store.hpp"
#include "stores/qtournament_store.hpp"
#include "store_handlers/qstore_handler.hpp"

#include "actions/actions.hpp"
#include "rulesets/rulesets.hpp"
#include "draw_systems/draw_systems.hpp"

class MasterStoreHandler : public QStoreHandler {
    Q_OBJECT
public:
    MasterStoreHandler();
    QTournamentStore & getTournament() override;
    const QTournamentStore & getTournament() const override;
    void dispatch(std::unique_ptr<Action> && action) override;
    bool canUndo() override;
    void undo() override;
    bool canRedo() override;
    void redo() override;
    bool read(const QString &path);
    bool write(const QString &path);
    void reset();
private:
    std::unique_ptr<QTournamentStore> mTournament;
    std::vector<std::unique_ptr<Action>> mActionStack;
    std::vector<std::unique_ptr<Action>> mRedoStack;
};
