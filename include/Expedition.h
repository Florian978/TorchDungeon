#ifndef TORCH_DUNGEON_EXPEDITION_H
#define TORCH_DUNGEON_EXPEDITION_H

#include "Hero.h"
#include "Room.h"

#include <cstddef>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

class Expedition {
private:
    std::string m_dungeonName;
    int m_torchLevel;
    int m_gold;
    std::size_t m_currentRoomIndex;
    bool m_roomEntered;
    std::vector<Hero> m_party;
    std::vector<Room> m_rooms;

    [[nodiscard]] bool hasLivingHero() const;
    [[nodiscard]] std::optional<std::size_t> findCounterattackTarget(std::size_t preferredIndex) const;
    void consumeEntryTorchIfNeeded();

public:
    Expedition(std::string dungeonName,
               int initialTorch,
               int initialGold,
               std::vector<Hero> party,
               std::vector<Room> rooms);

    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isVictory() const;
    [[nodiscard]] int getTorchLevel() const;
    [[nodiscard]] int getGold() const;
    [[nodiscard]] std::size_t getCurrentRoomIndex() const;
    [[nodiscard]] std::size_t getRoomCount() const;
    [[nodiscard]] std::size_t getPartySize() const;

    [[nodiscard]] std::string executeTurn(std::size_t heroIndex,
                                          std::size_t skillIndex,
                                          std::size_t supportTargetIndex);
    [[nodiscard]] std::string campAndRecover(int goldBudget);
    [[nodiscard]] std::string assessSurvivalRisk() const;

    friend std::ostream& operator<<(std::ostream& os, const Expedition& expedition);
};

#endif // TORCH_DUNGEON_EXPEDITION_H