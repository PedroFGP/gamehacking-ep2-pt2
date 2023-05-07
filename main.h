#pragma once
#include "pch.h"

typedef void(__thiscall* DrawTransitionOriginalType)(UGameViewportClient*, void*);
extern DrawTransitionOriginalType DrawTransitionOriginal;

void Initialize();