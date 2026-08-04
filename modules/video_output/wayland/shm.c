/**
 * @file shm.c
 * @brief Wayland shared memory video output module for VLC media player
 */
/*****************************************************************************
 * Copyright © 2014, 2017 Rémi Denis-Courmont
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include "viewporter-client-protocol.h"
#include "registry.h"

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include <vlc_fs.h>

#define MAX_PICTURES 4

typedef struct vout_display_sys_t
{
    vlc_window_t *embed; /* VLC window */
    struct wl_event_queue *eventq;
    struct wl_shm *shm;
    struct wp_viewporter *viewporter;
    struct wp_viewport *viewport;

    /* Medea: display-sized scratch buffers used to pad the picture out to the
     * full display area (see Prepare). This is a small rotating pool, not one
     * shared buffer: a single shared buffer was overwritten with the next
     * frame's pixels while the compositor could still be reading the previous
     * one out of the exact same memory, which tore/glitched visibly (most
     * noticeable on the audio-only "now playing" screen, which repaints its
     * text every frame). Rotating across PAD_BUFFER_COUNT independent regions
     * gives the compositor time to finish with slot N-1 while we write slot N,
     * the same reason the ordinary (non-padded) path already cycles across
     * MAX_PICTURES separate picture buffers instead of reusing one. */
    struct wl_shm_pool *pad_pool;
    void *pad_map;
    size_t pad_frame_size;  /* bytes per slot */
    unsigned pad_slot;      /* next slot to write, round-robin */
    unsigned pad_width;
    unsigned pad_height;
    int pad_fd;

    size_t active_buffers;
} vout_display_sys_t;

struct buffer_data
{
    picture_t *picture;
    size_t *counter;
};

static_assert (sizeof (vout_display_sys_t) >= MAX_PICTURES,
               "Pointer arithmetic error");

static void buffer_release_cb(void *data, struct wl_buffer *buffer)
{
    struct buffer_data *d = data;

    picture_Release(d->picture);
    (*(d->counter))--;
    free(d);
    wl_buffer_destroy(buffer);
}

static const struct wl_buffer_listener buffer_cbs =
{
    buffer_release_cb,
};

/* Medea: (re)allocate the rotating padding buffer pool for a given display
 * size. Allocates MAX_PICTURES independent slots inside one mmap/pool so
 * consecutive frames never share backing memory (see the struct comment). */
