#include <cassert>
#include <chrono>
#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#include <SFML/Graphics.hpp>

#include "DungeonLoader.h"
#include "Expedition.h"
#include "Hero.h"
#include "ResourceManager.hpp"
#include "Skill.h"

namespace {

void inspectSkills(const std::unordered_map<std::string, Skill>& skills) {
    std::cout << "=== Loaded Skills (" << skills.size() << ") ===\n";
    for (const auto& [key, skill] : skills) {
        std::cout << " - " << skill.getName()
                  << " (key: " << key
                  << ", support: " << (skill.isSupportSkill() ? "yes" : "no")
                  << ", power@bright: " << skill.calculateEffectivePower(85)
                  << ", power@dark: " << skill.calculateEffectivePower(15)
                  << ", stress: " << skill.getStressDelta()
                  << ", torch: " << skill.getTorchDelta() << ")\n"
                  << "   " << skill << '\n';
    }
    std::cout << '\n';
}

void testHeroRuleOfThree(const Hero& sampleHero) {
    std::cout << "=== Testing Hero Rule of Three ===\n";

    Hero copyHero{sampleHero};
    std::cout << "[cc] Copied hero: " << copyHero.getName()
              << " (HP: " << copyHero.getHp() << ")\n";
    assert(copyHero == sampleHero);

    copyHero.receiveImpact(10, 15, 50);
    std::cout << "[cc] After modifying copy -> copy HP: " << copyHero.getHp()
              << ", original HP: " << sampleHero.getHp() << '\n';
    assert(copyHero != sampleHero);

    Hero assignedHero{sampleHero};
    assignedHero = copyHero;
    std::cout << "[op=] Assigned hero HP: " << assignedHero.getHp() << '\n';
    assert(assignedHero == copyHero);

    assignedHero.applyRecovery(5, 10);
    std::cout << "[op=] After modifying assigned -> assigned HP: " << assignedHero.getHp()
              << ", copy HP: " << copyHero.getHp() << '\n';
    assert(assignedHero != copyHero);

    std::cout << "=== Rule of Three checks passed ===\n\n";
}

void runConsoleScenario(Expedition& expedition) {
    std::cout << "=== TorchDungeon Expedition Started ===\n";
    std::cout << "Commands: status | risk | turn <heroIdx> <skillIdx> <targetIdx> | camp <gold> | quit\n\n";

    std::string command;
    while (std::cin >> command) {
        if (command == "status") {
            std::cout << expedition << '\n';
        } else if (command == "risk") {
            std::cout << "[Risk] " << expedition.assessSurvivalRisk() << '\n';
        } else if (command == "turn") {
            std::size_t heroIdx = 0;
            std::size_t skillIdx = 0;
            std::size_t targetIdx = 0;
            if (std::cin >> heroIdx >> skillIdx >> targetIdx) {
                std::cout << "[Turn] " << expedition.executeTurn(heroIdx, skillIdx, targetIdx) << '\n';
            }
        } else if (command == "camp") {
            int goldBudget = 0;
            if (std::cin >> goldBudget) {
                std::cout << "[Camp] " << expedition.campAndRecover(goldBudget) << '\n';
            }
        } else if (command == "quit") {
            std::cout << "Exiting expedition loop.\n";
            break;
        } else {
            std::cout << "Unknown command: " << command << '\n';
        }
    }

    std::cout << "\n=== Expedition Summary ===\n";
    std::cout << "Result: "
              << (expedition.isVictory() ? "VICTORY" : (expedition.isActive() ? "IN PROGRESS" : "DEFEAT"))
              << "\nRooms cleared: " << expedition.getCurrentRoomIndex() << "/" << expedition.getRoomCount()
              << "\nParty size: " << expedition.getPartySize()
              << "\nFinal torch: " << expedition.getTorchLevel() << "/100"
              << "\nFinal gold: " << expedition.getGold()
              << "\nFinal risk: " << expedition.assessSurvivalRisk() << "\n\n";
}

void showExpeditionWindow(const Expedition& expedition) {
    sf::RenderWindow window;
    // NOTE: synced with env variable APP_WINDOW from .github/workflows/cmake.yml
    window.create(sf::VideoMode({800, 700}), "My Window", sf::Style::Default);
    window.setFramerateLimit(60);
    std::cout << "Fereastra a fost creată\n";

    const sf::Font font = ResourceManager::Instance().getFont("FiraSans-Regular.ttf");

    sf::Text titleText{font};
    titleText.setString("TorchDungeon - Expedition Report");
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color{245, 198, 92});
    titleText.setPosition({40.f, 35.f});

