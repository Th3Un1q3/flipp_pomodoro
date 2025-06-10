# Flipp Pomodoro Testing Guide

This document outlines test cases for the Flipp Pomodoro application. As there is currently no dedicated unit testing framework integrated, these tests are described for manual verification or future automation.

## I. Module: `flipp_pomodoro.c` (`FlippPomodoroState` and logic)

This module manages the core Pomodoro timer state and logic, including user-configurable settings.

### 1.1. `flipp_pomodoro__new()`
*   **Test Case:** Initialization of settings.
    *   **Action:** Call `flipp_pomodoro__new()`.
    *   **Expected Output:** The `settings` member of the returned `FlippPomodoroState` should have:
        *   `work_minutes` = 25
        *   `short_break_minutes` = 5
        *   `long_break_minutes` = 30
    *   **Verification:** Inspect the `state->settings` values.

### 1.2. `flipp_pomodoro__set_settings()`
*   **Test Case:** Update all settings.
    *   **Action:**
        1. Create a `FlippPomodoroState* state = flipp_pomodoro__new();`.
        2. Create a `FlippPomodoroSettings new_settings = { .work_minutes = 40, .short_break_minutes = 10, .long_break_minutes = 20 };`.
        3. Call `flipp_pomodoro__set_settings(state, &new_settings);`.
    *   **Expected Output:** `state->settings` should now reflect the `new_settings` values.
        *   `work_minutes` = 40
        *   `short_break_minutes` = 10
        *   `long_break_minutes` = 20
    *   **Verification:** Inspect `state->settings` after the call.

### 1.3. `flipp_pomodoro__get_settings()`
*   **Test Case:** Retrieve current settings.
    *   **Action:**
        1. Create `FlippPomodoroState* state = flipp_pomodoro__new();`.
        2. Modify settings if desired (e.g., using `flipp_pomodoro__set_settings()`).
        3. Call `FlippPomodoroSettings* retrieved_settings = flipp_pomodoro__get_settings(state);`.
    *   **Expected Output:** `retrieved_settings` should point to `state->settings`, and its members should match the current settings in `state`.
    *   **Verification:** Compare `retrieved_settings->work_minutes` etc. with `state->settings.work_minutes`.

### 1.4. `flipp_pomodoro__current_stage_total_duration()`
This function's behavior depends on the current stage and the settings stored in `FlippPomodoroState`.
*   **Pre-condition:** `FlippPomodoroState* state` is initialized, and settings can be default or custom.
*   **Helper:** `TIME_SECONDS_IN_MINUTE` (which is 60)

*   **Test Case 1.4.1:** Focus stage with default settings.
    *   **Action:**
        1. `state = flipp_pomodoro__new();` (ensure stage is Focus, which is default).
        2. Call `duration = flipp_pomodoro__current_stage_total_duration(state);`.
    *   **Expected Output:** `duration` = 25 * 60 = 1500 seconds.
    *   **Verification:** Check `duration`.

*   **Test Case 1.4.2:** Short Break stage with custom settings.
    *   **Action:**
        1. `state = flipp_pomodoro__new();`
        2. `FlippPomodoroSettings new_settings = { .work_minutes = 25, .short_break_minutes = 7, .long_break_minutes = 30 };`
        3. `flipp_pomodoro__set_settings(state, &new_settings);`
        4. Manually advance stage to Short Break (e.g. `state->current_stage_index = 1;` assuming sequence Focus, Rest, Focus, Rest...)
        5. Call `duration = flipp_pomodoro__current_stage_total_duration(state);`.
    *   **Expected Output:** `duration` = 7 * 60 = 420 seconds.
    *   **Verification:** Check `duration`.

*   **Test Case 1.4.3:** Long Break stage with custom settings.
    *   **Action:**
        1. `state = flipp_pomodoro__new();`
        2. `FlippPomodoroSettings new_settings = { .work_minutes = 50, .short_break_minutes = 10, .long_break_minutes = 45 };`
        3. `flipp_pomodoro__set_settings(state, &new_settings);`
        4. Manually advance stage to Long Break (e.g. by setting `state->current_stage_index` appropriately according to `stages_sequence` in `flipp_pomodoro.c`).
        5. Call `duration = flipp_pomodoro__current_stage_total_duration(state);`.
    *   **Expected Output:** `duration` = 45 * 60 = 2700 seconds.
    *   **Verification:** Check `duration`.


## II. View: `flipp_pomodoro_settings_view.c`

This module manages the UI for displaying and modifying timer settings.

### 2.1. Setter Functions (e.g., `flipp_pomodoro_settings_view_set_work_time`)
*   **Pre-condition:** `FlippPomodoroSettingsView* view = flipp_pomodoro_settings_view_alloc();`

