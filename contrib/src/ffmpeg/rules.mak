# FFmpeg

FFMPEG_HASH=ec47a3b95f88fc3f820b900038ac439e4eb3fede
FFMPEG_MAJVERSION := 8.1
FFMPEG_REVISION := 2
FFMPEG_VERSION := $(FFMPEG_MAJVERSION).$(FFMPEG_REVISION)
# FFMPEG_VERSION := $(FFMPEG_MAJVERSION)
FFMPEG_BRANCH=release/$(FFMPEG_MAJVERSION)
FFMPEG_URL := https://ffmpeg.org/releases/ffmpeg-$(FFMPEG_VERSION).tar.xz
FFMPEG_GITURL := https://code.ffmpeg.org/FFmpeg/FFmpeg.git
FFMPEG_LAVC_MIN := 57.37.100

FFMPEG_BASENAME := $(subst .,_,$(subst \,_,$(subst /,_,$(FFMPEG_HASH))))

# bsf=vp9_superframe is needed to mux VP9 inside webm/mkv
FFMPEGCONF = --prefix="$(PREFIX)" --enable-static --disable-shared \
	--extra-ldflags="$(LDFLAGS)" \
	--cc="$(CC)" \
	--host-cc="$(BUILDCC)" \
	--pkg-config="$(PKG_CONFIG)" \
	--disable-doc \
	--disable-encoder=vorbis \
	--disable-decoder=opus \
	--enable-libgsm \
	--enable-libopenjpeg \
	--disable-debug \
	--disable-avdevice \
	--disable-devices \
	--disable-avfilter \
	--disable-filters \
	--disable-protocol=concat \
	--disable-bsfs \
	--disable-bzlib \
	--disable-libvpx \
	--enable-bsf=vp9_superframe \
	--disable-swresample \
	--disable-iconv \
	--disable-avisynth \
	--disable-nvenc \
	--disable-linux-perf
ifdef HAVE_DARWIN_OS
FFMPEGCONF += \
	--disable-securetransport
endif

ifdef ENABLE_PDB
FFMPEGCONF += --ln_s=false
endif
ifdef ENABLE_LTO
FFMPEGCONF += --enable-lto
endif

DEPS_ffmpeg = zlib $(DEPS_zlib) gsm $(DEPS_gsm) openjpeg $(DEPS_openjpeg)

# Optional dependencies
ifndef BUILD_NETWORK
FFMPEGCONF += --disable-network
endif
ifdef BUILD_ENCODERS
FFMPEGCONF += --enable-libmp3lame
DEPS_ffmpeg += lame $(DEPS_lame)
else
FFMPEGCONF += --disable-encoders --disable-muxers
endif

ifneq ($(findstring amf,$(PKGS)),)
DEPS_ffmpeg += amf $(DEPS_amf)
endif

# Small size
ifdef WITH_OPTIMIZATION
ifdef ENABLE_SMALL
FFMPEGCONF += --enable-small
endif
ifeq ($(ARCH),arm)
ifdef HAVE_ARMV7A
FFMPEGCONF += --enable-thumb
endif
endif
else
FFMPEGCONF += --optflags=-Og
endif

ifdef HAVE_CROSS_COMPILE
FFMPEGCONF += --enable-cross-compile --disable-programs
ifndef HAVE_DARWIN_OS
FFMPEGCONF += --cross-prefix=$(HOST)-
endif
endif

# ARM stuff
ifeq ($(ARCH),arm)
FFMPEGCONF += --arch=arm
ifdef HAVE_ARMV7A
FFMPEGCONF += --cpu=cortex-a8
endif
ifdef HAVE_ARMV6
FFMPEGCONF += --cpu=armv6 --disable-neon
endif
endif

# ARM64 stuff
ifeq ($(ARCH),aarch64)
FFMPEGCONF += --arch=aarch64
endif

