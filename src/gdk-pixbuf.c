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

#include <sde-utils-gtk/gdk-pixbuf.h>
#include <glib.h>
#include <string.h>

//#define PIXBUF_INTERP GDK_INTERP_NEAREST
//#define PIXBUF_INTERP GDK_INTERP_TILES
//#define PIXBUF_INTERP GDK_INTERP_BILINEAR
#define PIXBUF_INTERP_GOOD_QUALITY GDK_INTERP_HYPER
#define PIXBUF_INTERP_POOR_QUALITY GDK_INTERP_BILINEAR

GKeyFile* get_settings();

/******************************************************************************/

/* Get a pixbuf from a pixmap.
 * Originally from libwnck, Copyright (C) 2001 Havoc Pennington. */
GdkPixbuf * su_gdk_pixbuf_get_from_pixmap(Pixmap xpixmap, int width, int height)
{
    /* Get the drawable. */
    GdkDrawable * drawable = (GdkDrawable *) gdk_xid_table_lookup(xpixmap);
    if (drawable != NULL)
        g_object_ref(G_OBJECT(drawable));
    else
        drawable = gdk_pixmap_foreign_new(xpixmap);

    GdkColormap * colormap = NULL;
    GdkPixbuf * retval = NULL;
    if (drawable != NULL)
    {
        if (width < 0 || height < 0)
        {
            width = 1;
            height = 1;
            gdk_pixmap_get_size(GDK_PIXMAP(drawable), &width, &height);
        }

        /* Get the colormap.
         * If the drawable has no colormap, use no colormap or the system colormap as recommended in the documentation of gdk_drawable_get_colormap. */
        colormap = gdk_drawable_get_colormap(drawable);
        gint depth = gdk_drawable_get_depth(drawable);
        if (colormap != NULL)
            g_object_ref(G_OBJECT(colormap));
        else if (depth == 1)
            colormap = NULL;
        else
        {
            if (depth == 32)
                colormap = gdk_screen_get_rgba_colormap(gdk_drawable_get_screen(drawable));
            else
                colormap = gdk_screen_get_rgb_colormap(gdk_drawable_get_screen(drawable));
            if (!colormap)
                colormap = gdk_screen_get_system_colormap(gdk_drawable_get_screen(drawable));
            g_object_ref(G_OBJECT(colormap));
        }

        /* Be sure we aren't going to fail due to visual mismatch. */
        if ((colormap != NULL) && (gdk_colormap_get_visual(colormap)->depth != depth))
        {
            g_object_unref(G_OBJECT(colormap));
            colormap = NULL;
        }

        /* Do the major work. */
        retval = gdk_pixbuf_get_from_drawable(NULL, drawable, colormap, 0, 0, 0, 0, width, height);
    }

    /* Clean up and return. */
    if (colormap != NULL)
        g_object_unref(G_OBJECT(colormap));
    if (drawable != NULL)
        g_object_unref(G_OBJECT(drawable));
    return retval;
}

/* http://git.gnome.org/browse/libwnck/tree/libwnck/tasklist.c?h=gnome-2-30 */

void su_gdk_pixbuf_dim(GdkPixbuf *pixbuf)
{
    int x, y, pixel_stride, row_stride;
    guchar *row, *pixels;
    int w, h;
    int i;

    if (!pixbuf)
        return;

    if (!gdk_pixbuf_get_has_alpha(pixbuf))
        return;

    gdouble alpha_multiplier = g_key_file_get_double(get_settings(), "Dim", "AlphaMultiplier", NULL);
    gdouble rgb_offset       = g_key_file_get_double(get_settings(), "Dim", "RGBOffset", NULL);
    gdouble desaturation     = g_key_file_get_double(get_settings(), "Dim", "Desaturation", NULL);

    w = gdk_pixbuf_get_width(pixbuf);
    h = gdk_pixbuf_get_height(pixbuf);

    pixel_stride = 4;

    row = gdk_pixbuf_get_pixels(pixbuf);
    row_stride = gdk_pixbuf_get_rowstride(pixbuf);

    for (y = 0; y < h; y++)
    {
        pixels = row;

        for (x = 0; x < w; x++)
        {
            if (desaturation)
            {
                int gray = (pixels[0] + pixels[1] + pixels[2]) / 3;
                pixels[0] = pixels[0] * (1.0 - desaturation) + gray * desaturation;
                pixels[1] = pixels[1] * (1.0 - desaturation) + gray * desaturation;
                pixels[2] = pixels[2] * (1.0 - desaturation) + gray * desaturation;
            }

            if (rgb_offset)
            {
                for (i = 0; i < 3; i++)
                {
                    int v = pixels[i] + rgb_offset * 255;
                    if (v < 0)
                        v = 0;
                    if (v > 255)
                        v = 255;
                    pixels[i] = v;
                }
            }

            pixels[3] *= alpha_multiplier;
            pixels += pixel_stride;
        }
        row += row_stride;
    }
}