static int PadEnsure(vout_display_t *vd, unsigned width, unsigned height)
{
    vout_display_sys_t *sys = vd->sys;

    if (sys->pad_map != NULL
     && sys->pad_width == width && sys->pad_height == height)
        return VLC_SUCCESS;

    if (sys->pad_pool != NULL)
    {
        wl_shm_pool_destroy(sys->pad_pool);
        sys->pad_pool = NULL;
    }
    if (sys->pad_map != NULL)
    {
        munmap(sys->pad_map, sys->pad_frame_size * MAX_PICTURES);
        sys->pad_map = NULL;
    }
    if (sys->pad_fd != -1)
    {
        vlc_close(sys->pad_fd);
        sys->pad_fd = -1;
    }

    if (width == 0 || height == 0)
        return VLC_EGENERIC;

    const size_t stride = (size_t)width * 4;
    const size_t frameSize = stride * height;
    const size_t totalSize = frameSize * MAX_PICTURES;

    int fd = vlc_memfd();
    if (fd == -1)
        return VLC_EGENERIC;

    if (ftruncate(fd, totalSize) < 0)
    {
        vlc_close(fd);
        return VLC_EGENERIC;
    }

    void *map = mmap(NULL, totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        vlc_close(fd);
        return VLC_EGENERIC;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(sys->shm, fd, totalSize);
    if (pool == NULL)
    {
        munmap(map, totalSize);
        vlc_close(fd);
        return VLC_EGENERIC;
    }

    sys->pad_fd = fd;
    sys->pad_map = map;
    sys->pad_frame_size = frameSize;
    sys->pad_slot = 0;
    sys->pad_pool = pool;
    sys->pad_width = width;
    sys->pad_height = height;
    return VLC_SUCCESS;
}

static void Prepare(vout_display_t *vd, picture_t *pic,
                    const struct vlc_render_subpicture *subpic,
                    vlc_tick_t date)
{
    VLC_UNUSED(date);
    vout_display_sys_t *sys = vd->sys;
    struct wl_display *display = sys->embed->display.wl;
    struct wl_surface *surface = sys->embed->handle.wl;
    struct picture_buffer_t *picbuf = pic->p_sys;

    if (picbuf->fd == -1)
        return;

    struct buffer_data *d = malloc(sizeof (*d));
    if (unlikely(d == NULL))
        return;

    d->picture = pic;
    d->counter = &sys->active_buffers;

    off_t offset = picbuf->offset;
    const size_t stride = pic->p->i_pitch;
    const size_t size = pic->p->i_lines * stride;
    struct wl_shm_pool *pool;
    struct wl_buffer *buf;

    /* Medea: upstream attached a buffer covering only the video rectangle and
     * then damaged the whole display area. Every pixel outside the video had no
     * buffer content at all, so it composited as fully transparent - the
     * desktop showed through beside pillarboxed video, during resizes, and
     * anywhere the interface was not painting over it.
     *
     * When the placed video does not cover the display, render into a
     * display-sized buffer instead and clear the surround. WL_SHM_FORMAT_XRGB8888
     * carries no alpha, so a zeroed buffer is opaque black rather than
     * see-through. The extra copy only happens in the software path, which is
     * already doing a full swscale conversion per frame. */
    const unsigned dispW = vd->cfg->display.width;
    const unsigned dispH = vd->cfg->display.height;
    const unsigned vidW = vd->fmt->i_visible_width;
    const unsigned vidH = vd->fmt->i_visible_height;
    const bool needPad = (sys->viewport == NULL)
                      && (vidW < dispW || vidH < dispH)
                      && dispW > 0 && dispH > 0;

    if (needPad && PadEnsure(vd, dispW, dispH) == VLC_SUCCESS)
    {
        const size_t dstStride = (size_t)dispW * 4;

        /* Round-robin slot: never write into the memory backing a buffer that
         * may still be attached/being read by the compositor. See the struct
         * comment for why a single shared buffer glitched. */
        const unsigned slot = sys->pad_slot;
        sys->pad_slot = (slot + 1) % MAX_PICTURES;
        const off_t slotOffset = (off_t)slot * sys->pad_frame_size;
        uint8_t *const slotBase = (uint8_t *)sys->pad_map + slotOffset;

        memset(slotBase, 0, sys->pad_frame_size); /* opaque black */

        const uint8_t *src = (const uint8_t *)picbuf->base + picbuf->offset
                           + 4 * vd->fmt->i_x_offset
                           + stride * vd->fmt->i_y_offset;

        /* Centre the picture the same way vout placed it. */
        const unsigned dstX = (dispW > vidW) ? ((dispW - vidW) / 2) : 0;
        const unsigned dstY = (dispH > vidH) ? ((dispH - vidH) / 2) : 0;
        uint8_t *dst = slotBase + dstY * dstStride + dstX * 4;

        const size_t rowBytes = (size_t)vidW * 4;
        for (unsigned y = 0; y < vidH; y++)
            memcpy(dst + y * dstStride, src + (size_t)y * stride, rowBytes);

        buf = wl_shm_pool_create_buffer(sys->pad_pool, slotOffset, dispW, dispH,
                                        dstStride, WL_SHM_FORMAT_XRGB8888);
        if (buf == NULL)
        {
            free(d);
            return;
        }

        picture_Hold(pic);
        wl_buffer_add_listener(buf, &buffer_cbs, d);
        wl_surface_attach(surface, buf, 0, 0);
        wl_surface_damage(surface, 0, 0, dispW, dispH);
        wl_display_flush(display);
        sys->active_buffers++;
        (void) subpic;
        return;
    }

    pool = wl_shm_create_pool(sys->shm, picbuf->fd, offset + size);
    if (pool == NULL)
    {
        free(d);
        return;
    }

    if (sys->viewport == NULL) /* Poor man's crop */
        offset += 4 * vd->fmt->i_x_offset
                  + pic->p->i_pitch * vd->fmt->i_y_offset;

    buf = wl_shm_pool_create_buffer(pool, offset, vd->fmt->i_visible_width,
                                    vd->fmt->i_visible_height, stride,
                                    WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    if (buf == NULL)
    {
        free(d);
        return;
    }

    picture_Hold(pic);

    wl_buffer_add_listener(buf, &buffer_cbs, d);
    wl_surface_attach(surface, buf, 0, 0);
    wl_surface_damage(surface, 0, 0, vd->cfg->display.width, vd->cfg->display.height);
    wl_display_flush(display);

    sys->active_buffers++;

    (void) subpic;
}

static void Display(vout_display_t *vd, picture_t *pic)
{
    vout_display_sys_t *sys = vd->sys;
    struct wl_display *display = sys->embed->display.wl;
    struct wl_surface *surface = sys->embed->handle.wl;

    wl_surface_commit(surface);
    wl_display_roundtrip_queue(display, sys->eventq);

    (void) pic;
}

static int ResetPictures(vout_display_t *vd, video_format_t *fmt)
{
    video_format_t src;
    vout_display_sys_t *sys = vd->sys;
    assert(sys->viewport == NULL);

    video_format_ApplyRotation(&src, vd->source);

    fmt->i_width  = src.i_width * vd->place->width
                                / src.i_visible_width;
    fmt->i_height = src.i_height * vd->place->height
                                    / src.i_visible_height;
    fmt->i_visible_width  = vd->place->width;
    fmt->i_visible_height = vd->place->height;
    fmt->i_x_offset = src.i_x_offset * vd->place->width
                                        / src.i_visible_width;
    fmt->i_y_offset = src.i_y_offset * vd->place->height
                                        / src.i_visible_height;
    return VLC_SUCCESS;
}

static int UpdateViewport(vout_display_t *vd)
{
    vout_display_sys_t *sys = vd->sys;

    if (sys->viewport == NULL)
        return VLC_EGENERIC;

    video_format_t fmt;

    video_format_ApplyRotation(&fmt, vd->source);

    wp_viewport_set_source(sys->viewport,
                    wl_fixed_from_int(fmt.i_x_offset),
                    wl_fixed_from_int(fmt.i_y_offset),
                    wl_fixed_from_int(fmt.i_visible_width),
                    wl_fixed_from_int(fmt.i_visible_height));
    wp_viewport_set_destination(sys->viewport,
                                vd->place->width, vd->place->height);
    return VLC_SUCCESS;
}

static int SetDisplaySize(vout_display_t *vd, unsigned width, unsigned height)
{
    VLC_UNUSED(width); VLC_UNUSED(height);
    return UpdateViewport(vd);
}

static int PlacementChanged(vout_display_t *vd, const vout_display_place_t *place)
{
    VLC_UNUSED(place);
    return UpdateViewport(vd);
}

static int AspectChanged(vout_display_t *vd, const video_format_t *source)
{
    VLC_UNUSED(source);
    return PlacementChanged(vd, NULL);
}

static void shm_format_cb(void *data, struct wl_shm *shm, uint32_t format)
{
    vout_display_t *vd = data;
    char str[4];

    memcpy(str, &format, sizeof (str));

    if (format >= 0x20202020)
        msg_Dbg(vd, "format %.4s (0x%08"PRIx32")", str, format);
    else
        msg_Dbg(vd, "format %4"PRIu32" (0x%08"PRIx32")", format, format);
    (void) shm;
}

static const struct wl_shm_listener shm_cbs =
{
    shm_format_cb,
};

static void Close(vout_display_t *vd)
{
    vout_display_sys_t *sys = vd->sys;
    struct wl_display *display = sys->embed->display.wl;
    struct wl_surface *surface = sys->embed->handle.wl;

    wl_surface_attach(surface, NULL, 0, 0);
    wl_surface_commit(surface);

    /* Wait until all picture buffers are released by the server */
    while (sys->active_buffers > 0) {
        msg_Dbg(vd, "%zu buffer(s) still active", sys->active_buffers);
        wl_display_roundtrip_queue(display, sys->eventq);
    }
    msg_Dbg(vd, "no active buffers left");

    if (sys->pad_pool != NULL)
        wl_shm_pool_destroy(sys->pad_pool);
    if (sys->pad_map != NULL)
        munmap(sys->pad_map, sys->pad_frame_size * MAX_PICTURES);
    if (sys->pad_fd != -1)
        vlc_close(sys->pad_fd);

    if (sys->viewport != NULL)
        wp_viewport_destroy(sys->viewport);
    if (sys->viewporter != NULL)
        wp_viewporter_destroy(sys->viewporter);
    wl_shm_destroy(sys->shm);
    wl_display_flush(display);
    wl_event_queue_destroy(sys->eventq);
    free(sys);
}

static const struct vlc_display_operations ops = {
    .close = Close,
    .prepare = Prepare,
    .display = Display,
    .set_display_size = SetDisplaySize,
    .reset_pictures = ResetPictures,
    .video_place_changed = PlacementChanged,
    .set_source_aspect = AspectChanged,
    .set_source_crop = AspectChanged,
};

static int Open(vout_display_t *vd,
                video_format_t *fmtp, vlc_video_context *context)
{
    if (vd->cfg->window->type != VLC_WINDOW_TYPE_WAYLAND)
        return VLC_EGENERIC;

    vout_display_sys_t *sys = malloc(sizeof (*sys));
    if (unlikely(sys == NULL))
        return VLC_ENOMEM;

    vd->sys = sys;
    sys->embed = NULL;
    sys->eventq = NULL;
    sys->shm = NULL;
    sys->active_buffers = 0;
    sys->pad_pool = NULL;
    sys->pad_map = NULL;
    sys->pad_frame_size = 0;
    sys->pad_slot = 0;
    sys->pad_width = 0;
    sys->pad_height = 0;
    sys->pad_fd = -1;

    /* Get window */
    sys->embed = vd->cfg->window;
    assert(sys->embed != NULL);

    struct wl_display *display = sys->embed->display.wl;
    struct vlc_wl_registry *registry = NULL;

    sys->eventq = wl_display_create_queue(display);
    if (sys->eventq == NULL)
        goto error;

    registry = vlc_wl_registry_get(display, sys->eventq);
    if (registry == NULL)
        goto error;

    sys->shm = vlc_wl_shm_get(registry);
    if (sys->shm == NULL)
        goto error;

    sys->viewporter = (struct wp_viewporter *)
                      vlc_wl_interface_bind(registry, "wp_viewporter",
                                            &wp_viewporter_interface, NULL);

    wl_shm_add_listener(sys->shm, &shm_cbs, vd);
    wl_display_roundtrip_queue(display, sys->eventq);

    struct wl_surface *surface = sys->embed->handle.wl;

    /* Medea: never take the compositor's scaling path here.
     *
     * When wp_viewporter is available, this module lets the compositor scale
     * the picture buffer up to vd->place (compositor-side scale, cheap) and
     * the subsurface is sized to exactly that placed rectangle - it does not
     * cover the rest of the display. Nothing else fills in the surrounding
     * area (the Wayland main surface is intentionally transparent so the
     * embedded video shows through it), so the letterbox/pillarbox bars, the
     * region during an interactive resize, and any area the UI briefly stops
     * painting are left with no buffer content at all and composite as the
     * desktop showing through.
     *
     * Forcing sys->viewport to stay NULL routes through the module's other,
     * already-existing code path (see UpdateViewport(), ResetPictures()):
     * core pre-scales pictures to the placed size via the normal filter
     * chain, and Prepare() below pads that placed picture out to the full
     * display size and clears the surround to opaque black. This is the same
     * fallback this module already uses on compositors that lack
     * wp_viewporter; it is simply never skipped now. sys->viewporter itself
     * is still bound above and torn down in Close() - it is just never used
     * to create a viewport object. */
    sys->viewport = NULL;

    /* Determine our pixel format */
    static const enum wl_output_transform transforms[8] = {
        [ORIENT_TOP_LEFT] = WL_OUTPUT_TRANSFORM_NORMAL,
        [ORIENT_TOP_RIGHT] = WL_OUTPUT_TRANSFORM_FLIPPED,
        [ORIENT_BOTTOM_LEFT] = WL_OUTPUT_TRANSFORM_FLIPPED_180,
        [ORIENT_BOTTOM_RIGHT] = WL_OUTPUT_TRANSFORM_180,
        [ORIENT_LEFT_TOP] = WL_OUTPUT_TRANSFORM_FLIPPED_270,
        [ORIENT_LEFT_BOTTOM] = WL_OUTPUT_TRANSFORM_90,
        [ORIENT_RIGHT_TOP] = WL_OUTPUT_TRANSFORM_270,
        [ORIENT_RIGHT_BOTTOM] = WL_OUTPUT_TRANSFORM_FLIPPED_90,
    };

    if (vlc_wl_interface_get_version(registry, "wl_compositor") >= 2)
    {
        wl_surface_set_buffer_transform(surface,
                                        transforms[fmtp->orientation]);
    }
    else
    {
        video_format_t fmt = *fmtp;
        video_format_ApplyRotation(fmtp, &fmt);
    }

    fmtp->i_chroma = VLC_CODEC_BGRX;

    vd->ops = &ops;

    vlc_wl_registry_destroy(registry);
    (void) context;
    return VLC_SUCCESS;

error:
    if (sys->shm != NULL)
        wl_shm_destroy(sys->shm);

    if (registry != NULL)
        vlc_wl_registry_destroy(registry);

    if (sys->eventq != NULL)
        wl_event_queue_destroy(sys->eventq);
    free(sys);
    return VLC_EGENERIC;
}

vlc_module_begin()
    set_shortname(N_("WL SHM"))
    set_description(N_("Wayland shared memory video output"))
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_callback_display(Open, 170)
    add_shortcut("wl")
vlc_module_end()
