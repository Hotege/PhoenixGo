#if !defined(YAO_WIN_CONTROLS_WINDOW_H)
#define YAO_WIN_CONTROLS_WINDOW_H

#include <string>
#include <windows.h>

namespace yw
{
    ATOM register_window_class(HINSTANCE hInstance);
    BOOL init_window();
    VOID get_message();
} // namespace yw


#endif // YAO_WIN_CONTROLS_WINDOW_H