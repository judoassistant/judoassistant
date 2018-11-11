#pragma once

#include <memory>
#include <QThread>

#include "core.hpp"
#include "stores/tournament_store.hpp"

class NetworkInterface : public QThread {
    Q_OBJECT
public:
    virtual void postSync(std::unique_ptr<TournamentStore> tournament) = 0;
    virtual void postAction(ActionId actionId, std::unique_ptr<Action> action) = 0;
    virtual void postUndo(ActionId actionId) = 0;

    virtual void start() = 0;
    virtual void quit() = 0;

signals:
    void actionReceived(ActionId actionId, std::shared_ptr<const Action> action);
    void actionConfirmReceived(ActionId actionId);
    void undoReceived(ActionId actionId);
    void undoConfirmReceived(ActionId actionId);
    void syncConfirmed();
};

