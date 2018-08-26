#pragma once

#include <QObject>

#include "core.hpp"
#include "serialize.hpp"
#include "stores/tournament_store.hpp"

class QTournamentStore : public QObject, public TournamentStore {
    Q_OBJECT
public:
    QTournamentStore() {}

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        cereal::base_class<TournamentStore>(this);
    }

signals:
    void tournamentChanged(Id id) override;
    void matchAdded(Id id) override;
    void matchChanged(Id id) override;
    void matchDeleted(Id id) override;
    void categoryAdded(Id id) override;
    void categoryChanged(Id id) override;
    void categoryDeleted(Id id) override;
    void playerAdded(Id) override;
    void playerChanged(Id id) override;
    void playerDeleted(Id id) override;
};

CEREAL_REGISTER_TYPE(QTournamentStore)
CEREAL_REGISTER_POLYMORPHIC_RELATION(TournamentStore, QTournamentStore)
