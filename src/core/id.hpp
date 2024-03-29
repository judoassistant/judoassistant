#pragma once

#include <random>
#include <limits>
#include <ostream>
#include <memory>
#include "core/hash.hpp"
#include "core/random.hpp"

class CategoryStore;
class TournamentStore;

template <typename CRTP>
class Id {
public:
    typedef uint32_t InternalType;

    class Generator {
    public:
        Generator() {}
        Generator(InternalType seed)
            : mEng(seed)
        {}

        CRTP operator()() {
            return CRTP(mEng());
        }

    private:
        std::mt19937 mEng;
    };

    struct Hasher {
    public:
        size_t operator()(const CRTP &k) const {
            return std::hash<InternalType>()(k.getValue());
        }
    };

    Id() {}
    Id(InternalType value) : mValue(value) {}
    Id(const std::string &str) {
        try {
            mValue = std::stoul(str);
        }
        catch (const std::exception &e) {
            throw std::invalid_argument(str);
        }
    }

    InternalType getValue() const {
        return mValue;
    }

    std::string toString() const {
        return std::to_string(mValue);
    }

    bool operator==(const CRTP &other) const {
        return mValue == other.mValue;
    }

    bool operator!=(const CRTP &other) const {
        return mValue != other.mValue;
    }

    bool operator<(const CRTP &other) const {
        return mValue < other.mValue;
    }

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mValue);
    }

private:
    InternalType mValue;
};

template <typename T>
std::ostream & operator<<(std::ostream & o, Id<T> id) {
    return o << std::hex << id.getValue() << std::dec;
}

class MatchId : public Id<MatchId> {
public:
    MatchId() {}
    MatchId(InternalType value) : Id(value) {}

    static MatchId generate(const CategoryStore &category);
    static MatchId generate(const CategoryStore &category, MatchId::Generator &generator);

    static std::vector<MatchId> generateMultiple(const CategoryStore &category, const size_t count);
    static std::vector<MatchId> generateMultiple(const CategoryStore &category, MatchId::Generator &generator, const size_t count);
};

class CategoryId : public Id<CategoryId> {
public:
    CategoryId() {}
    CategoryId(InternalType value) : Id(value) {}
    CategoryId(const std::string &str) : Id(str) {}

    static CategoryId generate(const TournamentStore &tournament);
    static CategoryId generate(const TournamentStore &tournament, CategoryId::Generator &generator);

    static std::vector<CategoryId> generateMultiple(const TournamentStore &tournament, const size_t count);
    static std::vector<CategoryId> generateMultiple(const TournamentStore &tournament, CategoryId::Generator &generator, const size_t count);
};

class PlayerId : public Id<PlayerId> {
public:
    PlayerId() {}
    PlayerId(InternalType value) : Id(value) {}
    PlayerId(const std::string &str) : Id(str) {}

    static PlayerId generate(const TournamentStore &tournament);
    static PlayerId generate(const TournamentStore &tournament, PlayerId::Generator &generator);

    static std::vector<PlayerId> generateMultiple(const TournamentStore &tournament, const size_t count);
    static std::vector<PlayerId> generateMultiple(const TournamentStore &tournament, PlayerId::Generator &generator, const size_t count);
};

class PositionId : public Id<PositionId> {
public:
    PositionId() {}
    PositionId(InternalType value) : Id(value) {}

    template <typename PositionManager>
    static PositionId generate(const PositionManager &manager) {
        static std::unique_ptr<PositionId::Generator> generator; // singleton
        if (!generator)
            generator = std::make_unique<PositionId::Generator>(getSeed());

        return generate(manager, *generator);
    };

    template <typename PositionManager>
    static PositionId generate(const PositionManager &manager, PositionId::Generator &generator) {
        PositionId id;
        do {
            id = generator();
        } while(manager.contains(id));

        return id;
    };
};

class ActionId : public Id<ActionId> {
public:
    ActionId() {}
    ActionId(InternalType value) : Id(value) {}

    static ActionId generate();
};

class ClientId : public Id<ClientId> {
public:
    ClientId() {}
    ClientId(InternalType value) : Id(value) {}

    static ClientId generate();
};

class TournamentId : public Id<TournamentId> {
public:
    TournamentId() {}
    TournamentId(InternalType value) : Id(value) {}

    static TournamentId generate();
};

class ClientActionId {
public:
    ClientActionId();
    ClientActionId(const ClientId &clientId, const ActionId &actionId);

    bool operator==(const ClientActionId &other) const;
    bool operator!=(const ClientActionId &other) const;
    bool operator<(const ClientActionId &other) const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mClientId, mActionId);
    }

    std::string toString() const;

    ClientId getClientId() const;
    ActionId getActionId() const;

private:
    ClientId mClientId;
    ActionId mActionId;
};

std::ostream & operator<<(std::ostream & o, ClientActionId id);

class CombinedId {
public:
    CombinedId();
    CombinedId(CategoryId categoryId, MatchId matchId);

    bool operator==(const CombinedId &other) const;
    bool operator!=(const CombinedId &other) const;
    bool operator<(const CombinedId &other) const;

    template<typename Archive>
    void serialize(Archive& ar, uint32_t const version) {
        ar(mCategoryId, mMatchId);
    }

    std::string toString() const;

    CategoryId getCategoryId() const {
        return mCategoryId;
    }

    MatchId getMatchId() const {
        return mMatchId;
    }

private:
    CategoryId mCategoryId;
    MatchId mMatchId;
};

namespace std {
    template<>
    class hash<MatchId> : public Id<MatchId>::Hasher {};

    template<>
    class hash<CategoryId> : public Id<CategoryId>::Hasher {};

    template<>
    class hash<PlayerId> : public Id<PlayerId>::Hasher {};

    template<>
    class hash<PositionId> : public Id<PositionId>::Hasher {};

    template<>
    class hash<ActionId> : public Id<ActionId>::Hasher {};

    template<>
    class hash<ClientId> : public Id<ClientId>::Hasher {};

    template<>
    class hash<TournamentId> : public Id<TournamentId>::Hasher {};

    template<>
    struct hash<ClientActionId> {
        size_t operator()(const ClientActionId &id) const {
            size_t seed = 0;
            hash_combine(seed, id.getClientId());
            hash_combine(seed, id.getActionId());
            return seed;
        }
    };

    template<>
    struct hash<CombinedId> {
        size_t operator()(const CombinedId &id) const {
            size_t seed = 0;
            hash_combine(seed, id.getCategoryId());
            hash_combine(seed, id.getMatchId());
            return seed;
        }
    };
}

