#pragma once

#include <QObject>
#include <QString>
#include <fstream>

#include "store_handlers/qstore_handler.hpp"

#include "serialize.hpp"
#include "stores/category_store.hpp"
#include "stores/match_store.hpp"
#include "stores/match_event.hpp"
#include "stores/player_store.hpp"
#include "stores/qtournament_store.hpp"

#include "actions/actions.hpp"
#include "rulesets/rulesets.hpp"
#include "draw_systems/draw_systems.hpp"

class MasterStoreHandler : public QStoreHandler {
    Q_OBJECT
public:
    MasterStoreHandler();
    void dispatch(std::unique_ptr<Action> && action) override;
    QTournamentStore & getTournament() override;
    const QTournamentStore & getTournament() const override;
    bool read(const QString &path);
    bool write(const QString &path);
    void reset();
private:
    std::unique_ptr<QTournamentStore> mTournament;
};
