#include "pch.h"

void DrawTransitionHook(UGameViewportClient* viewport, void* canvas)
{
	if (viewport->GameInstance)
	{
		if (viewport->World)
		{
			if (viewport->World->PersistentLevel)
			{
				for (int i = 0; i < viewport->World->PersistentLevel->Actors.Num(); i++)
				{
					auto currentActor = viewport->World->PersistentLevel->Actors[i];
				}
			}
		}
	}
	
	DrawTransitionOriginal(viewport, canvas);
}