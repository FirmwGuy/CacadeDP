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
                    projection:   3,  // Projection for 360 image/video.

                    srt:          1,  // Subs use SRT (SubRip Text), SSA (SubStation Alpha) otherwise.
                    //english:      1,  // English content available.
                    //spanish:      1,  // Spanish content available.
                    //indi:         1,  // Indi content available.
                    //mandarin:     1,  // Chinese mandarin content available.
                    //russian:      1,  // Russian content available.

                    _reserved:    1;
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

    CDP_MM_COTNR_SVG,           // Scalable vector graphics.
    CDP_MM_COTNR_PDF,           // Portable document format.

    CDP_MM_COTNR_OBJ,           // Open 3D container.
    CDP_MM_COTNR_FBX,           // Common 3D container.

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
    CDP_MM_SQ_MONO,             // Mono (1) audio channel.
    CDP_MM_SQ_STEREO,           // Stereo (2) audio channels.
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

enum _cdpMultimediaProjection {
    CDP_MM_PROJ_NONW,           // Unprojected.
    CDP_MM_PROJ_EQUIRECT,       // Equirectangular projection (the most common).
    CDP_MM_PROJ_CUBEMAP,        // Skybox kind of projection.
    CDP_MM_PROJ_EQUIANG,        // Equiangular (used by Google).

    CDP_MM_PROJ_OTHER = 7
};


enum _cdpMultimediaTagID {
    // Children
    CDP_TAG_MM_RESOLUTION,   // Image/video width in pixels.

    CDP_TAG_MM_DURATION,     // Duration in mili-seconds.
    CDP_TAG_MM_FRAMES,       // Duration in frames.
    CDP_TAG_MM_SAMPLES,      // Duration in audio samples.

    CDP_TAG_MM_ANIM_NAME,    // Name/id of animations.
    CDP_TAG_MM_ANIM_INDEX,   // Index of animation.

    CDP_TAG_MM_METADATA     // Anex information related to media (eg, copywrite, license, etc).

    CDP_TAG_MM_LANGUAGE,     // A list of per-language audio tracks.
    CDP_TAG_MM_SUBTITLE,     // A list of per-language subtitle tracks.

    // Uses
    CDP_TAG_MM_IMAGE,       // Static image.
    CDP_TAG_MM_AUDIO,       // Pure audio.
    CDP_TAG_MM_VIDEO,       // Video or animated image.
    CDP_TAG_MM_CAPTION,     // Textual overlay or subtitle.
    CDP_TAG_MM_3D,          // 3D model.

    CDP_TAG_MM_ICON,
    CDP_TAG_MM_THUMBNAIL,
    CDP_TAG_MM_PREVIEW,
    CDP_TAG_MM_BACKGROUND,
    CDP_TAG_MM_SCREENSHOT,

    CDP_TAG_MM_SOUND_EFFECT,
    CDP_TAG_MM_DIALOG,
    CDP_TAG_MM_MUSIC,
    CDP_TAG_MM_LOOP,
    CDP_TAG_MM_A_RECORDING,

    CDP_TAG_MM_MOVIE,
    CDP_TAG_MM_CLIP,
    CDP_TAG_MM_SCREEN_VCAP,

    CDP_TAG_MM_ANIMATION,
    CDP_TAG_MM_SPRITE_ACTION,
    CDP_TAG_MM_SPRITE_IDLE,

    // Agencies
    CDP_TAG_MM_LOAD,
    CDP_TAG_MM_UNLOAD,
    CDP_TAG_MM_PLAY,
    CDP_TAG_MM_PAUSE,
    CDP_TAG_MM_REWIND,
    CDP_TAG_MM_FORWARD,
    CDP_TAG_MM_STOP,
};


#endif
