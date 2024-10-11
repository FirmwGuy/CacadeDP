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


CDP_METADATA_STRUCT(cdpMultimediaAttribute,
    cdpAttribute    container:    4,  // Container for data (file format).
                    audio:        3,  // Codec for audio data.
                    soundq:       2,  // Sound quality in audio/video.
                    sampling:     2,  // Audio sampling frequency.
                    video:        4,  // Codec for video data.
                    imageq:       2,  // Image quality in image/video.
                    icspace:      2,  // Image/video color space.
                    projection:   2,  // Projection for 360 image/video.

                    srt:          1,  // Subs use SRT (SubRip Text), SSA (SubStation Alpha) otherwise.
                    //english:      1,  // English sound/sub available.
                    //spanish:      1,  // Spanish sound/sub available.
                    //indi:         1,  // Indi sound/sub available.
                    //mandarin:     1,  // Chinese mandarin sound/sub available.
                    //russian:      1,  // Russian sound/sub available.

                    _reserved:    10;   // ToDo: expand to include vector/3D secenes and (stand alone) animations.
);

enum _cdpMultimediaRole {
    CDP_ROLE_MEDIA_IMAGE,       // Static image.
    CDP_ROLE_MEDIA_AUDIO,       // Pure audio.
    CDP_ROLE_MEDIA_VIDEO,       // Video or animated image.
    CDP_ROLE_MEDIA_CAPTION,     // Textual overlay or subtitle.
    CDP_ROLE_MEDIA_3D,          // 3D model.
    CDP_ROLE_MEDIA_METADATA     // Anex information related to media (eg, copywrite, license, etc).
};

enum _cdpMultimediaContainer {
    CDP_COTNR_RAW,              // No container (plain data content).

    CDP_COTNR_PNG,              // Losseless image compression.
    CDP_COTNR_JPG,              // Lossy image compression.

    CDP_COTNR_OGG,              // Open audio container.
    CDP_COTNR_MP3,              // Common audio container.

    CDP_COTNR_MKV,              // Open video container.
    CDP_COTNR_MP4,              // Common video container.

    CDP_COTNR_MTS,              // MPEG Transport Stream.
    CDP_COTNR_MOV,              // Apple streaming stuff.

    CDP_COTNR_SVG,              // Scalable vector graphics.
    CDP_COTNR_PDF,              // Portable document format.

    CDP_COTNR_OBJ,              // Open (Wavefront) 3D container.
    CDP_COTNR_FBX,              // Common (Flimbox) 3D container.
};

enum _cdpMultimediaAudio {
    // Lossless
    CDP_CODEC_RAW,              // Raw audio in PCM format.
    CDP_CODEC_FLAC,             // Open lossless audio compression.
    CDP_CODEC_WAV,              // Legacy losseless audio codec.

    // Lossy
    CDP_CODEC_AAC = 4,          // Common audio codec.
    CDP_CODEC_MP3,              // MP3 as a codec.
    CDP_CODEC_OPUS,             // Open audio compression.
    CDP_CODEC_VORBIS,           // Legacy open audio codec.
};

enum _cdpMultimediaSoundQ {
    CDP_SQ_NONE,                // No audio.
    CDP_SQ_MONO,                // Mono (1) audio channel.
    CDP_SQ_STEREO,              // Stereo (2) audio channels.
    CDP_SQ_SORROUND             // 5.1 sorround audio.
};

enum _cdpMultimediaASample {
    CDP_AUDSAMP_44K,            // The standard.
    CDP_AUDSAMP_48K,            // HQ sampling.
    CDP_AUDSAMP_32K,            // LQ sampling.
    CDP_AUDSAMP_22K             // Legacy freq.
};

enum _cdpMultimediaVideo {
    // Losseless
    CDP_CODEC_RAW,              // Raw video in pixel screen format.
    CDP_CODEC_APNG,             // Used for short animations.
    CDP_CODEC_FFV1,             // Used by FFMPEG.
    CDP_CODEC_HUFFYUV,          // Legacy losseless video codec.

    // Lossy
    CDP_CODEC_H264 = 8,         // Aka Advanced Video Coding.
    CDP_CODEC_H265,             // Aka High Efficiency Video Coding.
    CDP_CODEC_AV1,              // Open video codec.
    CDP_CODEC_VP9,              // Used by Google.
    CDP_CODEC_MP2,              // Legacy video codec.
};

enum _cdpMultimediaImageQ {
    CDP_IQ_NONE,                // No image.
    CDP_IQ_MONOCHROME,          // Image is a bitmask.
    CDP_IQ_GRAYSCALE,           // Non colored image.
    CDP_IQ_COLOR                // Colored image.
};

enum _cdpMultimediaColorSpace {
    CDP_COLSPA_RGB,             // Computer RGB colorspace.
    CDP_COLSPA_RGBA,            // RGB with Alpha (transparency) channel.
    CDP_COLSPA_YUV,             // Video YUV color shceme.
    CDP_COLSPA_INDEX            // Image uses a palette of 256 (or less) colors.
};

enum _cdpMultimediaProjection {
    CDP_VPROJ_EQUIRECT,         // Equirectangular projection (the most common).
    CDP_VPROJ_CUBEMAP,          // Skybox kind of projection.
    CDP_VPROJ_EQUIANG,          // Equiangular (used by Google).
};


enum _cdpMultimediaTagID {
    // Children
    CDP_TAG_MEDIA_RESOLUTION,   // Image/video width in pixels.

    CDP_TAG_MEDIA_DURATION,     // Duration in mili-seconds.
    CDP_TAG_MEDIA_FRAMES,       // Duration in frames.
    CDP_TAG_MEDIA_SAMPLES,      // Duration in audio samples.

    CDP_TAG_MEDIA_ANIM_NAME,    // Name/id of animations.
    CDP_TAG_MEDIA_ANIM_INDEX,   // Index of animation.

    CDP_TAG_MEDIA_LANGUAGE,     // A list of per-language audio tracks.
    CDP_TAG_MEDIA_SUBTITLE,     // A list of per-language subtitle tracks.
    //

    CDP_TAG_MEDIA_ICON,
    CDP_TAG_MEDIA_THUMBNAIL,
    CDP_TAG_MEDIA_PREVIEW,
    CDP_TAG_MEDIA_BACKGROUND,
    CDP_TAG_MEDIA_SCREENSHOT,

    CDP_TAG_MEDIA_SOUND_EFFECT,
    CDP_TAG_MEDIA_DIALOG,
    CDP_TAG_MEDIA_MUSIC,
    CDP_TAG_MEDIA_LOOP,
    CDP_TAG_MEDIA_A_RECORDING,

    CDP_TAG_MEDIA_MOVIE,
    CDP_TAG_MEDIA_CLIP,
    CDP_TAG_MEDIA_SCREEN_VCAP,

    CDP_TAG_MEDIA_ANIMATION,
    CDP_TAG_MEDIA_SPRITE_ACTION,
    CDP_TAG_MEDIA_SPRITE_IDLE,
};


#endif
