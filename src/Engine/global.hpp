#pragma once

#ifdef BUILD_DEBUG
inline constexpr bool debugMode = true;
#else
inline constexpr bool debugMode = false;
#endif
inline constexpr bool useHDR = true;

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif