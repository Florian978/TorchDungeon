#ifndef TORCH_DUNGEON_ROOM_H
#define TORCH_DUNGEON_ROOM_H

#include "Hero.h"

#include <ostream>
#include <string>

enum class RoomType {
    Combat,
    Trap,
    Treasure,
    Camp
};

class Room {
private:
    std::string m_name;
    RoomType m_type;
    int m_torchCost;
    std::string m_encounterName;
    int m_enemyMaxHp;
    int m_enemyHp;
    int m_hazardDamage;
    int m_hazardStress;
    int m_goldReward;
    bool m_cleared;

    std::string resolveTrapEvent(Hero& scout, int& torchLevel);
    std::string resolveTreasureEvent(int torchLevel, int& partyGold);
    std::string resolveCampEvent(Hero& scout, int& torchLevel);

public:
    Room(std::string name,
         RoomType type,
         int torchCost,
         std::string encounterName,
         int enemyMaxHp,
         int hazardDamage,
         int hazardStress,
         int goldReward);

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] RoomType getType() const;
    [[nodiscard]] int getTorchCost() const;
    [[nodiscard]] bool isCleared() const;
    [[nodiscard]] bool hasActiveEnemy() const;
    [[nodiscard]] int getEnemyHp() const;

    [[nodiscard]] int takeEnemyDamage(int incomingDamage);
    void performEnemyCounterattack(Hero& target, int torchLevel) const;
    [[nodiscard]] std::string triggerExplorationEvent(Hero& scout, int& torchLevel, int& partyGold);

    friend std::ostream& operator<<(std::ostream& os, const Room& room);
};

#endif // TORCH_DUNGEON_ROOM_H