*   **Test Case 2.1.1:** Set valid work time.
    *   **Action:** `flipp_pomodoro_settings_view_set_work_time(view, 35);`
    *   **Expected Output:** `view->work_time` (internal model) should be 35.
    *   **Verification:** Inspect `view->work_time` (if accessible directly) or use a corresponding getter.

*   **Test Case 2.1.2:** Set work time above maximum (99).
    *   **Action:** `flipp_pomodoro_settings_view_set_work_time(view, 100);`
    *   **Expected Output:** `view->work_time` should be clamped to 99.
    *   **Verification:** Inspect `view->work_time`.

*   **Test Case 2.1.3:** Set work time to 0 (minimum).
    *   **Action:** `flipp_pomodoro_settings_view_set_work_time(view, 0);`
    *   **Expected Output:** `view->work_time` should be 0.
    *   **Verification:** Inspect `view->work_time`.

*   **(Repeat similar test cases for `_set_short_break_time` and `_set_long_break_time`)**

### 2.2. Getter Functions (e.g., `flipp_pomodoro_settings_view_get_work_time`)
*   **Pre-condition:** `FlippPomodoroSettingsView* view = flipp_pomodoro_settings_view_alloc();`

*   **Test Case 2.2.1:** Get work time after setting it.
    *   **Action:**
        1. `flipp_pomodoro_settings_view_set_work_time(view, 42);`
        2. `uint8_t val = flipp_pomodoro_settings_view_get_work_time(view);`
    *   **Expected Output:** `val` should be 42.
    *   **Verification:** Check `val`.

*   **Test Case 2.2.2:** Get default work time.
    *   **Action:** `uint8_t val = flipp_pomodoro_settings_view_get_work_time(view);` (immediately after alloc)
    *   **Expected Output:** `val` should be the default value set in `_alloc()` (e.g., 25).
    *   **Verification:** Check `val`.

*   **(Repeat similar test cases for `_get_short_break_time` and `_get_long_break_time`)**

### 2.3. Input Handling (in `flipp_pomodoro_settings_view_input_callback`)
Testing input handling is more complex without UI interaction. These tests describe the expected change in the view's model.
*   **Pre-condition:** `FlippPomodoroSettingsView* view = flipp_pomodoro_settings_view_alloc();`
*   **Helper:** `InputEvent event; event.type = InputTypeShort;`

*   **Test Case 2.3.1:** Increment selected item (Work Time).
    *   **Action:**
        1. `view->selected_item = 0;` (Work Time)
        2. `view->work_time = 25;`
        3. `event.key = InputKeyRight;`
        4. `flipp_pomodoro_settings_view_input_callback(&event, view);`
    *   **Expected Output:** `view->work_time` should be 26.
    *   **Verification:** Check `view->work_time`.

*   **Test Case 2.3.2:** Decrement selected item (Short Break Time) at minimum.
    *   **Action:**
        1. `view->selected_item = 1;` (Short Break)
        2. `view->short_break_time = 0;`
        3. `event.key = InputKeyLeft;`
        4. `flipp_pomodoro_settings_view_input_callback(&event, view);`
    *   **Expected Output:** `view->short_break_time` should remain 0.
    *   **Verification:** Check `view->short_break_time`.

*   **Test Case 2.3.3:** Increment selected item (Long Break Time) at maximum.
    *   **Action:**
        1. `view->selected_item = 2;` (Long Break)
        2. `view->long_break_time = 99;`
        3. `event.key = InputKeyRight;`
        4. `flipp_pomodoro_settings_view_input_callback(&event, view);`
    *   **Expected Output:** `view->long_break_time` should remain 99.
    *   **Verification:** Check `view->long_break_time`.

*   **Test Case 2.3.4:** Change selected item (Up/Down).
    *   **Action:**
        1. `view->selected_item = 1;`
        2. `event.key = InputKeyUp;`
        3. `flipp_pomodoro_settings_view_input_callback(&event, view);`
    *   **Expected Output:** `view->selected_item` should be 0.
    *   **Verification:** Check `view->selected_item`.

## III. Scene: `flipp_pomodoro_scene_settings.c`

Integration tests for scenes are typically harder without a running application.
The main logic points to verify are:
*   `on_enter`: The settings view is updated with values from `app->state` (via `flipp_pomodoro__get_settings`).
*   `on_event` (Back button): Values from the settings view are correctly retrieved (via `flipp_pomodoro_settings_view_get_..._time`) and saved to `app->state` (via `flipp_pomodoro__set_settings`).

These would typically be verified by running the app on a device/simulator, navigating to the settings screen, changing values, saving, and then observing if the timer behavior (e.g., stage durations) reflects these new settings.
