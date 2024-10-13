/*
 *  Copyright (c) 2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#ifndef CDP_DOMAIN_MULTIMEDIA_H
#define CDP_DOMAIN_MULTIMEDIA_H


#include "cdp_record.h"


CDP_METADATA_STRUCT(cdpMultimedia,
    cdpAttribute    container:    4,  // Container for data (file format).
                    audio:        4,  // Codec for audio data.
                    soundq:       2,  // Sound quality in audio/video.
                    sampling:     3,  // Audio sampling frequency.
                    video:        4,  // Codec for video data.
                    imageq:       2,  // Image/video quality.
                    icspace:      3,  // Image/video color space.
                    framerate:    3,  // Animation/video frames per second.
                    projection:   3,  // Projection for 360 image/video.
                    subtitle:     2,  // Subtitles encoding if available.

                    _reserved:    2;
);


enum _cdpMultimediaContainer {
    CDP_MM_COTNR_RAW,           // No container (plain data content).

    CDP_MM_COTNR_PNG,           // Losseless image compression.
    CDP_MM_COTNR_JPG,           // Lossy image compression.

    CDP_MM_COTNR_OGG,           // Open audio container.
    CDP_MM_COTNR_MP3,           // Common audio container.

    CDP_MM_COTNR_MKV,           // Open video container.
    CDP_MM_COTNR_MP4,           // Common video container.

    CDP_MM_COTNR_MTS,           // MPEG Transport Stream.
    CDP_MM_COTNR_MOV,           // Apple streaming stuff.

    CDP_MM_COTNR_OTHER = 15
};

enum _cdpMultimediaAudio {
    // Lossless
    CDP_MM_ACODEC_RAW,          // Raw audio in PCM format.
    CDP_MM_ACODEC_FLAC,         // Open lossless audio compression.
    CDP_MM_ACODEC_WAV,          // Legacy losseless audio codec.

    // Lossy
    CDP_MM_ACODEC_AAC = 4,      // Common audio codec.
    CDP_MM_ACODEC_MP3,          // MP3 as a codec.
    CDP_MM_ACODEC_OPUS,         // Open audio compression.
    CDP_MM_ACODEC_VORBIS,       // Legacy open audio codec.

    CDP_MM_ACODEC_OTHER = 15
};

enum _cdpMultimediaSoundQ {
    CDP_MM_SQ_NONE,             // No audio.
    CDP_MM_SQ_MONO,             // Mono (1 audio channel).
    CDP_MM_SQ_STEREO,           // Stereo (2 audio channels).
    CDP_MM_SQ_SORROUND          // 5.1 sorround audio.
};

enum _cdpMultimediaASample {
    CDP_MM_ASAMP_44K,           // The standard.
    CDP_MM_ASAMP_48K,           // HQ sampling.
    CDP_MM_ASAMP_32K,           // LQ sampling.
    CDP_MM_ASAMP_22K            // Legacy freq.

    CDP_MM_ASAMP_OTHER = 7
};

enum _cdpMultimediaVideo {
    // Losseless
    CDP_VCODEC_RAW,             // Raw video in pixel screen format.
    CDP_VCODEC_APNG,            // Used for short animations.
    CDP_VCODEC_FFV1,            // Used by FFMPEG.
    CDP_VCODEC_HUFFYUV,         // Legacy losseless video codec.

    // Lossy
    CDP_VCODEC_H264 = 8,        // Aka Advanced Video Coding.
    CDP_VCODEC_H265,            // Aka High Efficiency Video Coding.
    CDP_VCODEC_AV1,             // Open video codec.
    CDP_VCODEC_VP9,             // Used by Google.
    CDP_VCODEC_MPEG2,           // Legacy video codec.

    CDP_VCODEC_OTHER = 15
};

enum _cdpMultimediaImageQ {
    CDP_MM_IQ_NONE,             // No image.
    CDP_MM_IQ_MONOCHROME,       // Image is a bitmask.
    CDP_MM_IQ_GRAYSCALE,        // Non colored image.
    CDP_MM_IQ_COLOR             // Colored image.
};

enum _cdpMultimediaColorSpace {
    CDP_MM_COLSPA_RGB,          // Computer RGB colorspace.
    CDP_MM_COLSPA_RGBA,         // RGB with Alpha (transparency) channel.
    CDP_MM_COLSPA_YUV,          // Video YUV color shceme.
    CDP_MM_COLSPA_INDEX         // Image uses a palette of 256 (or less) colors.

    CDP_MM_COLSPA_OTHER = 7
};

enum _cdpMultimediaFramerate {
    CDP_MM_FR_NONE,             // Static image.
    CDP_MM_FR_10,
    CDP_MM_FR_20,
    CDP_MM_FR_24,
    CDP_MM_FR_30,               // Standard framerate.
    CDP_MM_FR_60,
    CDP_MM_FR_120,

    CDP_MM_FR_OTHER = 7
};

enum _cdpMultimediaProjection {
    CDP_MM_PROJ_NONE,           // Unprojected.
    CDP_MM_PROJ_EQUIRECT,       // Equirectangular projection (the most common).
    CDP_MM_PROJ_CUBEMAP,        // Skybox kind of projection.
    CDP_MM_PROJ_EQUIANG,        // Equiangular (used by Google).

    CDP_MM_PROJ_OTHER = 7
};

enum _cdpMultimediaSubtitle {
    CDP_MM_SUBS_NONE,           // No subtitles/captions.
    CDP_MM_SUBS_SRT,            // Subs in SRT (SubRip Text) format.
    CDP_MM_SUBS_SSA,            // Subs in SSA (SubStation Alpha) format.

    CDP_MM_SUBS_OTHER = 3
};


enum _cdpMultimediaTag {
    // Uses
    CDP_MM_TAG_AUDIO,       // Pure audio.
    CDP_MM_TAG_IMAGE,       // Static image.
    CDP_MM_TAG_ANIMATION,   // Animated image.
    CDP_MM_TAG_VIDEO,       // Pure video.
    CDP_MM_TAG_CAPTION,     // Textual overlay or subtitle.

    CDP_MM_TAG_ICON,
    CDP_MM_TAG_THUMBNAIL,
    CDP_MM_TAG_PREVIEW,
    CDP_MM_TAG_BACKGROUND,
    CDP_MM_TAG_SCREENSHOT,

    CDP_MM_TAG_SOUND_EFFECT,
    CDP_MM_TAG_DIALOG,
    CDP_MM_TAG_MUSIC,
    CDP_MM_TAG_LOOP,
    CDP_MM_TAG_A_RECORDING,

    CDP_MM_TAG_MOVIE,
    CDP_MM_TAG_CLIP,
    CDP_MM_TAG_SCREEN_VCAP,

    CDP_MM_TAG_ANIMATION,
    CDP_MM_TAG_SPRITE_ACTION,
    CDP_MM_TAG_SPRITE_IDLE,

    // Children
    CDP_MM_TAG_RESOLUTION,      // Image/video width in pixels.

    CDP_MM_TAG_DURATION,        // Duration in milliseconds.
    CDP_MM_TAG_FRAMES,          // Duration in frames.
    CDP_MM_TAG_SAMPLES,         // Duration in audio samples.

    CDP_MM_TAG_ANIM_NAME,       // Name/id of animations.
    CDP_MM_TAG_ANIM_INDEX,      // Index of animation.

    CDP_MM_TAG_METADATA         // Anex information related to media (eg, copywrite, license, etc).

    CDP_MM_TAG_LANGUAGE,        // A list of per-language audio tracks.
    CDP_MM_TAG_SUBTITLE,        // A list of per-language subtitle tracks.


    // Agencies
    CDP_MM_TAG_LOAD,
    CDP_MM_TAG_UNLOAD,
    CDP_MM_TAG_NEXT_PIXBUF,
    CDP_MM_TAG_NEXT_AUDIOFRAME,
    CDP_MM_TAG_PLAY,
    CDP_MM_TAG_PAUSE,
    CDP_MM_TAG_CAN_REWIND,
    CDP_MM_TAG_REWIND,
    CDP_MM_TAG_FORWARD,
    CDP_MM_TAG_STOP,

    // Events
    CDP_MM_TAG_END              // End of media was reached.

    //
    CDP_MM_TAG_INI_COUNT
};


#endif