/* Apply a mask to a pixbuf.
 * Originally from libwnck, Copyright (C) 2001 Havoc Pennington. */
GdkPixbuf * su_gdk_pixbuf_apply_mask(GdkPixbuf * pixbuf, GdkPixbuf * mask)
{
    /* Initialize. */
    int w = MIN(gdk_pixbuf_get_width(mask), gdk_pixbuf_get_width(pixbuf));
    int h = MIN(gdk_pixbuf_get_height(mask), gdk_pixbuf_get_height(pixbuf));
    GdkPixbuf * with_alpha = gdk_pixbuf_add_alpha(pixbuf, FALSE, 0, 0, 0);
    guchar * dst = gdk_pixbuf_get_pixels(with_alpha);
    guchar * src = gdk_pixbuf_get_pixels(mask);
    int dst_stride = gdk_pixbuf_get_rowstride(with_alpha);
    int src_stride = gdk_pixbuf_get_rowstride(mask);

    /* Loop to do the work. */
    int i;
    for (i = 0; i < h; i += 1)
    {
        int j;
        for (j = 0; j < w; j += 1)
        {
            guchar * s = src + i * src_stride + j * 3;
            guchar * d = dst + i * dst_stride + j * 4;

            /* s[0] == s[1] == s[2], they are 255 if the bit was set, 0 otherwise. */
            d[3] = ((s[0] == 0) ? 0 : 255);	/* 0 = transparent, 255 = opaque */
        }
    }

    return with_alpha;
}

/******************************************************************************/

void su_gdk_pixbuf_get_pixel (GdkPixbuf *pixbuf, int x, int y, unsigned * red, unsigned * green, unsigned * blue, unsigned * alpha)
{
    int width, height, rowstride, n_channels;
    guchar *pixels, *p;

    n_channels = gdk_pixbuf_get_n_channels (pixbuf);

    g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
    //g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
    //g_assert (n_channels == 4);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    g_assert (x >= 0 && x < width);
    g_assert (y >= 0 && y < height);

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    p = pixels + y * rowstride + x * n_channels;
    if (red)
        *red   = p[0];
    if (green)
        *green = p[1];
    if (blue)
        *blue  = p[2];
    if (alpha)
        *alpha = (n_channels > 3) ? p[3] : 0;
}

GdkPixbuf * su_gdk_pixbuf_scale_in_rect(GdkPixbuf * pixmap, int required_width, int required_height, gboolean good_quality)
{
    /* If we got a pixmap, scale it and return it. */
    if (pixmap == NULL)
        return NULL;
    else
    {
        gulong w = gdk_pixbuf_get_width (pixmap);
        gulong h = gdk_pixbuf_get_height (pixmap);

        gulong w1 = w;
        gulong h1 = h;

        if ((w > (unsigned int)required_width) || (h > (unsigned int)required_height))
        {
            float rw = required_width;
            float rh = required_height;
            float sw = w;
            float sh = h;

            float scalew = rw / sw;
            float scaleh = rh / sh;
            float scale = scalew < scaleh ? scalew : scaleh;

            sw *= scale;
            sh *= scale;

            w = sw;
            h = sh;

            if (w < 2)
                w = 2;
            if (h < 2)
                h = 2;
        }

        GdkInterpType interp = good_quality ?
            PIXBUF_INTERP_GOOD_QUALITY:
            PIXBUF_INTERP_POOR_QUALITY;

        /* Attemp to avoid hang up in gdk_pixbuf_scale_simple(). */
        interp =
        ((w < 3 && h < 3) ||
         (w < 5 && h < 5 && w1 > 200 && h1 > 2000) ) ? GDK_INTERP_NEAREST : interp;

        //g_print("w = %d, h = %d --> w = %d, h = %d\n", w1, h1, w, h);

        GdkPixbuf * ret = gdk_pixbuf_scale_simple(pixmap, w, h, interp);

        return ret;
    }
}

