#include "flipp_pomodoro_settings_view.h"
#include <gui/elements.h>
#include <furi.h>

// Define the view structure
struct FlippPomodoroSettingsView {
    View* view;
    FlippPomodoroSettingsViewInputCallback input_callback;
    void* input_context;
    uint8_t work_time;
    uint8_t short_break_time;
    uint8_t long_break_time;
    uint8_t selected_item; // 0: Work, 1: Short Break, 2: Long Break
};

// Draw callback for the view
static void flipp_pomodoro_settings_view_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    FlippPomodoroSettingsView* view = context;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);

    char buffer[16];

    // Display "Work" label and value
    snprintf(buffer, sizeof(buffer), "Work: %02u", view->work_time);
    elements_multiline_text_aligned(canvas, 10, 10, AlignLeft, AlignTop, buffer);
    if (view->selected_item == 0) {
        elements_frame(canvas, 5, 5, 60, 12); // Highlight selected item
    }

    // Display "Short Break" label and value
    snprintf(buffer, sizeof(buffer), "Short Break: %02u", view->short_break_time);
    elements_multiline_text_aligned(canvas, 10, 25, AlignLeft, AlignTop, buffer);
    if (view->selected_item == 1) {
        elements_frame(canvas, 5, 20, 100, 12); // Highlight selected item
    }

    // Display "Long Break" label and value
    snprintf(buffer, sizeof(buffer), "Long Break: %02u", view->long_break_time);
    elements_multiline_text_aligned(canvas, 10, 40, AlignLeft, AlignTop, buffer);
    if (view->selected_item == 2) {
        elements_frame(canvas, 5, 35, 100, 12); // Highlight selected item
    }
}

// Input callback for the view
static bool flipp_pomodoro_settings_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    FlippPomodoroSettingsView* view = context;
    bool consumed = false;

    if (event->type == InputTypeShort) {
        if (event->key == InputKeyUp) {
            if (view->selected_item > 0) {
                view->selected_item--;
            }
            consumed = true;
        } else if (event->key == InputKeyDown) {
            if (view->selected_item < 2) {
                view->selected_item++;
            }
            consumed = true;
        } else if (event->key == InputKeyLeft) {
            if (view->selected_item == 0) { // Work
                if (view->work_time > 0) view->work_time--;
            } else if (view->selected_item == 1) { // Short Break
                if (view->short_break_time > 0) view->short_break_time--;
            } else if (view->selected_item == 2) { // Long Break
                if (view->long_break_time > 0) view->long_break_time--;
            }
            consumed = true;
        } else if (event->key == InputKeyRight) {
            if (view->selected_item == 0) { // Work
                if (view->work_time < 99) view->work_time++;
            } else if (view->selected_item == 1) { // Short Break
                if (view->short_break_time < 99) view->short_break_time++;
            } else if (view->selected_item == 2) { // Long Break
                if (view->long_break_time < 99) view->long_break_time++;
            }
            consumed = true;
        } else if (event->key == InputKeyOk) {
            // Placeholder for confirming selection or action
            // For now, OK button might be used to confirm settings if we change the flow from auto-save on back.
            // Or it could cycle through fields, or open a specific editor for the field.
            // Currently, it calls a generic input_callback.
            // Will be implemented in a later step
            if (view->input_callback) {
                view->input_callback(view->input_context);
            }
            consumed = true;
        }
    }

    if (consumed) {
        view_commit_models(view->view); // Redraw the view
    }

    return consumed;
}

// Allocate and initialize the settings view
FlippPomodoroSettingsView* flipp_pomodoro_settings_view_alloc() {
    FlippPomodoroSettingsView* view = malloc(sizeof(FlippPomodoroSettingsView));
    view->view = view_alloc();
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, flipp_pomodoro_settings_view_draw_callback);
    view_set_input_callback(view->view, flipp_pomodoro_settings_view_input_callback);

    // Initialize with placeholder values
    view->work_time = 25;
    view->short_break_time = 5;
    view->long_break_time = 30;
    view->selected_item = 0; // Default to selecting "Work"

    return view;
}

// Free the settings view
void flipp_pomodoro_settings_view_free(FlippPomodoroSettingsView* view) {
    furi_assert(view);
    view_free(view->view);
    free(view);
}

// Get the underlying view object
View* flipp_pomodoro_settings_view_get_view(FlippPomodoroSettingsView* view) {
    furi_assert(view);
    return view->view;
}

// Set placeholder values
void flipp_pomodoro_settings_view_set_work_time(FlippPomodoroSettingsView* view, uint8_t work_time) {
    furi_assert(view);
    if (work_time > 99) work_time = 99; // Constrain to 0-99
    view->work_time = work_time;
    view_commit_models(view->view);
}

void flipp_pomodoro_settings_view_set_short_break_time(FlippPomodoroSettingsView* view, uint8_t short_break_time) {
    furi_assert(view);
    if (short_break_time > 99) short_break_time = 99; // Constrain to 0-99
    view->short_break_time = short_break_time;
    view_commit_models(view->view);
}

void flipp_pomodoro_settings_view_set_long_break_time(FlippPomodoroSettingsView* view, uint8_t long_break_time) {
    furi_assert(view);
    if (long_break_time > 99) long_break_time = 99; // Constrain to 0-99
    view->long_break_time = long_break_time;
    view_commit_models(view->view);
}

// Set input callback
void flipp_pomodoro_settings_view_set_input_callback(
    FlippPomodoroSettingsView* view,
    FlippPomodoroSettingsViewInputCallback callback,
    void* context) {
    furi_assert(view);
    view->input_callback = callback;
    view->input_context = context;
}

// Getter functions for settings values
uint8_t flipp_pomodoro_settings_view_get_work_time(FlippPomodoroSettingsView* view) {
    furi_assert(view);
    return view->work_time;
}

uint8_t flipp_pomodoro_settings_view_get_short_break_time(FlippPomodoroSettingsView* view) {
    furi_assert(view);
    return view->short_break_time;
}

uint8_t flipp_pomodoro_settings_view_get_long_break_time(FlippPomodoroSettingsView* view) {
    furi_assert(view);
    return view->long_break_time;
}
