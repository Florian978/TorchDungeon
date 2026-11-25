#include "Skill.h"

#include <algorithm>
#include <utility>

Skill::Skill(std::string name,
             int baseDamage,
             int healAmount,
             int stressDelta,
             int torchDelta,
             int accuracy)
    : m_name{std::move(name)},
      m_baseDamage{std::max(0, baseDamage)},
      m_healAmount{std::max(0, healAmount)},
      m_stressDelta{stressDelta},
      m_torchDelta{torchDelta},
      m_accuracy{std::clamp(accuracy, 0, 100)} {}

const std::string& Skill::getName() const {
    return m_name;
}

int Skill::getStressDelta() const {
    return m_stressDelta;
}

int Skill::getTorchDelta() const {
    return m_torchDelta;
}

bool Skill::isSupportSkill() const {
    return m_healAmount > 0 || (m_baseDamage == 0 && m_stressDelta < 0);
}

int Skill::calculateEffectivePower(int torchLevel) const {
    const int clampedTorch = std::clamp(torchLevel, 0, 100);
    const int basePower = isSupportSkill() ? m_healAmount : m_baseDamage;

    if (basePower <= 0) {
        return 0;
    }

    if (isSupportSkill()) {
        if (clampedTorch >= 75) {
            return (basePower * 115) / 100;
        }
        if (clampedTorch <= 25) {
            return (basePower * 85) / 100;
        }
        return basePower;
    }

    int effectiveDamage = basePower;
    if (clampedTorch <= 25) {
        effectiveDamage = (basePower * 120) / 100;
    } else if (clampedTorch < 75) {
        effectiveDamage = (basePower * 105) / 100;
    }

    if (m_accuracy < 80 && clampedTorch <= 25) {
        effectiveDamage = (effectiveDamage * m_accuracy) / 100;
    }

    return std::max(1, effectiveDamage);
}

std::ostream& operator<<(std::ostream& os, const Skill& skill) {
    os << skill.m_name << " [DMG: " << skill.m_baseDamage
       << ", HEAL: " << skill.m_healAmount
       << ", STRESS: " << skill.m_stressDelta
       << ", TORCH: " << skill.m_torchDelta
       << ", ACC: " << skill.m_accuracy << "%]";
    return os;
}