void su_gdk_pixbuf_get_color_sample (GdkPixbuf *pixbuf, GdkColor * c1, GdkColor * c2)
{
    /* scale pixbuff down */

    GdkPixbuf * p1 = su_gdk_pixbuf_scale_in_rect(pixbuf, 3, 3, FALSE);

    gulong pw = gdk_pixbuf_get_width(p1);
    gulong ph = gdk_pixbuf_get_height(p1);

    gdouble r = 0, g = 0, b = 0;

    unsigned r1, g1, b1, a; int samples_count = 0;

    /* pick colors */

    if (pw < 1 || ph < 1)
    {
        r1 += 128; g1 += 128; b1 += 128; samples_count++;
    }
    else if (pw == 1 && ph == 1)
    {
        su_gdk_pixbuf_get_pixel(p1, 0, 0, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
    }
    else
    {
        if (pw > 1 && ph > 1)
            su_gdk_pixbuf_get_pixel(p1, 1, 1, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
        if (ph > 1)
            su_gdk_pixbuf_get_pixel(p1, 0, 1, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
        if (pw > 1)
            su_gdk_pixbuf_get_pixel(p1, 1, 0, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
        if (ph > 2)
            su_gdk_pixbuf_get_pixel(p1, 0, 2, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
        if (pw > 2)
            su_gdk_pixbuf_get_pixel(p1, 2, 0, &r1, &g1, &b1, &a); r += r1; g += g1; b += b1; samples_count++;
    }

    g_object_unref(p1);

    /* to range (0,1) */

    r /= (samples_count * 255); g /= (samples_count * 255); b /= (samples_count * 255);

    if (r > 1)
        r = 1;
    if (g > 1)
        g = 1;
    if (b > 1)
        b = 1;

    gdouble h = 0, s = 0, v = 0;

    /* adjust saturation and value */

    gtk_rgb_to_hsv(r, g, b, &h, &s, &v);

    gdouble saturation_min = g_key_file_get_double(get_settings(), "ColorSample", "SaturationMin", NULL);
    gdouble saturation_max = g_key_file_get_double(get_settings(), "ColorSample", "SaturationMax", NULL);
    gdouble value_min = g_key_file_get_double(get_settings(), "ColorSample", "ValueMin", NULL);
    gdouble value_max = g_key_file_get_double(get_settings(), "ColorSample", "ValueMax", NULL);
    gdouble saturation_delta = g_key_file_get_double(get_settings(), "ColorSample", "SaturationDelta", NULL);
    gdouble value_delta = g_key_file_get_double(get_settings(), "ColorSample", "ValueDelta", NULL);

    if (s < saturation_min)
        s = saturation_min;
    if (s > saturation_max)
        s = saturation_max;

    if (v < value_min)
        v = value_min;
    if (v > value_max)
        v = value_max;

    gtk_hsv_to_rgb(h, s, v, &r, &g, &b);

    c1->red   = r * (256 * 256 - 1);
    c1->green = g * (256 * 256 - 1);
    c1->blue  = b * (256 * 256 - 1);

    s += saturation_delta;
    v += value_delta;

    gtk_hsv_to_rgb(h, s, v, &r, &g, &b);

    c2->red   = r * (256 * 256 - 1);
    c2->green = g * (256 * 256 - 1);
    c2->blue  = b * (256 * 256 - 1);

}

GdkPixbuf * su_gdk_pixbuf_composite_thumb_icon(GdkPixbuf * thumb, GdkPixbuf * icon, int size, int icon_size)
{
    GdkPixbuf * p1 = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, size, size);
    gdk_pixbuf_fill(p1, 0x00000000);
    gulong w = gdk_pixbuf_get_width(thumb);
    gulong h = gdk_pixbuf_get_height(thumb);
    gulong x = (size - w) / 2;
    gulong y = (size - h) / 2;
    gdk_pixbuf_copy_area(thumb, 0, 0, w, h, p1, x, y);
    if (icon)
    {
        GdkPixbuf * p3 = su_gdk_pixbuf_scale_in_rect(icon, icon_size, icon_size, TRUE);
        gulong w = gdk_pixbuf_get_width(p3);
        gulong h = gdk_pixbuf_get_height(p3);
        gdk_pixbuf_composite(p3, p1,
            size - w, size - h, w, h,
            size - w, size - h, 1, 1,
            PIXBUF_INTERP_GOOD_QUALITY,
            255);
        g_object_unref(p3);
    }
    return p1;
}

/********************************************************************/

/* Try to load an icon from a named file via the freedesktop.org data directories path.
 * http://standards.freedesktop.org/basedir-spec/basedir-spec-0.6.html */
static GdkPixbuf * load_icon_file(const char * file_name, int width, int height)
{
    GdkPixbuf * icon = NULL;
    const gchar ** dirs = (const gchar **) g_get_system_data_dirs();
    const gchar ** dir;
    for (dir = dirs; ((*dir != NULL) && (icon == NULL)); dir++)
    {
        char * file_path = g_build_filename(*dir, "pixmaps", file_name, NULL);
        icon = gdk_pixbuf_new_from_file_at_size(file_path, width, height, NULL);
        g_free(file_path);
    }
    return icon;
}

/* Try to load an icon from the current theme. */
static GdkPixbuf * load_icon_from_theme(GtkIconTheme * theme, const char * icon_name, int width, int height)
{
    GdkPixbuf * icon = NULL;

    /* Look up the icon in the current theme. */
    GtkIconInfo * icon_info = NULL;
#if GLIB_CHECK_VERSION(2,20,0)
    if (icon_name && strlen(icon_name) > 7 && memcmp("GIcon ", icon_name, 6) == 0)
    {
        GIcon * gicon = g_icon_new_for_string(icon_name + 6, NULL);
        icon_info = gtk_icon_theme_lookup_by_gicon(theme, gicon, width, GTK_ICON_LOOKUP_USE_BUILTIN);
        g_object_unref(G_OBJECT(gicon));
    }
    else
#endif
    {
        icon_info = gtk_icon_theme_lookup_icon(theme, icon_name, height, GTK_ICON_LOOKUP_USE_BUILTIN);
    }

    if (icon_info != NULL)
    {
        /* If that succeeded, get the filename of the icon.
         * If that succeeds, load the icon from the specified file.
         * Otherwise, try to get the builtin icon. */
        const char * file = gtk_icon_info_get_filename(icon_info);
        if (file != NULL)
            icon = gdk_pixbuf_new_from_file(file, NULL);
        else
        {
            icon = gtk_icon_info_get_builtin_pixbuf(icon_info);
            g_object_ref(icon);
        }
        gtk_icon_info_free(icon_info);

        /* If the icon is not sized properly, take a trip through the scaler.
         * The lookup above takes the desired size, so we get the closest result possible. */
        if (icon != NULL)
        {
            if ((height != gdk_pixbuf_get_height(icon)) || (width != gdk_pixbuf_get_width(icon)))
            {
                /* Handle case of unspecified width; gdk_pixbuf_scale_simple does not. */
                if (width < 0)
                {
                    int pixbuf_width = gdk_pixbuf_get_width(icon);
                    int pixbuf_height = gdk_pixbuf_get_height(icon);
                    width = height * pixbuf_width / pixbuf_height;
                }
                GdkPixbuf * scaled = gdk_pixbuf_scale_simple(icon, width, height, GDK_INTERP_BILINEAR);
                g_object_unref(icon);
                icon = scaled;
            }
        }
    }
    return icon;
}

GdkPixbuf * su_gdk_pixbuf_load_icon(const char* name, int width, int height, gboolean use_fallback, gboolean * themed)
{
    GdkPixbuf * icon = NULL;

    if (themed)
        *themed = TRUE;

    if (name != NULL)
    {
        if (g_path_is_absolute(name))
        {
            /* Absolute path. */
            icon = gdk_pixbuf_new_from_file_at_size(name, width, height, NULL);
            if (themed)
                *themed = FALSE;
        }
        else
        {
            /* Relative path. */
            GtkIconTheme * theme = gtk_icon_theme_get_default();
            char * suffix = strrchr(name, '.');
            if ((suffix != NULL)
            && ( (g_ascii_strcasecmp(&suffix[1], "png") == 0)
              || (g_ascii_strcasecmp(&suffix[1], "svg") == 0)
              || (g_ascii_strcasecmp(&suffix[1], "xpm") == 0)))
            {
                /* The file extension indicates it could be in the system pixmap directories. */
                icon = load_icon_file(name, width, height);
                if (icon == NULL)
                {
                    /* Not found.
                     * Let's remove the suffix, and see if this name can match an icon in the current icon theme. */
                    char * icon_name = g_strndup(name, suffix - name);
                    icon = load_icon_from_theme(theme, icon_name, width, height);
                    g_free(icon_name);
                }
            }
            else
            {
                 /* No file extension.  It could be an icon name in the icon theme. */
                 icon = load_icon_from_theme(theme, name, width, height);
            }
        }
    }

    /* Fall back to generic icons. */
    if ((icon == NULL) && (use_fallback))
    {
        GtkIconTheme * theme = gtk_icon_theme_get_default();
        icon = load_icon_from_theme(theme, "application-x-executable", width, height);
        if (icon == NULL)
            icon = load_icon_from_theme(theme, "gnome-mime-application-x-executable", width, height);
        if (icon == NULL)
            icon = load_icon_from_theme(theme, GTK_STOCK_EXECUTE, width, height);
    }
    return icon;
}
