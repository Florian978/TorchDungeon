#ifndef TORCH_DUNGEON_HERO_H
#define TORCH_DUNGEON_HERO_H

#include "Skill.h"

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

class Hero {
private:
    static constexpr int MAX_STRESS = 200;
    static constexpr int AFFLICTION_THRESHOLD = 100;

    std::string m_name;
    std::string m_role;
    int m_maxHp;
    int m_hp;
    int m_stress;
    int m_defense;
    bool m_afflicted;
    std::vector<Skill> m_skills;

    void clampStats();
    void updateMentalState();

public:
    Hero(std::string name,
         std::string role,
         int maxHp,
         int defense,
         std::vector<Skill> skills);

    Hero(const Hero& other);
    Hero& operator=(const Hero& other);
    ~Hero();

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] int getHp() const;
    [[nodiscard]] int getMaxHp() const;
    [[nodiscard]] int getStress() const;
    [[nodiscard]] bool isAlive() const;
    [[nodiscard]] bool isAfflicted() const;
    [[nodiscard]] std::size_t getSkillCount() const;

    void receiveImpact(int rawDamage, int stressDelta, int torchLevel);
    void applyRecovery(int healAmount, int stressRelief);
    [[nodiscard]] int useOffensiveSkill(std::size_t skillIndex, int& torchLevel);
    bool useSupportSkill(std::size_t skillIndex, Hero& target, int& torchLevel) const;

    [[nodiscard]] bool operator==(const Hero& other) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Hero& hero);
};

#endif // TORCH_DUNGEON_HERO_H