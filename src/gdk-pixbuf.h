/**
 * Copyright (c) 2012 Vadim Ushakov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __WATERLINE__PIXBUF_STUFF_H
#define __WATERLINE__PIXBUF_STUFF_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>

GdkPixbuf * su_gdk_pixbuf_get_from_pixmap(Pixmap xpixmap, int width, int height);
void        su_gdk_pixbuf_dim(GdkPixbuf *pixbuf);
GdkPixbuf * su_gdk_pixbuf_apply_mask(GdkPixbuf * pixbuf, GdkPixbuf * mask);
void        su_gdk_pixbuf_get_pixel(GdkPixbuf *pixbuf,
                                    int x, int y,
                                    unsigned * red, unsigned * green, unsigned * blue, unsigned * alpha);
GdkPixbuf * su_gdk_pixbuf_scale_in_rect(GdkPixbuf * pixmap,
                                        int required_width, int required_height, gboolean good_quality);
void        su_gdk_pixbuf_get_color_sample(GdkPixbuf *pixbuf, GdkColor * c1, GdkColor * c2);
GdkPixbuf * su_gdk_pixbuf_composite_thumb_icon(GdkPixbuf * thumb, GdkPixbuf * icon, int size, int icon_size);

#endif
