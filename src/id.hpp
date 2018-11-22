#pragma once

#include <random>
#include <limits>
#include <ostream>
#include <memory>

class CategoryStore;
class TournamentStore;
class StoreManager;

unsigned int getGeneratorSeed();

template <typename CRTP>
class Id {
public:
    typedef uint64_t InternalType;

    class Generator {
    public:
        Generator() {} // TODO: Seed the generator
        Generator(InternalType seed)
            : mEng(seed)
            , mDist(std::numeric_limits<InternalType>::min(), std::numeric_limits<InternalType>::max())
        {}

        CRTP operator()() {
            return CRTP(static_cast<InternalType>(mDist(mEng)));
        }

    private:
        std::default_random_engine mEng;
        std::uniform_int_distribution<InternalType> mDist;
    };

    struct Hasher {
    public:
        size_t operator()(const CRTP &k) const {
            return std::hash<InternalType>()(k.getValue());
        }
    };

    Id() {}
    Id(InternalType value) : mValue(value) {}

    InternalType getValue() const {
        return mValue;
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
};

class CategoryId : public Id<CategoryId> {
public:
    CategoryId() {}
    CategoryId(InternalType value) : Id(value) {}

    static CategoryId generate(const TournamentStore &tournament);
};

class PlayerId : public Id<PlayerId> {
public:
    PlayerId() {}
    PlayerId(InternalType value) : Id(value) {}

    static PlayerId generate(const TournamentStore &tournament);
};

class PositionId : public Id<PositionId> {
public:
    PositionId() {}
    PositionId(InternalType value) : Id(value) {}

    template <typename PositionManager>
    static PositionId generate(const PositionManager &manager) {
        static std::unique_ptr<PositionId::Generator> generator; // singleton
        if (!generator)
            generator = std::make_unique<PositionId::Generator>(getGeneratorSeed());

        PositionId id;
        do {
            id = (*generator)();
        } while(manager.containsId(id));

        return id;
    };

};

class ActionId : public Id<ActionId> {
public:
    ActionId() {}
    ActionId(InternalType value) : Id(value) {}

    static ActionId generate(const StoreManager &storeManager);
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
}