# MIPS stuff
ifeq ($(ARCH),mipsel)
FFMPEGCONF += --arch=mips
endif
ifeq ($(ARCH),mips64el)
FFMPEGCONF += --arch=mips64
endif

# RISC-V stuff
ifneq ($(findstring $(ARCH),riscv32 riscv64),)
FFMPEGCONF += --arch=riscv
endif

# x86 stuff
ifeq ($(ARCH),i386)
ifndef HAVE_DARWIN_OS
FFMPEGCONF += --arch=x86
endif
endif

# x86_64 stuff
ifeq ($(ARCH),x86_64)
ifndef HAVE_DARWIN_OS
FFMPEGCONF += --arch=x86_64
endif
endif

# Darwin
ifdef HAVE_DARWIN_OS
ifeq ($(ARCH),arm64_32)
# TODO remove when FFMpeg supports arm64_32
FFMPEGCONF += --arch=aarch64_32
else
FFMPEGCONF += --arch=$(ARCH)
endif
FFMPEGCONF += --target-os=darwin --extra-cflags="$(CFLAGS)"
FFMPEGCONF += --disable-lzma
ifeq ($(ARCH),x86_64)
FFMPEGCONF += --cpu=core2
endif
ifdef HAVE_IOS
FFMPEGCONF += --enable-pic --extra-ldflags="$(EXTRA_CFLAGS) -isysroot $(IOS_SDK)"
ifdef HAVE_WATCHOS
FFMPEGCONF += --disable-everything
FFMPEGCONF += --enable-decoder='aac,aac_latm,aac_fixed,aadec,ac3,adpcm_*,aiff,alac,alsdec,amrnb,amrwb,ape,atrac1,atrac3,atrac3plus,atrac9,binkaudio_dct,binkaudio_rdft,bmv_audio,cook,dca,derf,dirac,dpcm,dts,dvaudio,eaac,eac3,flac,flv,g722,g723,g726,g729,gsm,metasound,mpc7,mpc8,mpegaudiodec_fixed,mp3,m4a,nellymoser,opus,pcm_*,qdmc,qdm2,ra144,ra288,ralf,rka,shorten,tta,tak,truespeech,vorbis,wavpack,wma,wmalossless,wmapro,wmavoice'
FFMPEGCONF += --enable-parser='aac,aac_latm,ac3,adpcm,amr,aac_latm,ape,cook,dca,dvaudio,flac,g723,g729,gsm,mlp,mpegaudio,opus,sipr,vorbis,xma'
FFMPEGCONF += --enable-demuxer='aac,ac3,adts,aiff,ape,asf,au,avi,caf,daud,dirac,dts,dv,ea,flac,flv,gsm,ivf,matroska,mmf,mov,mp3,mpeg,ogg,pcm,rm,sbc,sdp,shorten,voc,w64,wav,wv'
FFMPEGCONF += --enable-swresample
endif
endif
endif

# Linux
ifdef HAVE_LINUX
FFMPEGCONF += --target-os=linux --enable-pic
# ffmpeg keeps HAVE_INLINE_ASM_DIRECT_SYMBOL_REFS on, so inline asm such as
# libavcodec/x86/vc1dsp_mmx.c refers to constants like ff_pw_9 (declared in C,
# in constants.c) by direct symbol reference. Those symbols default to GLOBAL
# DEFAULT visibility, which the linker must treat as preemptible, so the
# resulting static lib cannot go into one of VLC's shared plugins:
#   relocation R_X86_64_PC32 against symbol `ff_pw_9' can not be used when
#   making a shared object
# Hidden visibility makes the references non-preemptible and the PC32
# relocations valid. Note this is a C visibility problem, not an assembler one:
# adding -DPIC to X86ASMFLAGS does nothing, because these objects come from .c
# files rather than .asm.
FFMPEGCONF += --extra-cflags=-fvisibility=hidden

