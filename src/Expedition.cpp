#include "Expedition.h"

#include <algorithm>
#include <sstream>
#include <utility>

Expedition::Expedition(std::string dungeonName,
                       int initialTorch,
                       int initialGold,
                       std::vector<Hero> party,
                       std::vector<Room> rooms)
    : m_dungeonName{std::move(dungeonName)},
      m_torchLevel{std::clamp(initialTorch, 0, 100)},
      m_gold{std::max(0, initialGold)},
      m_currentRoomIndex{0},
      m_roomEntered{false},
      m_party{std::move(party)},
      m_rooms{std::move(rooms)} {}

bool Expedition::hasLivingHero() const {
    return std::any_of(m_party.begin(), m_party.end(), [](const Hero& hero) {
        return hero.isAlive();
    });
}

std::optional<std::size_t> Expedition::findCounterattackTarget(std::size_t preferredIndex) const {
    if (preferredIndex < m_party.size() && m_party[preferredIndex].isAlive()) {
        return preferredIndex;
    }

    for (std::size_t i = 0; i < m_party.size(); ++i) {
        if (m_party[i].isAlive()) {
            return i;
        }
    }

    return std::nullopt;
}

void Expedition::consumeEntryTorchIfNeeded() {
    if (m_roomEntered || m_currentRoomIndex >= m_rooms.size()) {
        return;
    }

    const int torchCost = m_rooms[m_currentRoomIndex].getTorchCost();
    m_torchLevel = std::max(0, m_torchLevel - torchCost);
    m_roomEntered = true;
}

bool Expedition::isActive() const {
    return hasLivingHero() && m_currentRoomIndex < m_rooms.size();
}

bool Expedition::isVictory() const {
    return hasLivingHero() && m_currentRoomIndex >= m_rooms.size();
}

int Expedition::getTorchLevel() const {
    return m_torchLevel;
}

int Expedition::getGold() const {
    return m_gold;
}

std::size_t Expedition::getCurrentRoomIndex() const {
    return m_currentRoomIndex;
}

std::size_t Expedition::getRoomCount() const {
    return m_rooms.size();
}

std::size_t Expedition::getPartySize() const {
    return m_party.size();
}

std::string Expedition::executeTurn(std::size_t heroIndex,
                                    std::size_t skillIndex,
                                    std::size_t supportTargetIndex) {
    if (!isActive()) {
        return "Expedition has already ended.";
    }

    if (heroIndex >= m_party.size()) {
        return "Invalid hero index.";
    }

    Hero& activeHero = m_party[heroIndex];
    if (!activeHero.isAlive()) {
        return activeHero.getName() + " has fallen and cannot act.";
    }

    Room& currentRoom = m_rooms[m_currentRoomIndex];
    consumeEntryTorchIfNeeded();

    if (currentRoom.getType() != RoomType::Combat) {
        const std::string eventReport = currentRoom.triggerExplorationEvent(activeHero, m_torchLevel, m_gold);
        if (currentRoom.isCleared()) {
            ++m_currentRoomIndex;
            m_roomEntered = false;
        }
        return eventReport;
    }

    if (skillIndex >= activeHero.getSkillCount()) {
        return activeHero.getName() + " does not have a skill at that index.";
    }

    std::ostringstream oss;
    oss << "[" << currentRoom.getName() << "] ";

    bool actionTaken = false;
    if (supportTargetIndex < m_party.size() &&
        activeHero.useSupportSkill(skillIndex, m_party[supportTargetIndex], m_torchLevel)) {
        actionTaken = true;
        oss << activeHero.getName() << " supported " << m_party[supportTargetIndex].getName()
            << " (HP: " << m_party[supportTargetIndex].getHp() << "/"
            << m_party[supportTargetIndex].getMaxHp()
            << ", Stress: " << m_party[supportTargetIndex].getStress() << ").";
    } else {
        const int damage = activeHero.useOffensiveSkill(skillIndex, m_torchLevel);
        if (damage > 0) {
            actionTaken = true;
            const int goldEarned = currentRoom.takeEnemyDamage(damage);
            oss << activeHero.getName() << " dealt " << damage << " DMG";

            if (currentRoom.isCleared()) {
                m_gold += goldEarned;
                ++m_currentRoomIndex;
                m_roomEntered = false;
                oss << " and cleared the room! Earned " << goldEarned << " gold.";
                return oss.str();
            }

            oss << " (Enemy HP left: " << currentRoom.getEnemyHp() << ").";
        }
    }

    if (!actionTaken) {
        return activeHero.getName() + " could not perform the selected skill.";
    }

    if (currentRoom.hasActiveEnemy()) {
        const auto targetIndex = findCounterattackTarget(heroIndex);
        if (targetIndex.has_value()) {
            Hero& target = m_party[*targetIndex];
            currentRoom.performEnemyCounterattack(target, m_torchLevel);
            oss << " Enemy counterattacked " << target.getName()
                << " (HP: " << target.getHp() << "/" << target.getMaxHp()
                << ", Stress: " << target.getStress() << ").";
        }
    }

    return oss.str();
}