    std::ostringstream summaryStream;
    summaryStream << "Outcome: "
                  << (expedition.isVictory() ? "VICTORY" : (expedition.isActive() ? "IN PROGRESS" : "DEFEAT"))
                  << "\nRooms cleared: " << expedition.getCurrentRoomIndex() << " / " << expedition.getRoomCount()
                  << "\nParty size: " << expedition.getPartySize()
                  << "\nGold collected: " << expedition.getGold()
                  << "\nTorch level: " << expedition.getTorchLevel() << " / 100"
                  << "\nRisk assessment:\n  " << expedition.assessSurvivalRisk()
                  << "\n\nPress ESC to close window.";

    sf::Text summaryText{font};
    summaryText.setString(summaryStream.str());
    summaryText.setCharacterSize(20);
    summaryText.setFillColor(sf::Color{230, 230, 230});
    summaryText.setPosition({40.f, 110.f});

    sf::RectangleShape torchBarBg{{400.f, 26.f}};
    torchBarBg.setFillColor(sf::Color{45, 45, 55});
    torchBarBg.setOutlineThickness(2.f);
    torchBarBg.setOutlineColor(sf::Color{180, 140, 70});
    torchBarBg.setPosition({40.f, 360.f});

    const float torchRatio = static_cast<float>(expedition.getTorchLevel()) / 100.f;
    sf::RectangleShape torchBarFill{{400.f * torchRatio, 26.f}};
    torchBarFill.setFillColor(expedition.getTorchLevel() <= 25
                                  ? sf::Color{185, 65, 55}
                                  : sf::Color{245, 175, 55});
    torchBarFill.setPosition({40.f, 360.f});

    while (window.isOpen()) {
        bool shouldExit = false;

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                std::cout << "Fereastra a fost închisă\n";
            } else if (event->is<sf::Event::Resized>()) {
                std::cout << "New width: " << window.getSize().x << '\n'
                          << "New height: " << window.getSize().y << '\n';
            } else if (event->is<sf::Event::KeyPressed>()) {
                const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                std::cout << "Received key "
                          << (keyPressed->scancode == sf::Keyboard::Scancode::X ? "X" : "(other)")
                          << '\n';
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    shouldExit = true;
                }
            }
        }

        if (shouldExit) {
            window.close();
            std::cout << "Fereastra a fost închisă (shouldExit == true)\n";
            break;
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);

        window.clear(sf::Color{20, 18, 26});
        window.draw(titleText);
        window.draw(summaryText);
        window.draw(torchBarBg);
        window.draw(torchBarFill);
        window.display();
    }
}

} // namespace

int main() {
    try {
        const auto skills = DungeonLoader::loadSkills("assets/skills.json");
        inspectSkills(skills);

        const auto heroes = DungeonLoader::loadHeroes("assets/heroes.json", skills);
        if (!heroes.empty()) {
            testHeroRuleOfThree(heroes.front());
        }

        Expedition expedition = DungeonLoader::loadExpedition(
            "assets/skills.json",
            "assets/heroes.json",
            "assets/dungeon.json"
        );

        runConsoleScenario(expedition);
        showExpeditionWindow(expedition);
    } catch (const std::exception& exp) {
        std::cerr << "Error: " << exp.what() << '\n';
        return 1;
    }

    std::cout << "Programul a terminat execuția\n";
    return 0;
}