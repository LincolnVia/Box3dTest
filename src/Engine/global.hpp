#pragma once

#ifdef BUILD_DEBUG
inline constexpr bool debugMode = true;
#else
inline constexpr bool debugMode = false;
#endif