# Medea decoder allowlist: upstream ffmpeg builds 526 decoders; a desktop
# audio/video player needs a small fraction of that (common video/audio
# codecs, subtitle text formats, embedded cover art images). Everything else
# here is either a legacy video-game cutscene format (bink, smacker, roq,
# Electronic Arts *_ea, Interplay, Sierra vmd, id CIN, etc.), a hardware
# decoder shim for GPUs this box cannot use (*_cuvid, *_amf, *_v4l2m2m - see
# 00-STATE.md: both VA-API and NVDEC fail here, software decode only), a dead
# VoIP/telephony codec (g728, g729, ilbc, qcelp, msnsiren), an obsolete
# proprietary format (RealAudio, ATRAC, old QuickTime svq/qdmc), or a
# professional/scientific still-image or intermediate codec (dpx, exr, fits,
# dnxhd, prores_raw) nobody has in a home media library. Trimming these does
# not affect playback of standard video/audio files, DVD/Blu-ray rips, or
# common subtitle formats - see the keep list this is the complement of.
FFMPEGCONF += --disable-decoder='aasc,acelp_kelvin,adpcm_4xm,adpcm_adx,adpcm_afc,adpcm_agm,adpcm_aica,adpcm_argo,adpcm_ct,adpcm_dtk,adpcm_ea,adpcm_ea_maxis_xa,adpcm_ea_r1,adpcm_ea_r2,adpcm_ea_r3,adpcm_ea_xas,adpcm_g722,adpcm_g726,adpcm_g726le,adpcm_ima_acorn,adpcm_ima_alp,adpcm_ima_amv,adpcm_ima_apc,adpcm_ima_apm,adpcm_ima_cunning,adpcm_ima_dat4,adpcm_ima_dk3,adpcm_ima_dk4,adpcm_ima_ea_eacs,adpcm_ima_ea_sead,adpcm_ima_iss,adpcm_ima_moflex,adpcm_ima_mtf,adpcm_ima_oki,adpcm_ima_qt,adpcm_ima_rad,adpcm_ima_smjpeg,adpcm_ima_ssi,adpcm_ima_wav,adpcm_ima_ws'
FFMPEGCONF += --disable-decoder='adpcm_ima_xbox,adpcm_ms,adpcm_mtaf,adpcm_psx,adpcm_sanyo,adpcm_sbpro_2,adpcm_sbpro_3,adpcm_sbpro_4,adpcm_swf,adpcm_thp,adpcm_thp_le,adpcm_vima,adpcm_xa,adpcm_xmd,adpcm_yamaha,adpcm_zork,agm,aic,alias_pix,als,amv,anm,ansi,anull,apac,apng,aptx,aptx_hd,apv,arbc,argo,asv1,asv2,atrac1,atrac3,atrac3al,atrac3p,atrac3pal,atrac9,aura'
FFMPEGCONF += --disable-decoder='aura2,av1_amf,av1_cuvid,avrn,avrp,avs,avui,bethsoftvid,bfi,bink,binkaudio_dct,binkaudio_rdft,bintext,bitpacked,bmv_audio,bmv_video,bonk,brender_pix,c93,cavs,cbd2_dpcm,ccaption,cdgraphics,cdtoons,cdxl,cfhd,clearvideo,cljr,cllc,comfortnoise,cook,cpia,cri,cscd,cyuv,dds,derf_dpcm,dfa,dfpwm,dirac'
FFMPEGCONF += --disable-decoder='dnxhd,dolby_e,dpx,dsd_lsbf,dsd_lsbf_planar,dsd_msbf,dsd_msbf_planar,dsicinaudio,dsicinvideo,dss_sp,dst,dvaudio,dxa,dxtory,dxv,eacmv,eamad,eatgq,eatgv,eatqi,eightbps,eightsvx_exp,eightsvx_fib,escape124,escape130,evrc,exr,fastaudio,ffv1,ffvhuff,ffwavesynth,fic,fits,flashsv,flashsv2,flic,fmvc,fourxm,fraps,frwu'
FFMPEGCONF += --disable-decoder='ftr,g2m,g723_1,g728,g729,gdv,gem,gremlin_dpcm,h261,h263_v4l2m2m,h264_amf,h264_cuvid,h264_v4l2m2m,hap,hca,hcom,hdr,hevc_amf,hevc_cuvid,hevc_v4l2m2m,hnm4_video,hq_hqa,hqx,huffyuv,hymt,iac,idcin,idf,iff_ilbm,ilbc,imc,imm4,imm5,indeo2,indeo3,indeo4,indeo5,interplay_acm,interplay_dpcm,interplay_video'
FFMPEGCONF += --disable-decoder='ipu,jpeg2000,jpegls,jv,kgv1,kmvc,lagarith,lead,libgsm,libgsm_ms,loco,lscr,m101,mace3,mace6,magicyuv,mdec,media100,metasound,mimic,misc4,mjpeg_cuvid,mmvideo,mobiclip,motionpixels,mpc7,mpc8,mpeg1_cuvid,mpeg1_v4l2m2m,mpeg2_cuvid,mpeg2_v4l2m2m,mpeg4_cuvid,mpeg4_v4l2m2m,mpegvideo,msa1,mscc,msnsiren,msp2,msrle,mss1'
FFMPEGCONF += --disable-decoder='mss2,msvideo1,mszh,mts2,mv30,mvc1,mvc2,mvdv,mvha,mwsc,mxpeg,nellymoser,notchlc,nuv,on2avc,osq,paf_audio,paf_video,pam,pbm,pcm_f16le,pcm_f24le,pcm_lxf,pcm_s16be_planar,pcm_s24daud,pcm_s64be,pcm_s64le,pcm_s8_planar,pcm_sga,pcm_u16be,pcm_u16le,pcm_u24be,pcm_u24le,pcm_u32be,pcm_u32le,pcm_vidc,pcx,pdv,pfm,pgm'
FFMPEGCONF += --disable-decoder='pgmyuv,pgx,phm,photocd,pictor,pixlet,ppm,prores_raw,prosumer,psd,ptx,qcelp,qdm2,qdmc,qdraw,qoa,qoi,qpeg,r10k,r210,ra_144,ra_288,ralf,rasc,rka,rl2,roq,roq_dpcm,rpza,rscc,rtv1,rv10,rv20,rv30,rv40,rv60,s302m,sanm,sbc,scpr'
FFMPEGCONF += --disable-decoder='screenpresso,sdx2_dpcm,sga,sgi,sgirle,sheervideo,shorten,simbiosis_imx,sipr,siren,smackaud,smacker,smc,smvjpeg,snow,sol_dpcm,sonic,sp5x,speedhq,speex,srgc,sunrast,svq1,svq3,tak,targa,targa_y216,tdsc,thp,tiertexseqvideo,tmv,truemotion1,truemotion2,truemotion2rt,truespeech,tscc,tscc2,twinvq,txd,ulti'
FFMPEGCONF += --disable-decoder='utvideo,v210,v210x,v308,v408,v410,vb,vble,vbn,vc1_cuvid,vc1_v4l2m2m,vcr1,vmdaudio,vmdvideo,vmix,vmnc,vnull,vp4,vp5,vp7,vp8_cuvid,vp8_v4l2m2m,vp9_amf,vp9_cuvid,vp9_v4l2m2m,vqa,vqc,vvc,wady_dpcm,wavarc,wbmp,wcmv,wnv1,wrapped_avframe,ws_snd1,xan_dpcm,xan_wc3,xan_wc4,xbin'
FFMPEGCONF += --disable-decoder='xbm,xface,xl,xma1,xma2,xpm,xwd,y41p,ylc,yop,yuv4,zero12v,zerocodec,zlib,zmbv'

