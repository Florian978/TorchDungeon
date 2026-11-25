#include "Hero.h"

#include <algorithm>
#include <iostream>
#include <utility>

Hero::Hero(std::string name,
           std::string role,
           int maxHp,
           int defense,
           std::vector<Skill> skills)
    : m_name{std::move(name)},
      m_role{std::move(role)},
      m_maxHp{std::max(1, maxHp)},
      m_hp{m_maxHp},
      m_stress{0},
      m_defense{std::max(0, defense)},
      m_afflicted{false},
      m_skills{std::move(skills)} {}

Hero::Hero(const Hero& other)
    : m_name{other.m_name},
      m_role{other.m_role},
      m_maxHp{other.m_maxHp},
      m_hp{other.m_hp},
      m_stress{other.m_stress},
      m_defense{other.m_defense},
      m_afflicted{other.m_afflicted},
      m_skills{other.m_skills} {
    std::cout << "[Hero] Copy constructor: " << m_name << '\n';
}

Hero& Hero::operator=(const Hero& other) {
    if (this == &other) {
        return *this;
    }

    m_name = other.m_name;
    m_role = other.m_role;
    m_maxHp = other.m_maxHp;
    m_hp = other.m_hp;
    m_stress = other.m_stress;
    m_defense = other.m_defense;
    m_afflicted = other.m_afflicted;
    m_skills = other.m_skills;

    std::cout << "[Hero] Copy assignment: " << m_name << '\n';
    return *this;
}

Hero::~Hero() {
    std::cout << "[Hero] Destructor: " << m_name << '\n';
}

void Hero::clampStats() {
    m_hp = std::clamp(m_hp, 0, m_maxHp);
    m_stress = std::clamp(m_stress, 0, MAX_STRESS);
}

void Hero::updateMentalState() {
    if (!isAlive()) {
        return;
    }

    if (m_stress >= MAX_STRESS) {
        m_afflicted = true;
        if (m_hp > 1) {
            m_hp = 1;
            m_stress = 170;
        } else {
            m_hp = 0;
        }
        return;
    }

    if (m_stress >= AFFLICTION_THRESHOLD) {
        m_afflicted = true;
    } else {
        m_afflicted = false;
    }
}

const std::string& Hero::getName() const {
    return m_name;
}

int Hero::getHp() const {
    return m_hp;
}

int Hero::getMaxHp() const {
    return m_maxHp;
}

int Hero::getStress() const {
    return m_stress;
}

bool Hero::isAlive() const {
    return m_hp > 0;
}

bool Hero::isAfflicted() const {
    return m_afflicted;
}

std::size_t Hero::getSkillCount() const {
    return m_skills.size();
}

void Hero::receiveImpact(int rawDamage, int stressDelta, int torchLevel) {
    if (!isAlive()) {
        return;
    }

    if (rawDamage > 0) {
        const int mitigatedDamage = std::max(1, rawDamage - m_defense);
        m_hp -= mitigatedDamage;
    }

    if (stressDelta > 0) {
        int finalStress = stressDelta;
        if (torchLevel <= 25) {
            finalStress = (finalStress * 125) / 100;
        }
        if (m_afflicted) {
            finalStress = (finalStress * 110) / 100;
        }
        m_stress += finalStress;
    } else if (stressDelta < 0) {
        m_stress += stressDelta;
    }

    clampStats();
    updateMentalState();
}

void Hero::applyRecovery(int healAmount, int stressRelief) {
    if (!isAlive()) {
        return;
    }

    if (healAmount > 0) {
        m_hp += healAmount;
    }
    if (stressRelief > 0) {
        m_stress -= stressRelief;
    }

    clampStats();
    updateMentalState();
}

int Hero::useOffensiveSkill(std::size_t skillIndex, int& torchLevel) {
    if (!isAlive() || skillIndex >= m_skills.size()) {
        return 0;
    }

    const Skill& skill = m_skills[skillIndex];
    if (skill.isSupportSkill()) {
        return 0;
    }

    int damage = skill.calculateEffectivePower(torchLevel);
    if (m_afflicted && damage > 0) {
        damage = std::max(1, (damage * 85) / 100);
    }

    torchLevel = std::clamp(torchLevel + skill.getTorchDelta(), 0, 100);

    if (skill.getStressDelta() != 0) {
        m_stress += skill.getStressDelta();
        clampStats();
        updateMentalState();
    }

    return damage;
}

bool Hero::useSupportSkill(std::size_t skillIndex, Hero& target, int& torchLevel) const {
    if (!isAlive() || !target.isAlive() || skillIndex >= m_skills.size()) {
        return false;
    }

    const Skill& skill = m_skills[skillIndex];
    if (!skill.isSupportSkill()) {
        return false;
    }

    int heal = skill.calculateEffectivePower(torchLevel);
    if (m_afflicted && heal > 0) {
        heal = std::max(1, (heal * 85) / 100);
    }

    const int stressRelief = skill.getStressDelta() < 0 ? -skill.getStressDelta() : 0;
    target.applyRecovery(heal, stressRelief);
    torchLevel = std::clamp(torchLevel + skill.getTorchDelta(), 0, 100);
    return true;
}

std::ostream& operator<<(std::ostream& os, const Hero& hero) {
    os << hero.m_name << " (" << hero.m_role << ")"
       << " | HP: " << hero.m_hp << "/" << hero.m_maxHp
       << " | Stress: " << hero.m_stress << "/" << Hero::MAX_STRESS
       << " | DEF: " << hero.m_defense
       << " | Status: ";

    if (!hero.isAlive()) {
        os << "Fallen";
    } else if (hero.m_afflicted) {
        os << "Afflicted";
    } else {
        os << "Steady";
    }

    os << "\n  Skills:\n";
    for (const auto& skill : hero.m_skills) {
        os << "    - " << skill << '\n';
    }
    return os;
}