#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/view.h>
#include "../flipp_pomodoro_app.h"
#include "../modules/flipp_pomodoro.h"
// Auto-compiled icons
#include "../flipp_pomodoro_icons.h"

typedef struct FilppPomodoroTimerView {
    View* view;
    // NEXT STAGE CALLBACK
    void* context;
};

static const Icon* stage_background_image[] = {
    [Work] = &I_flipp_pomodoro_work_64,
    [Rest] = &I_flipp_pomodoro_rest_64,
};

static void flipp_pomodoro_view_timer_draw_callback(Canvas* canvas, void* ctx) {
    // WARNING: place no side-effects into rener cycle!!
    FlippPomodoroApp* app_context = ctx;

    const TimeDifference remaining_stage_time = flipp_pomodoro__stage_remaining_duration(app_context->state);

    // Format remaining stage time;
    FuriString* timer_string = furi_string_alloc();
    furi_string_printf(timer_string, "%02u:%02u", remaining_stage_time.minutes, remaining_stage_time.seconds);
    const char* remaining_stage_time_string = furi_string_get_cstr(timer_string);

    // Render interface
    canvas_clear(canvas);

    canvas_draw_icon(canvas, 0, 0, stage_background_image[app_context->state->stage]);

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

    // Draw layout
    canvas_set_font(canvas, FontSecondary);
    elements_button_right(canvas, flipp_pomodoro__next_stage_label(app_context->state));


    //  Cleanup
    furi_string_free(timer_string);
}

void flipp_pomodoro_view_timer_input_callback(InputEvent* event, void* context) {

}

FilppPomodoroTimerView* flipp_pomodoro_view_timer_alloc(void* ctx) {
    FilppPomodoroTimerView* timer = malloc(sizeof(FilppPomodoroTimerView));
    timer->view = view_alloc();

    view_set_context(timer->view, timer);
    view_set_draw_callback(timer->view, flipp_pomodoro_view_timer_draw_callback);
    view_set_input_callback(timer->view, flipp_pomodoro_view_timer_input_callback);

    return timer;
};