endif

ifdef HAVE_ANDROID
# broken text relocations
ifeq ($(ANDROID_ABI), x86)
FFMPEGCONF +=  --disable-mmx --disable-mmxext --disable-inline-asm
endif
endif

# Windows
ifdef HAVE_WIN32
ifndef HAVE_VISUALSTUDIO
DEPS_ffmpeg += mingw12-fixes $(DEPS_mingw12-fixes) d3d12 $(DEPS_d3d12)
endif
FFMPEGCONF += --target-os=mingw32
FFMPEGCONF += --enable-w32threads
# We don't currently support D3D12 in VLC
FFMPEGCONF += --disable-d3d12va
ifndef HAVE_WINSTORE
FFMPEGCONF += --enable-dxva2
else
FFMPEGCONF += --disable-dxva2 --disable-mediafoundation
endif

ifeq ($(ARCH),x86_64)
FFMPEGCONF += --arch=x86_64
else
ifeq ($(ARCH),i386) # 32bits intel
FFMPEGCONF+= --arch=x86
else
ifdef HAVE_ARMV7A
FFMPEGCONF+= --arch=arm
endif
endif
endif

else # !Windows
FFMPEGCONF += --enable-pthreads
endif

# Solaris
ifdef HAVE_SOLARIS
ifeq ($(ARCH),x86_64)
FFMPEGCONF += --cpu=core2
endif
FFMPEGCONF += --target-os=sunos --enable-pic
endif

