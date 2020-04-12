#include "controls/window.h"

int main(int argc, char const *argv[])
{
    if (!yw::register_window_class(GetModuleHandle(nullptr)))
        return -1;
    if (!yw::init_window())
        return -1;
    yw::get_message();
    return 0;
}
