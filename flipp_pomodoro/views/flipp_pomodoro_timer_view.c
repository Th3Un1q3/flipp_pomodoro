#include "flipp_pomodoro_timer_view.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view.h>
#include "../helpers/debug.h"
#include "../flipp_pomodoro_app.h"
#include "../modules/flipp_pomodoro.h"
// Auto-compiled icons
#include "flipp_pomodoro_icons.h"

struct FlippPomodoroTimerView {
    View* view;
};

typedef struct {
    FlippPomodoroState* state;
} FlippPomodoroTimerViewModel;

static const Icon* stage_background_image[] = {
    [Work] = &I_flipp_pomodoro_work_64,
    [Rest] = &I_flipp_pomodoro_rest_64,
};

static void flipp_pomodoro_view_timer_draw_callback(Canvas* canvas, void* _model) {
    if(!_model) {
        return;
    };
    UNUSED(stage_background_image);
    UNUSED(canvas);
    FlippPomodoroTimerViewModel* model = _model;
    UNUSED(model);

    const TimeDifference remaining_stage_time = flipp_pomodoro__stage_remaining_duration(model->state);
    // Format remaining stage time;
    FuriString* timer_string = furi_string_alloc();
    furi_string_printf(timer_string, "%02u:%02u", remaining_stage_time.minutes, remaining_stage_time.seconds);
    const char* remaining_stage_time_string = furi_string_get_cstr(timer_string);

    // // Render interface
    canvas_clear(canvas);

    canvas_draw_icon(canvas, 0, 0, stage_background_image[model->state->stage]);

    // Countdown section
    const uint8_t countdown_box_height = canvas_height(canvas)*0.4;
    const uint8_t countdown_box_width = canvas_width(canvas)*0.5;
    const uint8_t countdown_box_x = canvas_width(canvas) - countdown_box_width - 2;
    const uint8_t countdown_box_y = 0;

    elements_bold_rounded_frame(canvas,
        countdown_box_x,
        countdown_box_y,
        countdown_box_width,
        countdown_box_height
    );

    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas,
        countdown_box_x + (countdown_box_width/2),
        countdown_box_y + (countdown_box_height/2),
        AlignCenter,
        AlignCenter,
        remaining_stage_time_string
    );

    // // Draw layout
    canvas_set_font(canvas, FontSecondary);

    // elements_button_right(canvas, "Next");
    elements_button_right(canvas, flipp_pomodoro__next_stage_label(model->state));


    // //  Cleanup
    furi_string_free(timer_string);
};

bool flipp_pomodoro_view_timer_input_callback(InputEvent* event, void* context) {
    FURI_LOG_I(TAG, "flipp_pomodoro_view_timer_input_callback:{key:%d}", event->key);
    furi_assert(context);
    // TODO: attach controls and emit custom events on dispatch
    return true;
};

View* flipp_pomodoro_view_timer_get_view(FlippPomodoroTimerView* timer) {
    furi_assert(timer);
    return timer->view;
};

FlippPomodoroTimerView* flipp_pomodoro_view_timer_alloc(void* ctx) {
    furi_assert(ctx);
    FlippPomodoroApp* app = ctx;
    FURI_LOG_I(TAG, "flipp_pomodoro_view_timer_alloc");
    FlippPomodoroTimerView* timer = malloc(sizeof(FlippPomodoroTimerView));
    timer->view = view_alloc();

    view_allocate_model(timer->view, ViewModelTypeLockFree, sizeof(FlippPomodoroTimerViewModel));
    view_set_context(timer->view, app);
    view_set_draw_callback(timer->view, flipp_pomodoro_view_timer_draw_callback);
    view_set_input_callback(timer->view, flipp_pomodoro_view_timer_input_callback);

    return timer;
};


void flipp_pomodoro_view_timer_set_state(View* view, FlippPomodoroState* state) {
    furi_assert(view);
    furi_assert(state);
    with_view_model(
        view,
        FlippPomodoroTimerViewModel * model,
        {
            model->state = state;
        },
        false
    );
};


void flipp_pomodoro_view_timer_free(FlippPomodoroTimerView* timer) {
    furi_assert(timer);
    view_free(timer->view);
    free(timer);
};