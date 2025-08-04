#include "flipp_pomodoro_settings.h"
#include <furi.h>
#include <storage/storage.h>
#include <string.h>

#define SETTINGS_PATH "/ext/flipp_pomodoro_settings.bin"

void flipp_pomodoro_settings_set_default(FlippPomodoroSettings* settings) {
    settings->focus_minutes = 25;
    settings->short_break_minutes = 5;
    settings->long_break_minutes = 30;
}

bool flipp_pomodoro_settings_load(FlippPomodoroSettings* settings) {
    Storage* storage = furi_record_open("storage");
    File* file = storage_file_alloc(storage);

    bool ok = false;
    if(storage_file_open(file, SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        if(storage_file_read(file, settings, sizeof(FlippPomodoroSettings)) == sizeof(FlippPomodoroSettings)) {
            ok = true;
        }
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close("storage");

    if(!ok) {
        flipp_pomodoro_settings_set_default(settings);
    }
    return ok;
}

bool flipp_pomodoro_settings_save(const FlippPomodoroSettings* settings) {
    Storage* storage = furi_record_open("storage");
    File* file = storage_file_alloc(storage);

    bool ok = false;
    if(storage_file_open(file, SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        if(storage_file_write(file, settings, sizeof(FlippPomodoroSettings)) == sizeof(FlippPomodoroSettings)) {
            ok = true;
        }
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close("storage");
    return ok;
}
