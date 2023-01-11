typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    FlippPomodoroAppCustomEventTogglePomodoroManual = 100,
    FlippPomodoroAppCustomEventTogglePomodoroNatural, // By expiration
    RpcDebugAppCustomEventInputDataExchange,
    RpcDebugAppCustomEventRpcDataExchange,
} FlippPomodoroAppCustomEvent;

typedef struct {
    FlippPomodoroState* state;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notification_app;
} FlippPomodoroApp;

typedef enum {
    FlippPomodoroAppViewTimer,
} FlippPomodoroAppView;