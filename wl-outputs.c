#include <assert.h>
#include <malloc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <wayland-client.h>
#include <wayland-util.h>
#include <xdg-output-unstable-v1-client.h>

/* Change this if you need numbers bigger than 65535 for sizes and resolutions.
 * Keep in mind that increasing integer sizes will increase memory
 * consumption a little. */
typedef short metric_t;

/* Stores extra information about an output */
typedef struct {
    const char *name, *description;
    metric_t log_width, log_height, log_x, log_y;
    struct zxdg_output_v1 *xdg_output;
} xdg_output_info_t;

/* Stores information about an output */
typedef struct {
    metric_t scale;
    struct wl_output *output;
    const char *name, *description;
    struct {
        metric_t x, y, phys_width, phys_height;
        const char *model, *make;
    } geometry;
    struct wl_list link;
    xdg_output_info_t *xinfo;
} output_info_t;

typedef struct {
    struct wl_list *output_link;
    struct zxdg_output_manager_v1 *manager;
} state_t;

#define state_has_xdg(state) ((state)->manager != NULL)

static void noop() {}

/* COPY FUNCTIONS */
static metric_t metric_copy(metric_t num) { return num; }

/* Basically `strdup`, but in C11. */
static char *string_copy(const char *text) {
    size_t length = strlen(text);
    // The extra byte is the null terminator
    length += 1;
    char *new_text = malloc(length);
    strncpy(new_text, text, length);
    // Since `strncpy` does not always NULL-terminate its output string, this is
    // needed to guarantee a proper string.
    new_text[length - 1] = '\0';
    return new_text;
}

/* HELPER MACROS */

/* A setter that receives only 1 argument. Something like
 * object->field = copy(value);
 */
#define DEFINE_SETTER(type, output_type, field_type, member, copy_func)        \
    void type##_set_##member(void *data, struct output_type *wayland_type,     \
                             field_type member) {                              \
        (void)wayland_type;                                                    \
        type##_t *info = data;                                                 \
        info->member = copy_func(member);                                      \
    }

/* A setter that receives 2 arguments. Something like
 * object->field1 = value1;
 * object->field2 = value2;
 */
#define DEFINE_SETTER2(type, output_type, field_type, member1, member2)        \
    void type##_set_##member1##_and_##member2(                                 \
        void *data, struct output_type *wayland_type, field_type member1,      \
        field_type member2) {                                                  \
        (void)wayland_type;                                                    \
        type##_t *info = data;                                                 \
        info->member1 = member1;                                               \
        info->member2 = member2;                                               \
    }

/* WL_OUTPUT CALLBACK STUFF */
DEFINE_SETTER(output_info, wl_output, const char *, name, string_copy)
DEFINE_SETTER(output_info, wl_output, const char *, description, string_copy)
DEFINE_SETTER(output_info, wl_output, int, scale, metric_copy)

// This callback is so massive I couldn't abstract in a macro.
void output_info_set_geometry(void *data, struct wl_output *output, int x,
                              int y, int phys_width, int phys_height,
                              int _subpixel, const char *make,
                              const char *model, int _transform) {
    (void)output;
    (void)_subpixel;
    (void)_transform;
    output_info_t *info = data;
    info->geometry.x = (metric_t)x;
    info->geometry.y = (metric_t)y;
    info->geometry.phys_width = phys_width;
    info->geometry.phys_height = phys_height;
    // info->geometry.subpixel = subpixel;
    info->geometry.make = string_copy(make);
    info->geometry.model = string_copy(model);
    // info->geometry.transform = transform;
}

void output_info_destroy(output_info_t *info) {
    free((char *)info->description);
    free((char *)info->name);
    free((char *)info->geometry.make);
    free((char *)info->geometry.model);
    wl_output_destroy(info->output);
    free(info);
}

static struct wl_output_listener output_listener_impl = {
    .name = output_info_set_name,
    .description = output_info_set_description,
    .scale = output_info_set_scale,
    .geometry = output_info_set_geometry,
    .mode = noop,
    .done = noop,
};

/* XDG_OUTPUT CALLBACK STUFF */

DEFINE_SETTER(xdg_output_info, zxdg_output_v1, const char *, name, string_copy)
DEFINE_SETTER(xdg_output_info, zxdg_output_v1, const char *, description,
              string_copy)
