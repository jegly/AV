#!/usr/bin/env python3
"""Remove network-capable modules from the meson build.

AV is a local-only player. Most network features come out through meson
feature options (see configure-av.sh), but VLC's core access modules -
http, ftp, tcp, udp, rtp and friends - need nothing beyond libc sockets, so
they have no feature flag and are built unconditionally.

This deletes their `vlc_modules += {...}` entries outright. It is idempotent:
re-running it on an already-stripped tree reports 0 removals.

Usage: extras/av/strip_network_modules.py [--check]
  --check  report what would be removed, change nothing (exit 1 if any found)
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

# Module names to delete, per meson file.
TARGETS = {
    "modules/access/meson.build": [
        "ftp", "gopher", "http", "tcp", "udp", "amt", "satip", "cdda",
        "rtp", "rtsp", "sdp", "vnc", "rdp", "nfs", "smb2", "dsm",
        "sftp", "live555", "rist", "srt", "avio", "concat",
    ],
    "modules/access_output/meson.build": [
        "access_output_http", "access_output_livehttp", "access_output_shout",
        "access_http_put", "access_output_srt", "access_output_rist",
        "access_output_udp", "access_output_rtmp",
    ],
    "modules/services_discovery/meson.build": [
        "upnp", "microdns", "avahi", "sap", "podcast", "bonjour",
    ],
    "modules/misc/meson.build": [
        "gnutls", "securetransport", "medialibrary_network",
    ],
    # adaptive is DASH / HLS / Smooth Streaming. It links vlc_http_lib, which
    # lives in the access/http subdir we drop, so leaving it in breaks
    # configure outright rather than merely shipping dead code.
    "modules/demux/meson.build": [
        "adaptive", "dash", "hls", "smooth",
        # g64rtp demuxes RTP payloads out of local G64 surveillance files -
        # no socket, but a parser reachable straight from file input with no
        # transport left to justify it.
        "g64rtp",
    ],
    "modules/codec/meson.build": [
        "rtpvideo", "rtp_rawvid",
    ],
    # rc is the remote-control interface; it calls bind()/listen() and can be
    # told to accept connections over a socket. netsync is clock sync between
    # instances over the network.
    "modules/control/meson.build": [
        "rc", "oldrc", "netsync", "telnet", "lirc", "motion",
    ],
    # cdda calls socket()/connect() for CDDB metadata lookup, and AV has no
    # optical disc support anyway.
    }

# `subdir(...)` lines pulling in whole network subtrees.
SUBDIRS = {
    "modules/access/meson.build": ["http", "rtp", "dsm", "live555", "rist", "srt"],
    "modules/stream_out/meson.build": ["chromecast", "dlna", "hls", "rtp", "sdi"],
}


def strip_block(text, name):
    """Delete the `vlc_modules += { ... 'name' : <name> ... }` block.

    Brace-counts rather than regexing the whole block, so nested dicts and
    lists inside a module entry cannot terminate the match early.
    """
    removed = 0
    while True:
        m = re.search(r"vlc_modules \+= \{", text)
        found_at = None
        for m in re.finditer(r"vlc_modules \+= \{", text):
            start = m.start()
            i = m.end() - 1
            depth = 0
            while i < len(text):
                if text[i] == "{":
                    depth += 1
                elif text[i] == "}":
                    depth -= 1
                    if depth == 0:
                        break
                i += 1
            block = text[start:i + 1]
            if re.search(r"'name'\s*:\s*'%s'\s*," % re.escape(name), block):
                found_at = (start, i + 1)
                break
        if not found_at:
            return text, removed
        s, e = found_at
        # swallow a trailing newline and any immediately preceding comment line
        while e < len(text) and text[e] == "\n":
            e += 1
        line_start = text.rfind("\n", 0, s) + 1
        prev_line_start = text.rfind("\n", 0, line_start - 1) + 1
        prev = text[prev_line_start:line_start]
        if prev.lstrip().startswith("#"):
            s = prev_line_start
        text = text[:s] + text[e:]
        removed += 1


def main():
    check = "--check" in sys.argv
    total = 0

    for relpath, names in TARGETS.items():
        path = os.path.join(ROOT, relpath)
        if not os.path.exists(path):
            continue
        text = open(path).read()
        original = text
        hits = []
        for name in names:
            text, n = strip_block(text, name)
            if n:
                hits.append(f"{name}({n})")
                total += n
        if hits:
            print(f"  {relpath}: {' '.join(hits)}")
        if text != original and not check:
            open(path, "w").write(text)

    for relpath, subs in SUBDIRS.items():
        path = os.path.join(ROOT, relpath)
        if not os.path.exists(path):
            continue
        text = open(path).read()
        original = text
        hits = []
        for sub in subs:
            text, n = re.subn(r"[ \t]*subdir\('%s'\)\n" % re.escape(sub), "", text)
            if n:
                hits.append(f"subdir:{sub}")
                total += n
        if hits:
            print(f"  {relpath}: {' '.join(hits)}")
        if text != original and not check:
            open(path, "w").write(text)

    if total == 0:
        print("  nothing to remove (already stripped)")
    print(f"\n  {'would remove' if check else 'removed'}: {total} entries")
    return 1 if (check and total) else 0


sys.exit(main())
