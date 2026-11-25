#include "DungeonLoader.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

namespace {

[[nodiscard]] nlohmann::json readJsonFile(const std::string& filePath) {
    std::ifstream inputFile{filePath};
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open JSON file: " + filePath);
    }

    nlohmann::json data;
    inputFile >> data;
    return data;
}

[[nodiscard]] RoomType parseRoomType(const std::string& typeStr) {
    if (typeStr == "Combat") {
        return RoomType::Combat;
    }
    if (typeStr == "Trap") {
        return RoomType::Trap;
    }
    if (typeStr == "Treasure") {
        return RoomType::Treasure;
    }
    if (typeStr == "Camp") {
        return RoomType::Camp;
    }
    throw std::runtime_error("Unknown room type in JSON: " + typeStr);
}

[[nodiscard]] std::vector<Room> parseRooms(const nlohmann::json& roomsJson) {
    std::vector<Room> rooms;
    rooms.reserve(roomsJson.size());

    for (const auto& entry : roomsJson) {
        std::string name = entry.at("name").get<std::string>();
        const RoomType type = parseRoomType(entry.at("type").get<std::string>());
        const int torchCost = entry.at("torchCost").get<int>();
        std::string encounterName = entry.at("encounterName").get<std::string>();
        const int enemyMaxHp = entry.at("enemyMaxHp").get<int>();
        const int hazardDamage = entry.at("hazardDamage").get<int>();
        const int hazardStress = entry.at("hazardStress").get<int>();
        const int goldReward = entry.at("goldReward").get<int>();

        rooms.emplace_back(std::move(name),
                           type,
                           torchCost,
                           std::move(encounterName),
                           enemyMaxHp,
                           hazardDamage,
                           hazardStress,
                           goldReward);
    }

    return rooms;
}

} // namespace

namespace DungeonLoader {

std::unordered_map<std::string, Skill> loadSkills(const std::string& filePath) {
    const nlohmann::json data = readJsonFile(filePath);
    std::unordered_map<std::string, Skill> skillsByName;

    for (const auto& entry : data.at("skills")) {
        std::string name = entry.at("name").get<std::string>();
        const int baseDamage = entry.at("baseDamage").get<int>();
        const int healAmount = entry.at("healAmount").get<int>();
        const int stressDelta = entry.at("stressDelta").get<int>();
        const int torchDelta = entry.at("torchDelta").get<int>();
        const int accuracy = entry.at("accuracy").get<int>();

        Skill skill{name, baseDamage, healAmount, stressDelta, torchDelta, accuracy};
        skillsByName.emplace(std::move(name), std::move(skill));
    }

    return skillsByName;
}

std::vector<Hero> loadHeroes(const std::string& filePath,
                             const std::unordered_map<std::string, Skill>& skillsByName) {
    const nlohmann::json data = readJsonFile(filePath);
    const auto& heroesJson = data.at("heroes");

    std::vector<Hero> heroes;
    heroes.reserve(heroesJson.size());

    for (const auto& entry : heroesJson) {
        std::string name = entry.at("name").get<std::string>();
        std::string role = entry.at("role").get<std::string>();
        const int maxHp = entry.at("maxHp").get<int>();
        const int defense = entry.at("defense").get<int>();

        const auto& skillListJson = entry.at("skills");
        std::vector<Skill> heroSkills;
        heroSkills.reserve(skillListJson.size());

        for (const auto& skillNameJson : skillListJson) {
            const std::string skillName = skillNameJson.get<std::string>();
            const auto it = skillsByName.find(skillName);
            if (it == skillsByName.end()) {
                throw std::runtime_error("Unknown skill referenced by hero " + name + ": " + skillName);
            }
            heroSkills.push_back(it->second);
        }

        heroes.emplace_back(std::move(name), std::move(role), maxHp, defense, std::move(heroSkills));
    }

    return heroes;
}

Expedition loadExpedition(const std::string& skillsPath,
                          const std::string& heroesPath,
                          const std::string& dungeonPath) {
    const auto skills = loadSkills(skillsPath);
    auto heroes = loadHeroes(heroesPath, skills);

    const nlohmann::json dungeonData = readJsonFile(dungeonPath);
    std::string dungeonName = dungeonData.at("dungeonName").get<std::string>();
    const int initialTorch = dungeonData.at("initialTorch").get<int>();
    const int initialGold = dungeonData.at("initialGold").get<int>();
    auto rooms = parseRooms(dungeonData.at("rooms"));

    return Expedition{std::move(dungeonName),
                      initialTorch,
                      initialGold,
                      std::move(heroes),
                      std::move(rooms)};
}

} // namespace DungeonLoader//