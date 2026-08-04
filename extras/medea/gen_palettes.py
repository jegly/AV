#!/usr/bin/env python3
"""Generate medeapalettes.hpp from the Android theme-port sources.

Four input formats, one output table:
  * ptyxis-palettes/*.palette  - GNOME keyfile, full 16-slot ANSI table (33)
  * kotlin-theme/Ptyxis.kt     - curated enum, hand-picked accents        (11)
  * kotlin-theme/Catppuccin.kt - BASE_PALETTE + accent PALETTE            (4)
  * kotlin-theme/Dracula.kt    - DARK_BASE + DARK_ACCENT                  (1)

The .palette files are the ground truth for the extended set; the Kotlin is the
only source for the 11 curated Ptyxis entries and for Catppuccin/Dracula, whose
base/mantle/crust/surface layers are authored rather than derived.
"""
import os
import re
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))

# The palette sources live outside the tree (they were extracted from an Android
# app), so allow an override; default to a sibling of the checkout.
SRC = os.environ.get(
    "MEDEA_THEME_SRC",
    os.path.join(os.path.dirname(_ROOT), "theme port for android apps"))
OUT = os.environ.get(
    "MEDEA_PALETTE_OUT",
    os.path.join(_ROOT, "modules", "gui", "qt", "style", "medeapalettes.hpp"))


# ---------------------------------------------------------------- colour utils

