#pragma once

#include <gui/scene_manager.h>

// Define the scene handlers
void flipp_pomodoro_scene_settings_on_enter(void* context);
bool flipp_pomodoro_scene_settings_on_event(void* context, SceneManagerEvent event);
void flipp_pomodoro_scene_settings_on_exit(void* context);
