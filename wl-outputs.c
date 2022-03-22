#include <stdio.h>
#include <string.h>

#include <wayland-client.h>
#include <xdg-output-unstable-v1-client.h>

#define MAX_OUTPUTS 10l

typedef struct {
    struct wl_output *output;
    const char *name, *description;
    int scale;
    struct {
        int x, y, phys_width, phys_height;
        const char *model, *make;
    } geometry;
} output_info_t;

static output_info_t outputs[MAX_OUTPUTS] = {0};

typedef struct {
    struct zxdg_output_v1 *xdg_output;
    const char *name, *description;
    int log_width, log_height, log_x, log_y;
} xdg_output_info_t;

typedef struct {
    size_t output_number;
    struct zxdg_output_manager_v1 *manager;
} state_t;

#define state_has_xdg(state) ((state)->manager != NULL)

static xdg_output_info_t xdg_outputs[MAX_OUTPUTS] = {0};

void noop() {}

/* HELPER MACROS */

#define DEFINE_SETTER(type, output_type, field_type, name)                     \
    void type##_set_##name(void *data, struct output_type *wayland_type,       \
                           field_type name) {                                  \
        (void)wayland_type;                                                    \
        type##_t *info = data;                                                 \
        info->name = name;                                                     \
    }

#define DEFINE_SETTER2(type, output_type, field_type, name1, name2)            \
    void type##_set_##name1##_and_##name2(                                     \
        void *data, struct output_type *wayland_type, field_type name1,        \
        field_type name2) {                                                    \
        (void)wayland_type;                                                    \
        type##_t *info = data;                                                 \
        info->name1 = name1;                                                   \
        info->name2 = name2;                                                   \
    }

#define DEFINE_DONE(type, output_type, name)                                   \
    void type##_done(void *data, struct output_type *name) {                   \
        type##_t *info = data;                                                 \
        info->name = name;                                                     \
    }

/* WL_OUTPUT CALLBACK STUFF */

DEFINE_SETTER(output_info, wl_output, const char *, name)
DEFINE_SETTER(output_info, wl_output, const char *, description)
DEFINE_SETTER(output_info, wl_output, int, scale)
DEFINE_DONE(output_info, wl_output, output)

void output_info_set_geometry(void *data, struct wl_output *output, int x,
                              int y, int phys_width, int phys_height,
                              int _subpixel, const char *make,
                              const char *model, int _transform) {
    (void)output;
    (void)_subpixel;
    (void)_transform;
    output_info_t *info = data;
    info->geometry.x = x;
    info->geometry.y = y;
    info->geometry.phys_width = phys_width;
    info->geometry.phys_height = phys_height;
    // info->geometry.subpixel = subpixel;
    info->geometry.make = make;
    info->geometry.model = model;
    // info->geometry.transform = transform;
}

static struct wl_output_listener output_listener_impl = {
    .name = output_info_set_name,
    .description = output_info_set_description,
    .scale = output_info_set_scale,
    .geometry = output_info_set_geometry,
    .mode = noop,
    .done = output_info_done,
};

/* XDG_OUTPUT CALLBACK STUFF */

DEFINE_SETTER(xdg_output_info, zxdg_output_v1, const char *, name)
DEFINE_SETTER(xdg_output_info, zxdg_output_v1, const char *, description)
DEFINE_SETTER2(xdg_output_info, zxdg_output_v1, int, log_width, log_height)
DEFINE_SETTER2(xdg_output_info, zxdg_output_v1, int, log_x, log_y)
DEFINE_DONE(xdg_output_info, zxdg_output_v1, xdg_output)

static struct zxdg_output_v1_listener xdg_output_listener_impl = {
    .name = xdg_output_info_set_name,
    .description = xdg_output_info_set_description,
    .logical_position = xdg_output_info_set_log_x_and_log_y,
    .logical_size = xdg_output_info_set_log_width_and_log_height,
    .done = xdg_output_info_done,
};

/* REGISTRY STUFF */

void registry_receive(void *data, struct wl_registry *reg, uint32_t name,
                      const char *iface, uint32_t ver) {
    (void)name;
    (void)reg;
    state_t *state = data;
    if (strstr(iface, wl_output_interface.name)) {
        if (state->output_number >= MAX_OUTPUTS) {
            fprintf(stderr,
                    "Max number of displays reached (%ld), cannot get more "
                    "information.\n"
                    "",
                    MAX_OUTPUTS);
            return;
        }
        struct wl_output *output =
            wl_registry_bind(reg, name, &wl_output_interface, ver);
        wl_output_add_listener(output, &output_listener_impl,
                               &outputs[state->output_number]);
        state->output_number += 1;
    } else if (strstr(iface, zxdg_output_manager_v1_interface.name)) {
        state->manager =
            wl_registry_bind(reg, name, &zxdg_output_manager_v1_interface, ver);
    }
}

static struct wl_registry_listener reg_impl = {
    .global = registry_receive,
    .global_remove = noop,
};

int main() {
    struct wl_display *display = wl_display_connect(NULL);
    struct wl_registry *reg = wl_display_get_registry(display);
    state_t state = {0};
    wl_registry_add_listener(reg, &reg_impl, &state);
    // Register globals
    wl_display_roundtrip(display);
    // Receive callbacks
    wl_display_roundtrip(display);

    // Ask for xdg outputs if the compositor supports it
    if (state_has_xdg(&state)) {
        for (size_t i = 0; i < state.output_number; i++) {
            struct zxdg_output_v1 *xdg_output =
                zxdg_output_manager_v1_get_xdg_output(state.manager,
                                                      outputs[i].output);
            zxdg_output_v1_add_listener(xdg_output, &xdg_output_listener_impl,
                                        &xdg_outputs[i]);
        }
        // Receive callbacks again
        wl_display_roundtrip(display);
    }
    for (size_t i = 0; i < state.output_number; i++) {
        output_info_t info = outputs[i];
        printf("Name: %s\n"
               "Description: %s\n"
               "Scale: %d\n",
               info.name, info.description, info.scale);

        if (state_has_xdg(&state)) {
            xdg_output_info_t xinfo = xdg_outputs[i];
            printf("XDG Information:\n"
                   "\tName: %s\n"
                   "\tDescription: %s\n"
                   "\tLogical Size: (%d, %d)\n"
                   "\tLogical Position: (%d, %d)\n",
                   xinfo.name, xinfo.description, xinfo.log_width,
                   xinfo.log_height, xinfo.log_x, xinfo.log_y);
        }

        printf("Geometry:\n"
               "\tPosition: (%d, %d)\n"
               "\tSize: (%dmm, %dmm)\n"
               "\tModel: %s\n"
               "\tMaker: %s\n",
               info.geometry.x, info.geometry.y, info.geometry.phys_width,
               info.geometry.phys_height, info.geometry.model,
               info.geometry.make);
        puts("\n");
    }
    return 0;
}