def rgb(v):
    return ((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF)


def pack(r, g, b):
    return (int(r) << 16) | (int(g) << 8) | int(b)


def lum(v):
    """Relative luminance, sRGB-weighted. Decides light vs dark themes."""
    r, g, b = rgb(v)
    return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0


def mix(a, b, t):
    ar, ag, ab = rgb(a)
    br, bg, bb = rgb(b)
    return pack(ar + (br - ar) * t, ag + (bg - ag) * t, ab + (bb - ab) * t)


def shade(base, t, dark):
    """Step a base colour toward black (dark themes) or white (light themes)."""
    return mix(base, 0x000000 if dark else 0xFFFFFF, t)


def derive_layers(base, dark):
    """Glass-mode layer tiers when a palette doesn't author them itself.

    GLASS_MODE_INTEGRATION.md wants BASE > MANTLE > CRUST getting progressively
    darker, with SURFACE lifted off the body for cards/popovers.
    """
    if dark:
        return dict(mantle=shade(base, 0.28, True),
                    crust=shade(base, 0.46, True),
                    surface=mix(base, 0xFFFFFF, 0.07))
    return dict(mantle=shade(base, 0.22, False),
                crust=shade(base, 0.36, False),
                surface=mix(base, 0x000000, 0.04))


def parse_hex(s):
    s = s.strip().lstrip("#")
    if s[:2].lower() == "0x":  # Kotlin literals arrive as 0xAARRGGBB
        s = s[2:]
    if len(s) == 8:            # drop the alpha byte, we only store RGB
        s = s[2:]
    return int(s, 16)


# ------------------------------------------------------------------- 33 files

def load_palette_files():
    d = os.path.join(SRC, "ptyxis-palettes")
    out = []
    for fn in sorted(os.listdir(d)):
        if not fn.endswith(".palette"):
            continue
        kv = {}
        for line in open(os.path.join(d, fn), encoding="utf-8"):
            line = line.strip()
            if "=" in line and not line.startswith("#"):
                k, _, v = line.partition("=")
                kv[k.strip()] = v.strip()
        name = kv.get("Name") or os.path.splitext(fn)[0]
        base = parse_hex(kv["Background"])
        text = parse_hex(kv["Foreground"])
        ansi = [parse_hex(kv[f"Color{i}"]) for i in range(16)]
        dark = lum(base) < 0.5

        # Standard ANSI role convention, per the port's MANIFEST: slot 4 blue ->
        # primary, 5 magenta -> secondary, 6 cyan -> tertiary, 1 red -> error,
        # 2 green -> positive, 3 yellow -> neutral. Bright variants (8..15) read
        # better as accents on dark backgrounds.
        hi = 8 if dark else 0
        out.append(dict(
            key="ptyxis_" + re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_"),
            name=name, dark=dark, base=base, text=text,
            subtext=mix(text, base, 0.35),
            primary=ansi[hi + 4], secondary=ansi[hi + 5], tertiary=ansi[hi + 6],
            negative=ansi[hi + 1], positive=ansi[hi + 2], neutral=ansi[hi + 3],
            **derive_layers(base, dark)))
    return out


# --------------------------------------------------------- 11 curated (Kotlin)

def load_curated_ptyxis():
    txt = open(os.path.join(SRC, "kotlin-theme", "Ptyxis.kt"), encoding="utf-8").read()
    body = txt[txt.index("enum class PtyxisPalette"):]
    rx = re.compile(
        r'^\s*[A-Z0-9_]+\(\s*"([a-z0-9_]+)"\s*,\s*"([^"]+)"\s*,'
        r'\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*,'
        r'\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*,\s*(0x[0-9A-Fa-f]{8})\s*\)',
        re.M)
    out = []
    for key, name, bg, fg, pri, sec, ter, err in rx.findall(body):
        base, text = parse_hex(bg), parse_hex(fg)
        primary, secondary, tertiary = map(parse_hex, (pri, sec, ter))
        dark = lum(base) < 0.5
        out.append(dict(
            key="ptyxis_" + key, name=name, dark=dark, base=base, text=text,
            subtext=mix(text, base, 0.35),
            primary=primary, secondary=secondary, tertiary=tertiary,
            negative=parse_hex(err),
            # This enum carries no explicit positive/neutral; reuse the palette's
            # own tertiary/secondary rather than inventing a hue that isn't in it.
            positive=tertiary, neutral=secondary,
            **derive_layers(base, dark)))
    return out


# ------------------------------------------------------- Catppuccin & Dracula

def kt_fields(block):
    """Pull `name = Color(0xAARRGGBB)` pairs out of a Kotlin data-class literal."""
    return {k: parse_hex(v) for k, v in
            re.findall(r"(\w+)\s*=\s*Color\((0x[0-9A-Fa-f]{8})\)", block)}


def kt_map(block):
    """Pull `Enum.KEY to Color(0xAARRGGBB)` pairs out of a Kotlin map literal."""
    return {k.lower(): parse_hex(v) for k, v in
            re.findall(r"\.(\w+)\s+to\s+Color\((0x[0-9A-Fa-f]{8})\)", block)}


def build(key, name, base_c, accents):
    """Assemble an entry from authored base layers + a named accent map."""
    dark = lum(base_c["base"]) < 0.5

    def pick(*names):
        for n in names:
            if n in accents:
                return accents[n]
        return base_c["text"]

    return dict(
        key=key, name=name, dark=dark,
        base=base_c["base"], mantle=base_c["mantle"], crust=base_c["crust"],
        surface=base_c.get("surface0", base_c["base"]),
        text=base_c["text"], subtext=base_c.get("subtext1", base_c["text"]),
        primary=pick("blue", "purple", "mauve"),
        secondary=pick("mauve", "pink", "purple"),
        tertiary=pick("teal", "cyan", "sky"),
        negative=pick("red"), positive=pick("green"), neutral=pick("yellow", "peach", "orange"))


def load_catppuccin():
    txt = open(os.path.join(SRC, "kotlin-theme", "Catppuccin.kt"), encoding="utf-8").read()
    accent_blk = txt[txt.index("private val PALETTE"):txt.index("private val BASE_PALETTE")]
    base_blk = txt[txt.index("private val BASE_PALETTE"):]
    out = []
    for flavor in ("LATTE", "FRAPPE", "MACCHIATO", "MOCHA"):
        a = re.search(r"CatppuccinFlavor\.%s to mapOf\((.*?)\n  \)" % flavor, accent_blk, re.S)
        b = re.search(r"CatppuccinFlavor\.%s to CatppuccinBaseColors\((.*?)\n  \)" % flavor, base_blk, re.S)
        if not (a and b):
            print(f"  WARN: catppuccin {flavor} not matched", file=sys.stderr)
            continue
        out.append(build("catppuccin_" + flavor.lower(),
                         "Catppuccin " + flavor.title(),
                         kt_fields(b.group(1)), kt_map(a.group(1))))
    return out


def load_dracula():
    txt = open(os.path.join(SRC, "kotlin-theme", "Dracula.kt"), encoding="utf-8").read()
    acc = re.search(r"private val DARK_ACCENT[^=]*=\s*mapOf\((.*?)\n\)", txt, re.S)
    bas = re.search(r"private val DARK_BASE\s*=\s*DraculaBaseColors\((.*?)\n\)", txt, re.S)
    if not (acc and bas):
        print("  WARN: dracula not matched", file=sys.stderr)
        return []
    return [build("dracula", "Dracula", kt_fields(bas.group(1)), kt_map(acc.group(1)))]


# ------------------------------------------------------------------- emission

FIELDS = ["base", "mantle", "crust", "surface", "text", "subtext",
          "primary", "secondary", "tertiary", "negative", "positive", "neutral"]

HEADER = """/*****************************************************************************
 * medeapalettes.hpp : Medea colour palettes
 *****************************************************************************
 * Copyright (C) 2026 Medea authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/* GENERATED FILE - DO NOT EDIT BY HAND.
 * Regenerate with extras/medea/gen_palettes.py, which reads the palette sources in
 * "theme port for android apps". Edit the sources or the generator instead.
 *
 * Layer names follow GLASS_MODE_INTEGRATION.md: base is the window body,
 * mantle the sidebars, crust the headerbar (the "glass edge"), surface the
 * cards and popovers. Glass mode applies its alpha tiers to these four.
 */

#ifndef VLC_MEDEAPALETTES_HPP
#define VLC_MEDEAPALETTES_HPP

#include <QtGui/qrgb.h>

struct MedeaPalette
{
    const char* key;
    const char* displayName;
    bool isDark;

    /* window layers, light -> dark */
    QRgb base;
    QRgb mantle;
    QRgb crust;
    QRgb surface;

    /* text */
    QRgb text;
    QRgb subtext;

    /* semantic accents */
    QRgb primary;
    QRgb secondary;
    QRgb tertiary;
    QRgb negative;
    QRgb positive;
    QRgb neutral;
};

static const MedeaPalette medea_palettes[] =
{
"""

FOOTER = """};

static const int medea_palettes_count =
    (int)(sizeof(medea_palettes) / sizeof(medea_palettes[0]));

/* Look a palette up by its stable key. Call sites should use this rather than
 * hardcoding an index, so that reordering or adding palettes cannot silently
 * repoint them at the wrong theme. Returns -1 when the key is unknown. */
static inline int medea_palette_index(const char* key)
{
    if (!key)
        return -1;
    for (int i = 0; i < medea_palettes_count; ++i)
    {
        const char* a = medea_palettes[i].key;
        const char* b = key;
        while (*a && *a == *b) { ++a; ++b; }
        if (*a == '\\0' && *b == '\\0')
            return i;
    }
    return -1;
}

/* Defaults referenced from the interface. */
#define MEDEA_DEFAULT       "ptyxis_nord"
#define MEDEA_DEFAULT_DARK  "ptyxis_nord"
#define MEDEA_DEFAULT_LIGHT "catppuccin_latte"

#endif // VLC_MEDEAPALETTES_HPP
"""


def main():
    groups = [("Catppuccin", load_catppuccin()),
              ("Dracula", load_dracula()),
              ("Ptyxis (curated)", load_curated_ptyxis()),
              ("Ptyxis (extended)", load_palette_files())]

    seen, rows, lines = set(), 0, []
    for label, entries in groups:
        lines.append(f"    /* ---- {label} ---- */")
        for e in entries:
            if e["key"] in seen:
                print(f"  WARN: duplicate key {e['key']}, skipped", file=sys.stderr)
                continue
            seen.add(e["key"])
            cols = ", ".join("0xFF%06X" % e[f] for f in FIELDS)
            lines.append('    { "%s", "%s", %s, %s },'
                         % (e["key"], e["name"], "true" if e["dark"] else "false", cols))
            rows += 1
        lines.append("")
        print(f"  {label:20s} {len(entries):3d}")

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as f:
        f.write(HEADER + "\n".join(lines) + FOOTER)
    print(f"\n  total {rows} palettes -> {OUT}")


main()
