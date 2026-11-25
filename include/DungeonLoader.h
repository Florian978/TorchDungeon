#ifndef TORCH_DUNGEON_DUNGEON_LOADER_H
#define TORCH_DUNGEON_DUNGEON_LOADER_H

#include "Expedition.h"
#include "Hero.h"
#include "Room.h"
#include "Skill.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace DungeonLoader {

    [[nodiscard]] std::unordered_map<std::string, Skill> loadSkills(const std::string& filePath);
    [[nodiscard]] std::vector<Hero> loadHeroes(const std::string& filePath,
                                               const std::unordered_map<std::string, Skill>& skillsByName);
    [[nodiscard]] Expedition loadExpedition(const std::string& skillsPath,
                                            const std::string& heroesPath,
                                            const std::string& dungeonPath);

} // namespace DungeonLoader

#endif // TORCH_DUNGEON_DUNGEON_LOADER_H