std::string Expedition::campAndRecover(int goldBudget) {
    if (!isActive()) {
        return "Cannot camp: expedition is no longer active.";
    }

    if (m_roomEntered && m_rooms[m_currentRoomIndex].hasActiveEnemy()) {
        return "Cannot camp while engaged in combat!";
    }

    if (goldBudget < 10) {
        return "Need at least 10 gold to buy camping supplies.";
    }

    if (m_gold < goldBudget) {
        return "Not enough gold to camp (available: " + std::to_string(m_gold) + ").";
    }

    const int bundles = goldBudget / 10;
    const int actualCost = bundles * 10;
    const int torchGain = bundles * 8;
    const int healGain = bundles * 3;
    const int stressRelief = bundles * 6;

    m_gold -= actualCost;
    m_torchLevel = std::clamp(m_torchLevel + torchGain, 0, 100);

    for (auto& hero : m_party) {
        if (hero.isAlive()) {
            hero.applyRecovery(healGain, stressRelief);
        }
    }

    std::ostringstream oss;
    oss << "Party camped for " << actualCost << " gold: +" << torchGain
        << " torch (now " << m_torchLevel << "), +" << healGain
        << " HP, -" << stressRelief << " stress for all living heroes.";
    return oss.str();
}

std::string Expedition::assessSurvivalRisk() const {
    if (m_party.empty()) {
        return "CRITICAL (100/100) - No heroes in expedition.";
    }

    int livingCount = 0;
    int afflictedCount = 0;
    int totalHp = 0;
    int totalMaxHp = 0;
    int totalStress = 0;

    for (const auto& hero : m_party) {
        if (hero.isAlive()) {
            ++livingCount;
            totalHp += hero.getHp();
            totalMaxHp += hero.getMaxHp();
            totalStress += hero.getStress();
            if (hero.isAfflicted()) {
                ++afflictedCount;
            }
        }
    }

    if (livingCount == 0) {
        return "CRITICAL (100/100) - The entire party has fallen.";
    }

    const int partySize = static_cast<int>(m_party.size());
    const int fallenCount = partySize - livingCount;
    const int avgHpPercent = (totalMaxHp > 0) ? (totalHp * 100) / totalMaxHp : 0;
    const int avgStress = totalStress / livingCount;

    int riskScore = (100 - avgHpPercent) / 2 + avgStress / 5;
    riskScore += afflictedCount * 10;
    riskScore += fallenCount * 15;

    if (m_torchLevel <= 25) {
        riskScore += 20;
    } else if (m_torchLevel < 60) {
        riskScore += 10;
    }

    riskScore = std::clamp(riskScore, 0, 100);

    std::string level = "LOW";
    if (riskScore >= 75) {
        level = "CRITICAL";
    } else if (riskScore >= 50) {
        level = "HIGH";
    } else if (riskScore >= 25) {
        level = "MODERATE";
    }

    std::ostringstream oss;
    oss << level << " (" << riskScore << "/100) | Living: " << livingCount << "/" << partySize
        << " | Avg HP: " << avgHpPercent << "% | Avg Stress: " << avgStress
        << " | Torch: " << m_torchLevel << "/100";
    return oss.str();
}

std::ostream& operator<<(std::ostream& os, const Expedition& expedition) {
    os << "=== Expedition: " << expedition.m_dungeonName << " ===\n"
       << "Torch: " << expedition.m_torchLevel << "/100"
       << " | Gold: " << expedition.m_gold
       << " | Progress: " << expedition.m_currentRoomIndex << "/" << expedition.m_rooms.size()
       << " rooms cleared\n\n";

    os << "-- Party --\n";
    for (const auto& hero : expedition.m_party) {
        os << hero;
    }

    os << "\n-- Dungeon Rooms --\n";
    for (std::size_t i = 0; i < expedition.m_rooms.size(); ++i) {
        os << "  " << (i + 1) << ". " << expedition.m_rooms[i];
        if (i == expedition.m_currentRoomIndex) {
            os << "  <-- CURRENT";
        }
        os << '\n';
    }

    return os;
}