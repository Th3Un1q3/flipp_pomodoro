#include <furi.h>
#include <gui/scene_manager.h>

enum
{
    SceneEventConusmed = true,
    SceneEventNotConusmed = false
};

uint8_t ExitSignal = 0;



void flipp_pomodoro_scene_info_on_next_stage(void *ctx)
{
    furi_assert(ctx);
};

void flipp_pomodoro_scene_info_on_enter(void *ctx)
{
    furi_assert(ctx);
};

bool flipp_pomodoro_scene_info_on_event(void *ctx, SceneManagerEvent event)
{
    furi_assert(ctx);

    switch (event.type)
    {
    case SceneManagerEventTypeBack:
        return ExitSignal;
    default:
        break;
    };
    return SceneEventNotConusmed;
};

void flipp_pomodoro_scene_info_on_exit(void *ctx)
{
    UNUSED(ctx);
};