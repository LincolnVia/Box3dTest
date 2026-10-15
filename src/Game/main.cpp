#include "../Engine/engine.hpp"
#include <filesystem>

namespace {
constexpr const char *kDefaultBspPath = "resources/scenes/initial_box3d.bsp";

void SetProjectWorkingDirectory(const char *executablePath) {
  std::error_code error;
  std::filesystem::path candidate =
      std::filesystem::absolute(executablePath, error).parent_path();
  for (int depth = 0; depth < 4 && !candidate.empty(); ++depth) {
    if (std::filesystem::is_directory(candidate / "resources", error)) {
      std::filesystem::current_path(candidate, error);
      return;
    }
    candidate = candidate.parent_path();
  }
}
} // namespace

int main(int argc, char **argv) {
  SetProjectWorkingDirectory(argv[0]);
  Engine engine("Box3D Game");
  engine.LoadBspLevel(argc > 1 ? argv[1] : kDefaultBspPath);
  engine.Process();

  return 0;
}
