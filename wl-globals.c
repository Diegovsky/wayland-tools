#include <stdio.h>
#include <wayland-client.h>

void registry_receive(void *data, struct wl_registry *reg, uint32_t name,
                      const char *iface, uint32_t ver) {
    (void)data;
    (void)name;
    (void)reg;
    printf("%s version %d\n", iface, ver);
}

void noop() {}

static struct wl_registry_listener reg_impl = {
    .global = registry_receive,
    .global_remove = noop,
};

int main() {
    struct wl_display *display = wl_display_connect(NULL);
    struct wl_registry *reg = wl_display_get_registry(display);
    wl_registry_add_listener(reg, &reg_impl, NULL);
    // Register globals and callbacks
    wl_display_roundtrip(display);

    // Receive callbacks.
    // Without this, there is memory leak.
    wl_display_roundtrip(display);

    wl_registry_destroy(reg);

    wl_display_disconnect(display);
    return 0;
}