ifdef HAVE_EMSCRIPTEN
FFMPEGCONF+= --arch=wasm32 --target-os=none --enable-pic
endif

# Build
PKGS += ffmpeg
ifeq ($(call need_pkg,"libavcodec >= $(FFMPEG_LAVC_MIN) libavformat >= 53.21.0 libswscale"),)
PKGS_FOUND += ffmpeg
endif

FFMPEGCONF += --nm="$(NM)" --ar="$(AR)" --ranlib="$(RANLIB)"

$(TARBALLS)/ffmpeg-$(FFMPEG_BASENAME).tar.xz:
	$(call download_git,$(FFMPEG_GITURL),$(FFMPEG_BRANCH),$(FFMPEG_HASH))

# .sum-ffmpeg: $(TARBALLS)/ffmpeg-$(FFMPEG_BASENAME).tar.xz
# 	$(call check_githash,$(FFMPEG_HASH))
# 	touch $@

$(TARBALLS)/ffmpeg-$(FFMPEG_VERSION).tar.xz:
	$(call download_pkg,$(FFMPEG_URL),ffmpeg)

.sum-ffmpeg: ffmpeg-$(FFMPEG_VERSION).tar.xz

ffmpeg: ffmpeg-$(FFMPEG_VERSION).tar.xz .sum-ffmpeg
	$(UNPACK)
	$(APPLY) $(SRC)/ffmpeg/dxva_vc1_crash.patch
	$(APPLY) $(SRC)/ffmpeg/h264_early_SAR.patch
	$(APPLY) $(SRC)/ffmpeg/0001-avcodec-dxva2-add-support-for-HEVC-RExt-DXVA-profile.patch
	$(APPLY) $(SRC)/ffmpeg/0001-avcodec-mpeg12dec-don-t-call-hw-end_frame-when-start.patch
	$(APPLY) $(SRC)/ffmpeg/0002-avcodec-mpeg12dec-don-t-end-a-slice-without-first_sl.patch
	$(APPLY) $(SRC)/ffmpeg/0001-fix-mf_utils-compilation-with-mingw64.patch
	$(APPLY) $(SRC)/ffmpeg/0011-avcodec-videotoolboxenc-disable-calls-on-unsupported.patch
	$(APPLY) $(SRC)/ffmpeg/avcodec-fix-compilation-visionos.patch
	$(MOVE)

.ffmpeg: ffmpeg
	$(MAKEBUILDDIR)
	$(MAKECONFDIR)/configure $(FFMPEGCONF)
	+$(MAKEBUILD)
	+$(MAKEBUILD) install-libs install-headers
	touch $@
