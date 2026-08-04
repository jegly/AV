# Access output plugins

# Dummy
vlc_modules += {
    'name' : 'access_output_dummy',
    'sources' : files('dummy.c'),
    'shortname' : 'a_o_dumm',
}

# File
vlc_modules += {
    'name' : 'access_output_file',
    'sources' : files('file.c'),
    'shortname' : 'a_o_file',
}

# liveHTTP
gpg_error_dep = cc.find_library('gpg-error', dirs: contrib_libdir, required: false)
# Shout (Icecast and old Shoutcast)
shout_dep = dependency('shout', version: '>= 2.1', required: get_option('shout'))
if shout_dep.found()
    endif

# SRT
if srt_dep.found()
    endif

# RIST
if librist_dep.found()
    endif
