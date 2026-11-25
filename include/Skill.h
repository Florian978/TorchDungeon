#ifndef TORCH_DUNGEON_SKILL_H
#define TORCH_DUNGEON_SKILL_H

#include <ostream>
#include <string>

class Skill {
private:
    std::string m_name;
    int m_baseDamage;
    int m_healAmount;
    int m_stressDelta;
    int m_torchDelta;
    int m_accuracy;

public:
    Skill(std::string name,
          int baseDamage,
          int healAmount,
          int stressDelta,
          int torchDelta,
          int accuracy);

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] int getStressDelta() const;
    [[nodiscard]] int getTorchDelta() const;
    [[nodiscard]] bool isSupportSkill() const;

    [[nodiscard]] int calculateEffectivePower(int torchLevel) const;

    [[nodiscard]] bool operator==(const Skill& other) const = default;
    friend std::ostream& operator<<(std::ostream& os, const Skill& skill);
};

#endif // TORCH_DUNGEON_SKILL_H