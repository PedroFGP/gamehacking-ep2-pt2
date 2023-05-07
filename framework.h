#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <Psapi.h>
#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <locale>

#include "engine.h"
#include "main.h"
#include "hook.h"