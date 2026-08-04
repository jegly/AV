/*****************************************************************************
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
    /* ---- Catppuccin ---- */
    { "catppuccin_latte", "Catppuccin Latte", false, 0xFFEFF1F5, 0xFFE6E9EF, 0xFFDCE0E8, 0xFFCCD0DA, 0xFF4C4F69, 0xFF5C5F77, 0xFF1E66F5, 0xFF8839EF, 0xFF179299, 0xFFD20F39, 0xFF40A02B, 0xFFDF8E1D },
    { "catppuccin_frappe", "Catppuccin Frappe", true, 0xFF303446, 0xFF292C3C, 0xFF232634, 0xFF414559, 0xFFC6D0F5, 0xFFB5BFE2, 0xFF8CAAEE, 0xFFCA9EE6, 0xFF81C8BE, 0xFFE78284, 0xFFA6D189, 0xFFE5C890 },
    { "catppuccin_macchiato", "Catppuccin Macchiato", true, 0xFF24273A, 0xFF1E2030, 0xFF181926, 0xFF363A4F, 0xFFCAD3F5, 0xFFB8C0E0, 0xFF8AADF4, 0xFFC6A0F6, 0xFF8BD5CA, 0xFFED8796, 0xFFA6DA95, 0xFFEED49F },
    { "catppuccin_mocha", "Catppuccin Mocha", true, 0xFF1E1E2E, 0xFF181825, 0xFF11111B, 0xFF313244, 0xFFCDD6F4, 0xFFBAC2DE, 0xFF89B4FA, 0xFFCBA6F7, 0xFF94E2D5, 0xFFF38BA8, 0xFFA6E3A1, 0xFFF9E2AF },

    /* ---- Dracula ---- */
    { "dracula", "Dracula", true, 0xFF282A36, 0xFF21222C, 0xFF191A21, 0xFF383A48, 0xFFF8F8F2, 0xFFB8BAC8, 0xFFBD93F9, 0xFFFF79C6, 0xFF8BE9FD, 0xFFFF5555, 0xFF50FA7B, 0xFFF1FA8C },

    /* ---- Ptyxis (curated) ---- */
    { "ptyxis_fairy_floss", "Fairy Floss", true, 0xFF5A5475, 0xFF403C54, 0xFF302D3F, 0xFF655F7E, 0xFFC2FFDF, 0xFF9DC3B9, 0xFFFFB8D1, 0xFFAE81FF, 0xFFC2FFDF, 0xFFFF857F, 0xFFC2FFDF, 0xFFAE81FF },
    { "ptyxis_nord", "Nord", true, 0xFF2E3440, 0xFF21252E, 0xFF181C22, 0xFF3C424D, 0xFFD8DEE9, 0xFF9CA2AD, 0xFF88C0D0, 0xFF81A1C1, 0xFF8FBCBB, 0xFFBF616A, 0xFF8FBCBB, 0xFF81A1C1 },
    { "ptyxis_bim", "Bim", true, 0xFF012849, 0xFF001C34, 0xFF001527, 0xFF123755, 0xFFA9BED8, 0xFF6E89A5, 0xFF5EA2EC, 0xFFF557A0, 0xFFA9EE55, 0xFFF557A0, 0xFFA9EE55, 0xFFF557A0 },
    { "ptyxis_borland", "Borland", true, 0xFF0000A4, 0xFF000076, 0xFF000058, 0xFF1111AA, 0xFFFFFF4E, 0xFFA5A56C, 0xFFFFFF4E, 0xFFFF73FD, 0xFF96CBFE, 0xFFFF6C60, 0xFF96CBFE, 0xFFFF73FD },
    { "ptyxis_c64", "C64", true, 0xFF40318D, 0xFF2E2365, 0xFF221A4C, 0xFF4D3F94, 0xFF7869C4, 0xFF6455B0, 0xFF67B6BD, 0xFFBFCE72, 0xFF8B3F96, 0xFF883932, 0xFF8B3F96, 0xFFBFCE72 },
    { "ptyxis_cobalt_neon", "Cobalt Neon", true, 0xFF142838, 0xFF0E1C28, 0xFF0A151E, 0xFF243745, 0xFF8FF586, 0xFF63AD6A, 0xFF8FF586, 0xFF3BA5FF, 0xFFE9E75C, 0xFFFF2320, 0xFFE9E75C, 0xFF3BA5FF },
    { "ptyxis_grass", "Grass", true, 0xFF13773D, 0xFF0D552B, 0xFF0A4020, 0xFF23804A, 0xFFFFF0A5, 0xFFACC580, 0xFFE7B000, 0xFF00BBBB, 0xFFFFF0A5, 0xFFBB0000, 0xFFFFF0A5, 0xFF00BBBB },
    { "ptyxis_homebrew_ocean", "Homebrew Ocean", true, 0xFF224FBC, 0xFF183887, 0xFF122A65, 0xFF315BC0, 0xFFFFFFFF, 0xFFB1C1E7, 0xFF00A6B2, 0xFF00A600, 0xFF999900, 0xFF990000, 0xFF999900, 0xFF00A600 },
    { "ptyxis_mono_amber", "Mono Amber", true, 0xFF2B1900, 0xFF1E1200, 0xFF170D00, 0xFF392911, 0xFFFF9400, 0xFFB46800, 0xFFFF9400, 0xFFFF9400, 0xFFFF9400, 0xFFFF9400, 0xFFFF9400, 0xFFFF9400 },
    { "ptyxis_mono_red", "Mono Red", true, 0xFF2B0C00, 0xFF1E0800, 0xFF170600, 0xFF391D11, 0xFFFF3600, 0xFFB42700, 0xFFFF3600, 0xFFFF3600, 0xFFFF3600, 0xFFFF3600, 0xFFFF3600, 0xFFFF3600 },
    { "ptyxis_synthwave", "Synthwave", true, 0xFF262335, 0xFF1B1926, 0xFF14121C, 0xFF353243, 0xFFFFFFFF, 0xFFB3B2B8, 0xFFFF7EDB, 0xFF03EDF9, 0xFFFEDE5D, 0xFFFE4450, 0xFFFEDE5D, 0xFF03EDF9 },

    /* ---- Ptyxis (extended) ---- */
    { "ptyxis_aci", "Aci", true, 0xFF0D1926, 0xFF09121B, 0xFF070D14, 0xFF1D2935, 0xFFB4E1FD, 0xFF799BB1, 0xFF1E8EFF, 0xFF8E1EFF, 0xFF1EFF8E, 0xFFFF1E8E, 0xFF8EFF1E, 0xFFFF8E1E },
    { "ptyxis_afterglow", "Afterglow", true, 0xFF222222, 0xFF181818, 0xFF121212, 0xFF313131, 0xFFD0D0D0, 0xFF939393, 0xFF547C99, 0xFF9F4E85, 0xFF7DD6CF, 0xFFA53C23, 0xFF7B9246, 0xFFD3A04D },
    { "ptyxis_argonaut", "Argonaut", true, 0xFF0E1019, 0xFF0A0B12, 0xFF07080D, 0xFF1E2029, 0xFFFFFAF4, 0xFFAAA8A7, 0xFF0092FF, 0xFF9A5FEB, 0xFF67FFF0, 0xFFFF2740, 0xFFABE15B, 0xFFFFD242 },
    { "ptyxis_aura", "Aura", true, 0xFF15141B, 0xFF0F0E13, 0xFF0B0A0E, 0xFF25242A, 0xFFEDECEE, 0xFFA1A0A4, 0xFFA277FF, 0xFFA277FF, 0xFF61FFCA, 0xFFFFCA85, 0xFFA277FF, 0xFFFFCA85 },
    { "ptyxis_ayu_mirage", "Ayu Mirage", true, 0xFF1F2430, 0xFF161922, 0xFF101319, 0xFF2E333E, 0xFFCBCCC6, 0xFF8E9191, 0xFF73D0FF, 0xFFD4BFFF, 0xFF95E6CB, 0xFFFF3333, 0xFFBAE67E, 0xFFFFA759 },
    { "ptyxis_belafonte", "Belafonte", true, 0xFF20111B, 0xFF170C13, 0xFF11090E, 0xFF2F212A, 0xFF968C83, 0xFF6C605E, 0xFF426A79, 0xFF97522C, 0xFF989A9C, 0xFFBE100E, 0xFF858162, 0xFFEAA549 },
    { "ptyxis_birds_of_paradise", "Birds Of Paradise", true, 0xFF2A1F1D, 0xFF1E1614, 0xFF16100F, 0xFF382E2C, 0xFFE0DBB7, 0xFFA09981, 0xFFB8D3ED, 0xFFD19ECB, 0xFF93CFD7, 0xFFE84627, 0xFF95D8BA, 0xFFD0D150 },
    { "ptyxis_blazer", "Blazer", true, 0xFF0D1926, 0xFF09121B, 0xFF070D14, 0xFF1D2935, 0xFFD9E6F2, 0xFF919EAA, 0xFFBDBDDB, 0xFFDBBDDB, 0xFFBDDBDB, 0xFFDBBDBD, 0xFFBDDBBD, 0xFFDBDBBD },
    { "ptyxis_brogrammer", "Brogrammer", true, 0xFF131313, 0xFF0D0D0D, 0xFF0A0A0A, 0xFF232323, 0xFFD6DBE5, 0xFF91959B, 0xFF1081D6, 0xFF5350B9, 0xFF0F7DDB, 0xFFDE352E, 0xFF1DD361, 0xFFF3BD09 },
    { "ptyxis_chalkboard", "Chalkboard", true, 0xFF29262F, 0xFF1D1B21, 0xFF161419, 0xFF37353D, 0xFFD9E6F2, 0xFF9BA2AD, 0xFFAAAADB, 0xFFDBAADA, 0xFFAADADB, 0xFFDBAAAA, 0xFFAADBAA, 0xFFDADBAA },
    { "ptyxis_espresso_libre", "Espresso Libre", true, 0xFF2A211C, 0xFF1E1714, 0xFF16110F, 0xFF38302B, 0xFFB8A898, 0xFF86786C, 0xFF43A8ED, 0xFFFF818A, 0xFF34E2E2, 0xFFEF2929, 0xFF9AFF87, 0xFFFFFB5C },
    { "ptyxis_everforest", "Everforest", true, 0xFF2D353B, 0xFF20262A, 0xFF181C1F, 0xFF3B4348, 0xFFD3C6AA, 0xFF989383, 0xFF3A94C5, 0xFFDF69BA, 0xFF35A77C, 0xFFF85552, 0xFF8DA101, 0xFFDFA000 },
    { "ptyxis_flatland", "Flatland", true, 0xFF1D1F21, 0xFF141617, 0xFF0F1011, 0xFF2C2E30, 0xFFB8DBEF, 0xFF8199A6, 0xFF61B9D0, 0xFF695ABC, 0xFFD63865, 0xFFD22A24, 0xFFA7D42C, 0xFFFF8949 },
    { "ptyxis_github", "Github", true, 0xFF101216, 0xFF0B0C0F, 0xFF08090B, 0xFF202226, 0xFF8B949E, 0xFF5F666E, 0xFF6CA4F8, 0xFFDB61A2, 0xFF2B7489, 0xFFF78166, 0xFF56D364, 0xFFE3B341 },
    { "ptyxis_ibm3270", "Ibm3270", true, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF111111, 0xFFFDFDFD, 0xFFA4A4A4, 0xFFB3BFEF, 0xFFEFB3E3, 0xFF9CE2E2, 0xFFEF8383, 0xFF7ED684, 0xFFEFE28B },
    { "ptyxis_ic_green_ppl", "Ic Green Ppl", true, 0xFF3A3D3F, 0xFF292B2D, 0xFF1F2022, 0xFF474A4C, 0xFFD9EFD3, 0xFFA1B09F, 0xFF72FFB5, 0xFF50FF3E, 0xFF22FF71, 0xFFA7FF3F, 0xFF9FFF6D, 0xFFD2FF6D },
    { "ptyxis_kanagawa", "Kanagawa", true, 0xFF1F1F28, 0xFF16161C, 0xFF101015, 0xFF2E2E37, 0xFFDCD7BA, 0xFF999686, 0xFF7FB4CA, 0xFF938AA9, 0xFF7AA89F, 0xFFE82424, 0xFF98BB6C, 0xFFE6C384 },
    { "ptyxis_material", "Material", true, 0xFF1E282C, 0xFF151C1F, 0xFF101517, 0xFF2D373A, 0xFFC3C7D1, 0xFF898F97, 0xFF7DC6BF, 0xFF6C71C3, 0xFF34434D, 0xFFEB606B, 0xFFC3E88D, 0xFFF7EB95 },
    { "ptyxis_mona_lisa", "Mona Lisa", true, 0xFF120B0D, 0xFF0C0709, 0xFF090507, 0xFF221C1D, 0xFFF7D66A, 0xFFA68E49, 0xFF9EB2B4, 0xFFFF5B6A, 0xFF8ACD8F, 0xFFFF4331, 0xFFB4B264, 0xFFFF9566 },
    { "ptyxis_mono_cyan", "Mono Cyan", true, 0xFF00222B, 0xFF00181E, 0xFF001217, 0xFF113139, 0xFF00CCFF, 0xFF0090B4, 0xFF00CCFF, 0xFF00CCFF, 0xFF00CCFF, 0xFF00CCFF, 0xFF00CCFF, 0xFF00CCFF },
    { "ptyxis_monokai_pro", "Monokai Pro", true, 0xFF363537, 0xFF262627, 0xFF1D1C1D, 0xFF444345, 0xFFFDF9F3, 0xFFB7B4B1, 0xFFFC9867, 0xFFAB9DF2, 0xFF78DCE8, 0xFFFF6188, 0xFFA9DC76, 0xFFFFD866 },
    { "ptyxis_omni", "Omni", true, 0xFF191622, 0xFF120F18, 0xFF0D0B12, 0xFF292631, 0xFFABB2BF, 0xFF777B88, 0xFF78D1E1, 0xFF988BC7, 0xFFFF79C6, 0xFFE96379, 0xFF67E480, 0xFFE89E64 },
    { "ptyxis_paraiso_dark", "Paraiso Dark", true, 0xFF2F1E2E, 0xFF211521, 0xFF191018, 0xFF3D2D3C, 0xFFA39E9B, 0xFF7A7174, 0xFF06B6EF, 0xFF815BA4, 0xFF5BC4BF, 0xFFEF6155, 0xFF48B685, 0xFFFEC418 },
    { "ptyxis_pixiefloss", "Pixiefloss", true, 0xFF241F33, 0xFF191624, 0xFF13101B, 0xFF332E41, 0xFFD1CAE8, 0xFF948EA8, 0xFFC5A3FF, 0xFFEF6155, 0xFFC2FFFF, 0xFFF1568E, 0xFF5ADBA2, 0xFFD5A425 },
    { "ptyxis_powershell", "Powershell", true, 0xFF052454, 0xFF03193C, 0xFF02132D, 0xFF16335F, 0xFFF6F6F7, 0xFFA1ACBD, 0xFF268AD2, 0xFFFE13FA, 0xFF29FFFE, 0xFFEF2929, 0xFF1CFE3C, 0xFFFEFE45 },
    { "ptyxis_relaxed", "Relaxed", true, 0xFF353A44, 0xFF262930, 0xFF1C1F24, 0xFF434751, 0xFFD9D9D9, 0xFF9FA1A4, 0xFF7EAAC7, 0xFFB06698, 0xFFACBBD0, 0xFFBC5653, 0xFFA0AC77, 0xFFEBC17A },
    { "ptyxis_sea_shells", "Sea Shells", true, 0xFF09141B, 0xFF060E13, 0xFF040A0E, 0xFF1A242A, 0xFFDEB88D, 0xFF937E65, 0xFF1BBCDD, 0xFFBBE3EE, 0xFF87ACB4, 0xFFD48678, 0xFF628D98, 0xFFFDD39F },
    { "ptyxis_solarized", "Solarized", true, 0xFF002B36, 0xFF001E26, 0xFF00171D, 0xFF113944, 0xFF839496, 0xFF556F74, 0xFF2699FF, 0xFFD33682, 0xFF43B8C3, 0xFFD87979, 0xFF88CF76, 0xFF657B83 },
    { "ptyxis_spacedust", "Spacedust", true, 0xFF0A1E24, 0xFF071519, 0xFF051013, 0xFF1B2D33, 0xFFECF0C1, 0xFF9CA68A, 0xFF67A0CE, 0xFFFF8A3A, 0xFF83A7B4, 0xFFFF8A3A, 0xFFAECAB8, 0xFFFFC878 },
    { "ptyxis_spring", "Spring", true, 0xFF0A1E24, 0xFF071519, 0xFF051013, 0xFF1B2D33, 0xFFECF0C1, 0xFF9CA68A, 0xFF15A9FD, 0xFF8959A8, 0xFF3E999F, 0xFFFF0021, 0xFF1FC231, 0xFFD5B807 },
    { "ptyxis_twilight", "Twilight", true, 0xFF141414, 0xFF0E0E0E, 0xFF0A0A0A, 0xFF242424, 0xFFFFFFD4, 0xFFACAC90, 0xFF5A5E62, 0xFFD0DC8E, 0xFF8A989B, 0xFFDE7C4C, 0xFFCCD88C, 0xFFE2C47E },
    { "ptyxis_urple", "Urple", true, 0xFF1B1B23, 0xFF131319, 0xFF0E0E12, 0xFF2A2A32, 0xFF877A9B, 0xFF615871, 0xFF867AED, 0xFFA05EEE, 0xFFEAEAEA, 0xFFFF6388, 0xFF29E620, 0xFFF08161 },
    { "ptyxis_xterm", "XTerm", true, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF111111, 0xFFFFFFFF, 0xFFA5A5A5, 0xFF5C5CFF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00 },
};

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
        if (*a == '\0' && *b == '\0')
            return i;
    }
    return -1;
}

/* Defaults referenced from the interface. */
#define MEDEA_DEFAULT       "ptyxis_nord"
#define MEDEA_DEFAULT_DARK  "ptyxis_nord"
#define MEDEA_DEFAULT_LIGHT "catppuccin_latte"

#endif // VLC_MEDEAPALETTES_HPP
