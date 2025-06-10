#include "flipp_pomodoro_settings_storage.h"
#include "../modules/flipp_pomodoro.h" // For FlippPomodoroState and FlippPomodoroSettings
#include <storage/storage.h>
#include <furi.h> // For FURI_LOG_D, FURI_LOG_E, etc.

#define TAG "FlippPomodoroSettingsStorage"

// Helper function to apply default settings
static void apply_default_settings(FlippPomodoroSettings* settings) {
    settings->work_minutes = 25;
    settings->short_break_minutes = 5;
    settings->long_break_minutes = 30;
    FURI_LOG_I(TAG, "Applied default settings");
}

// Helper function to validate settings values
static bool validate_settings(FlippPomodoroSettings* settings) {
    bool valid = true;
    if (settings->work_minutes > 99) {
        FURI_LOG_W(TAG, "Invalid work_minutes %u, resetting to default.", settings->work_minutes);
        settings->work_minutes = 25; // Or some other default for this specific field
        valid = false; // Indicate that at least one value was invalid
    }
    if (settings->short_break_minutes > 99) {
        FURI_LOG_W(TAG, "Invalid short_break_minutes %u, resetting to default.", settings->short_break_minutes);
        settings->short_break_minutes = 5;
        valid = false;
    }
    if (settings->long_break_minutes > 99) {
        FURI_LOG_W(TAG, "Invalid long_break_minutes %u, resetting to default.", settings->long_break_minutes);
        settings->long_break_minutes = 30;
        valid = false;
    }
    // If any value was invalid, it might be good to consider all settings compromised
    // and reload all defaults, or handle individually as done here.
    // For simplicity, individual reset is shown. If 'valid' is false, the caller might decide to save these corrected settings.
    return valid;
}


void flipp_pomodoro_settings_load(FlippPomodoroState* state) {
    furi_assert(state);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperApplicationFile* file = storage_file_alloc(storage);
    bool settings_loaded_successfully = false;

    FURI_LOG_I(TAG, "Attempting to load settings from %s", FLIPP_POMODORO_SETTINGS_FILE_PATH);

    if(storage_file_open(file, FLIPP_POMODORO_SETTINGS_FILE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_D(TAG, "File opened successfully for reading.");
        FlippPomodoroSettings temp_settings;
        if(storage_file_read(file, &temp_settings, sizeof(FlippPomodoroSettings)) == sizeof(FlippPomodoroSettings)) {
            FURI_LOG_D(TAG, "Successfully read %d bytes from file.", sizeof(FlippPomodoroSettings));
            if(validate_settings(&temp_settings)) {
                state->settings = temp_settings;
                settings_loaded_successfully = true;
                FURI_LOG_I(TAG, "Settings loaded and validated successfully: W:%u, S:%u, L:%u",
                    state->settings.work_minutes,
                    state->settings.short_break_minutes,
                    state->settings.long_break_minutes);
            } else {
                // Validation failed, some values were corrected to defaults.
                // We'll use these corrected (partially default) settings.
                state->settings = temp_settings;
                // It might be good to save these corrected settings back.
                // For now, we proceed with the corrected ones.
                FURI_LOG_W(TAG, "Settings validation failed, some values were reset. Using corrected values.");
                // Consider settings_loaded_successfully = true; here if corrected values are acceptable.
                // For now, if any value was out of bounds, we'll treat it as a partial success
                // and the corrected values are now in state->settings.
                // The next save operation would persist these corrections.
                settings_loaded_successfully = true; // Or false if we want to force a full default overwrite and save.
                                                // Let's say true, as validate_settings already corrected them.
            }
        } else {
            FURI_LOG_E(TAG, "Failed to read settings data from file or size mismatch.");
        }
        storage_file_close(file);
    } else {
        FURI_LOG_W(TAG, "Failed to open settings file: %s. File may not exist or is inaccessible.", storage_file_get_error_desc(file));
    }

    if(!settings_loaded_successfully) {
        FURI_LOG_I(TAG, "Applying default settings because loading failed or validation issues.");
        apply_default_settings(&state->settings);
        // Attempt to save these default settings to create the file or overwrite a corrupt one.
        if(flipp_pomodoro_settings_save(state)) {
            FURI_LOG_I(TAG, "Default settings saved to storage.");
        } else {
            FURI_LOG_E(TAG, "Failed to save default settings to storage.");
        }
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool flipp_pomodoro_settings_save(FlippPomodoroState* state) {
    furi_assert(state);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperApplicationFile* file = storage_file_alloc(storage);
    bool success = false;

    FURI_LOG_I(TAG, "Attempting to save settings to %s", FLIPP_POMODORO_SETTINGS_FILE_PATH);

    if(storage_file_open(file, FLIPP_POMODORO_SETTINGS_FILE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_D(TAG, "File opened successfully for writing.");
        size_t bytes_written = storage_file_write(file, &state->settings, sizeof(FlippPomodoroSettings));
        if(bytes_written == sizeof(FlippPomodoroSettings)) {
            success = true;
            FURI_LOG_I(TAG, "Settings saved successfully: W:%u, S:%u, L:%u",
                state->settings.work_minutes,
                state->settings.short_break_minutes,
                state->settings.long_break_minutes);
        } else {
            FURI_LOG_E(TAG, "Failed to write complete settings data to file. Expected %d, wrote %d", sizeof(FlippPomodoroSettings), bytes_written);
        }
        storage_file_close(file);
    } else {
        FURI_LOG_E(TAG, "Failed to open settings file for writing: %s", storage_file_get_error_desc(file));
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return success;
}
