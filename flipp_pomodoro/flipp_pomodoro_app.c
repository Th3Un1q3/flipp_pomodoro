#include "flipp_pomodoro_app_i.h"

FlippPomodoroApp* flipp_pomodoro_app_alloc() {
    // TODO: add free method
    FlippPomodoroApp* app_context = malloc(sizeof(FlippPomodoroApp));
    FlippPomodoroState pomodoro_state = flipp_pomodoro__new();
    app_context->state = &pomodoro_state;

    return app_context;
}

/// @brief Actions to be processed in a queue
typedef enum {
    TimerTickType = 42,
    InputEventType,
} ActionType;

/// @brief Single action contains type and payload
typedef struct {
    ActionType type;
    void* payload;
} Action;

// static const Icon* stage_background_image[] = {
//     [Work] = &I_flipp_pomodoro_work_64,
//     [Rest] = &I_flipp_pomodoro_rest_64,
// };


// // Screen is 128x64 px
// static void app_draw_callback(Canvas* canvas, void* ctx) {
//     // WARNING: place no side-effects into rener cycle!!
//     FlippPomodoroApp* app_context = ctx;

//     const TimeDifference remaining_stage_time = flipp_pomodoro__stage_remaining_duration(app_context->state);

//     // Format remaining stage time;
//     FuriString* timer_string = furi_string_alloc();
//     furi_string_printf(timer_string, "%02u:%02u", remaining_stage_time.minutes, remaining_stage_time.seconds);
//     const char* remaining_stage_time_string = furi_string_get_cstr(timer_string);

//     // Render interface
//     canvas_clear(canvas);

//     canvas_draw_icon(canvas, 0, 0, stage_background_image[app_context->state->stage]);

//     // Countdown section
//     const uint8_t countdown_box_height = canvas_height(canvas)*0.4;
//     const uint8_t countdown_box_width = canvas_width(canvas)*0.5;
//     const uint8_t countdown_box_x = canvas_width(canvas) - countdown_box_width - 2;
//     const uint8_t countdown_box_y = 0;

//     elements_bold_rounded_frame(canvas,
//         countdown_box_x,
//         countdown_box_y,
//         countdown_box_width,
//         countdown_box_height
//     );

//     canvas_set_font(canvas, FontBigNumbers);
//     canvas_draw_str_aligned(
//         canvas,
//         countdown_box_x + (countdown_box_width/2),
//         countdown_box_y + (countdown_box_height/2),
//         AlignCenter,
//         AlignCenter,
//         remaining_stage_time_string
//     );

//     // Draw layout
//     canvas_set_font(canvas, FontSecondary);
//     elements_button_right(canvas, flipp_pomodoro__next_stage_label(app_context->state));


//     //  Cleanup
//     furi_string_free(timer_string);
// }

static void clock_tick_callback(void* ctx) { // TODO: move to timer view
    furi_assert(ctx);
    FuriMessageQueue* queue = ctx;
    Action action = {.type = TimerTickType};
    view_dispatcher_send_custom_event();
    // furi_message_queue_put(, &action, 0);
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    Action action = {.type=InputEventType, .payload=input_event};

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, &action, FuriWaitForever);
};

static bool is_input_event(Action action) {
    return action.type == InputEventType;
}

static bool is_press_or_repeat(InputEvent* input_event) {
    return (input_event->type == InputTypePress) || (input_event->type == InputTypeRepeat);
}

static bool is_key_eq(InputEvent* input_event, InputKey expected_key) {
    return input_event->key == expected_key;
}

static bool is_button_back_pressed(Action action) {
   return is_input_event(action)
    && is_press_or_repeat(action.payload)
    && is_key_eq(action.payload, InputKeyBack);
}

static bool is_button_right_pressed(Action action) {
    return is_input_event(action)
    && is_press_or_repeat(action.payload)
    && is_key_eq(action.payload, InputKeyRight);
}

static bool is_timer_tick(Action action) {
    return action.type == TimerTickType;
};

static bool flipp_pomodoro_app_back_event_callback(void* context) {
    furi_assert(context);
    FlippPomodoroApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static bool flipp_pomodoro_app_custom_event_callback(FlippPomodoroApp* app, uint32_t event) {
    bool consumed = false;
    switch (event)
    {
    case FlippPomodoroAppCustomEventTogglePomodoroManual:
        flipp_pomodoro__toggle_stage(app->state);
        consumed = true;
        break;
    case FlippPomodoroAppCustomEventTogglePomodoroNatural:
        if(app->state->stage == Work) {
            // REGISTER a deed on work stage complete to get an acheivement
            DOLPHIN_DEED(DolphinDeedPluginGameWin);
        };

        flipp_pomodoro__toggle_stage(app->state);
        notification_message(app->notification_app, stage_start_notification_sequence_map[app->state->stage]);
        consumed = true;
        break;
    default:
        break;
    }
    return consumed; // Returns true for event consumed;
}

int32_t flipp_pomodoro_main(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(Action));
    ViewPort* view_port = view_port_alloc();
    FlippPomodoroState state = flipp_pomodoro__new();

    // Configure view port
    FlippPomodoroApp* app = flipp_pomodoro_app_alloc(); // TODO: free every alloc eventually and move declarations inside the alloc
    app->scene_manager = scene_manager_alloc(&flipp_pomodoro_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->notification_app = furi_record_open(RECORD_NOTIFICATION);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flipp_pomodoro_app_custom_event_callback);
    gui_add_view_port(app->gui, view_port, GuiLayerFullscreen);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, flipp_pomodoro_app_back_event_callback);
    
    View* timer_view = flipp_pomodoro_view_timer_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        FlippPomodoroAppViewTimer,
        timer_view);

   

    scene_manager_next_scene(app->scene_manager, FlippPomodoroSceneTimer);

    view_dispatcher_run(app->view_dispatcher);


    // view_port_draw_callback_set(view_port, app_draw_callback, app);
    // view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Initiate timer
    FuriTimer* timer = furi_timer_alloc(clock_tick_callback, FuriTimerTypePeriodic, app);
    furi_timer_start(timer, 500);

 



    // bool running = true;

    // const int queue_reading_window_tics = 200;

    // while(running) {
    //     Action action;
    //     if(furi_message_queue_get(event_queue, &action, queue_reading_window_tics) != FuriStatusOk) {
    //         // Queue read is failed
    //         continue;
    //     };

    //     if(!action.type) {
    //         // No item in queue
    //         continue;
    //     }

    //     if(is_button_back_pressed(action)) {
    //         running = false;
    //         continue;
    //     };

    //     if(is_timer_tick(action) && flipp_pomodoro__is_stage_expired(&state)) {
    //         if(state.stage == Work) {
    //             // REGISTER a deed on work stage complete to get an acheivement
    //             DOLPHIN_DEED(DolphinDeedPluginGameWin);
    //         };

    //         flipp_pomodoro__toggle_stage(&state);

    //         notification_message(notification_app, stage_start_notification_sequence_map[state.stage]);
    //     }

    //     if(is_button_right_pressed(action)) {
    //         flipp_pomodoro__toggle_stage(&state);
    //     };

    //     view_port_update(view_port); // Only re-draw on event
    // }

    // view_port_enabled_set(view_port, false);
    // gui_remove_view_port(gui, view_port);
    // view_port_free(view_port);
    // furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_timer_free(timer);
    flipp_pomodoro__destroy(&state);

    return 0;
}