DEFINE_SETTER2(xdg_output_info, zxdg_output_v1, int, log_width, log_height)
DEFINE_SETTER2(xdg_output_info, zxdg_output_v1, int, log_x, log_y)

void xdg_output_info_destroy(xdg_output_info_t *info) {
    free((char *)info->description);
    free((char *)info->name);
    zxdg_output_v1_destroy(info->xdg_output);
    free(info);
}

static struct zxdg_output_v1_listener xdg_output_listener_impl = {
    .name = xdg_output_info_set_name,
    .description = xdg_output_info_set_description,
    .logical_position = xdg_output_info_set_log_x_and_log_y,
    .logical_size = xdg_output_info_set_log_width_and_log_height,
    /* This callback is deprecated, just set it to `noop` */
    .done = noop,
};

/* REGISTRY STUFF */

void registry_receive(void *data, struct wl_registry *reg, uint32_t name,
                      const char *iface, uint32_t ver) {
    (void)name;
    (void)reg;
    state_t *state = data;
    if (strstr(iface, wl_output_interface.name)) {
        struct wl_output *output =
            wl_registry_bind(reg, name, &wl_output_interface, ver);
        output_info_t *info = calloc(1, sizeof(output_info_t));
        info->output = output;
        // wl_list_init(&info->link);
        wl_list_insert(state->output_link, &info->link);
        wl_output_add_listener(output, &output_listener_impl, info);
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
    // Initialize wl_list */
    state.output_link = malloc(sizeof(struct wl_list));
    wl_list_init(state.output_link);

    wl_registry_add_listener(reg, &reg_impl, &state);
    // Register globals and callbacks
    wl_display_roundtrip(display);
    // Receive callbacks
    wl_display_roundtrip(display);

    // Ask for xdg outputs if the compositor supports it
    if (state_has_xdg(&state)) {
        output_info_t *info;
        wl_list_for_each(info, state.output_link, link) {
            struct zxdg_output_v1 *xdg_output =
                zxdg_output_manager_v1_get_xdg_output(state.manager,
                                                      info->output);
            xdg_output_info_t *xinfo = calloc(1, sizeof(xdg_output_info_t));
            info->xinfo = xinfo;
            xinfo->xdg_output = xdg_output;
            zxdg_output_v1_add_listener(xdg_output, &xdg_output_listener_impl,
                                        xinfo);
        }
        // Receive callbacks again
        wl_display_roundtrip(display);
    }
    output_info_t *info;
    wl_list_for_each(info, state.output_link, link) {
        printf("Name: %s\n"
               "Description: %s\n"
               "Scale: %d\n",
               info->name, info->description, info->scale);

        if (state_has_xdg(&state)) {
            xdg_output_info_t *xinfo = info->xinfo;
            printf("XDG Information:\n"
                   "\tName: %s\n"
                   "\tDescription: %s\n"
                   "\tLogical Size: (%d, %d)\n"
                   "\tLogical Position: (%d, %d)\n",
                   xinfo->name, xinfo->description, xinfo->log_width,
                   xinfo->log_height, xinfo->log_x, xinfo->log_y);
        }

        printf("Geometry:\n"
               "\tPosition: (%d, %d)\n"
               "\tSize: (%dmm, %dmm)\n"
               "\tModel: %s\n"
               "\tMaker: %s\n",
               info->geometry.x, info->geometry.y, info->geometry.phys_width,
               info->geometry.phys_height, info->geometry.model,
               info->geometry.make);
        puts("\n");
    }
    size_t outputs_number = wl_list_length(state.output_link);
    printf("Number of Ouputs: %zu\n", outputs_number);

    // Free up resources
    output_info_t *tmp;
    wl_list_for_each_safe(info, tmp, state.output_link, link) {
        xdg_output_info_destroy(info->xinfo);
        wl_list_remove(&info->link);
        output_info_destroy(info);
    }
    free(state.output_link);

    if (state.manager != NULL) {
        zxdg_output_manager_v1_destroy(state.manager);
    }
    wl_registry_destroy(reg);
    // Acknowledge `delete_id` and other callbacks
    wl_display_roundtrip(display);
    // Not needed, but it's polite to.
    wl_display_disconnect(display);
    return 0;
}
