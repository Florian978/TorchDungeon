#include "Room.h"

#include <algorithm>
#include <sstream>
#include <string_view>
#include <utility>

namespace {
std::string_view roomTypeToString(RoomType type) {
    switch (type) {
        case RoomType::Combat:
            return "Combat";
        case RoomType::Trap:
            return "Trap";
        case RoomType::Treasure:
            return "Treasure";
        case RoomType::Camp:
            return "Camp";
    }
    return "Unknown";
}
} // namespace

Room::Room(std::string name,
           RoomType type,
           int torchCost,
           std::string encounterName,
           int enemyMaxHp,
           int hazardDamage,
           int hazardStress,
           int goldReward)
    : m_name{std::move(name)},
      m_type{type},
      m_torchCost{std::max(0, torchCost)},
      m_encounterName{std::move(encounterName)},
      m_enemyMaxHp{type == RoomType::Combat ? std::max(1, enemyMaxHp) : 0},
      m_enemyHp{m_enemyMaxHp},
      m_hazardDamage{std::max(0, hazardDamage)},
      m_hazardStress{std::max(0, hazardStress)},
      m_goldReward{std::max(0, goldReward)},
      m_cleared{false} {}

const std::string& Room::getName() const {
    return m_name;
}

RoomType Room::getType() const {
    return m_type;
}

int Room::getTorchCost() const {
    return m_torchCost;
}

bool Room::isCleared() const {
    return m_cleared;
}

bool Room::hasActiveEnemy() const {
    return m_type == RoomType::Combat && !m_cleared && m_enemyHp > 0;
}

int Room::getEnemyHp() const {
    return m_enemyHp;
}

int Room::takeEnemyDamage(int incomingDamage) {
    if (!hasActiveEnemy() || incomingDamage <= 0) {
        return 0;
    }

    m_enemyHp = std::max(0, m_enemyHp - incomingDamage);
    if (m_enemyHp == 0) {
        m_cleared = true;
        return m_goldReward;
    }

    return 0;
}

void Room::performEnemyCounterattack(Hero& target, int torchLevel) const {
    if (!hasActiveEnemy() || !target.isAlive()) {
        return;
    }

    int finalDamage = m_hazardDamage;
    int finalStress = m_hazardStress;

    if (torchLevel <= 25) {
        finalDamage = (finalDamage * 120) / 100;
        finalStress = (finalStress * 125) / 100;
    }

    target.receiveImpact(finalDamage, finalStress, torchLevel);
}

std::string Room::resolveTrapEvent(Hero& scout, int& torchLevel) {
    m_cleared = true;
    std::ostringstream oss;

    if (torchLevel >= 60) {
        scout.applyRecovery(0, 5);
        oss << scout.getName() << " spotted and disarmed " << m_encounterName
            << " thanks to bright torchlight (-5 stress).";
    } else {
        scout.receiveImpact(m_hazardDamage, m_hazardStress, torchLevel);
        torchLevel = std::max(0, torchLevel - 5);
        oss << scout.getName() << " triggered " << m_encounterName
            << " in dim light! Took damage and stress, torch dimmed by 5.";
    }

    return oss.str();
}

std::string Room::resolveTreasureEvent(int torchLevel, int& partyGold) {
    m_cleared = true;
    int totalGold = m_goldReward;

    if (torchLevel <= 25) {
        totalGold = (m_goldReward * 135) / 100;
    } else if (torchLevel < 75) {
        totalGold = (m_goldReward * 110) / 100;
    }

    partyGold += totalGold;

    std::ostringstream oss;
    oss << "Looted " << m_encounterName << " and found " << totalGold << " gold.";
    return oss.str();
}

std::string Room::resolveCampEvent(Hero& scout, int& torchLevel) {
    m_cleared = true;
    torchLevel = std::min(100, torchLevel + 20);
    scout.applyRecovery(10, 20);

    std::ostringstream oss;
    oss << scout.getName() << " rested at " << m_encounterName
        << " (+10 HP, -20 stress, +20 torch).";
    return oss.str();
}

std::string Room::triggerExplorationEvent(Hero& scout, int& torchLevel, int& partyGold) {
    if (m_cleared) {
        return "Room " + m_name + " is already cleared.";
    }

    if (!scout.isAlive()) {
        return scout.getName() + " cannot explore while fallen.";
    }

    switch (m_type) {
        case RoomType::Combat:
            return "Combat room! Defeat " + m_encounterName + " before moving on.";
        case RoomType::Trap:
            return resolveTrapEvent(scout, torchLevel);
        case RoomType::Treasure:
            return resolveTreasureEvent(torchLevel, partyGold);
        case RoomType::Camp:
            return resolveCampEvent(scout, torchLevel);
    }

    return "Nothing happened in " + m_name + ".";
}

std::ostream& operator<<(std::ostream& os, const Room& room) {
    os << "[" << roomTypeToString(room.m_type) << "] " << room.m_name
       << " | Encounter: " << room.m_encounterName
       << " | Torch cost: " << room.m_torchCost;

    if (room.m_type == RoomType::Combat) {
        os << " | Enemy HP: " << room.m_enemyHp << "/" << room.m_enemyMaxHp
           << " | ATK: " << room.m_hazardDamage
           << " | Stress ATK: " << room.m_hazardStress;
    }

    if (room.m_goldReward > 0) {
        os << " | Gold: " << room.m_goldReward;
    }

    os << " | State: " << (room.m_cleared ? "Cleared" : "Active");
    return os;
}