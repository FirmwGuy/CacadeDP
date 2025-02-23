/*
 *  Copyright (c) 2024-2025 Victor M. Barrientos
 *  (https://github.com/FirmwGuy/CacadeDP)
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

#ifndef CDP_MULTIMEDIA_H
#define CDP_MULTIMEDIA_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpMultimedia,
            container:  4,     // Container for data (file format).
            audio:      4,     // Codec for audio data.
            soundq:     3,     // Sound quality in audio/video.
            sampling:   3,     // Audio sampling frequency.
            video:      4,     // Codec for video data.
            imageq:     3,     // Image/video quality.
            icspace:    3,     // Image/video color space.
            framerate:  3,     // Animation/video frames per second.
            projection: 3,     // Projection for 360 image/video.
            subtitle:   2,     // Subtitles encoding if available.
                       
            _reserved:  18  
);


enum _cdpMultimediaContainer {
    CDP_MM_CONTAINER_RAW,       // No container (plain data content).

    CDP_MM_CONTAINER_PNG,       // Losseless image compression.
    CDP_MM_CONTAINER_JPG,       // Lossy image compression.

    CDP_MM_CONTAINER_OGG,       // Open audio container.
    CDP_MM_CONTAINER_MP3,       // Common audio container.

    CDP_MM_CONTAINER_MKV,       // Open video container.
    CDP_MM_CONTAINER_MP4,       // Common video container.

    CDP_MM_CONTAINER_MTS,       // MPEG Transport Stream.
    CDP_MM_CONTAINER_MOV,       // Apple streaming stuff.

    CDP_MM_CONTAINER_OTHER = 15
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

    CDP_MM_SQ_MONO,             // Mono 16 BPSam.

    CDP_MM_SQ_STEREO,           // Stereo (signed) 16 BPSam.
    CDP_MM_SQ_STEREO24,         // Stereo (signed) 24 BPSam.
    CDP_MM_SQ_STEREO32F,        // Stereo (float) 32 BPSam.

    CDP_MM_SQ_SORROUND,         // 5.1 surround audio 16 BPSam.

    CDP_MM_SQ_OTHER = 7
};

enum _cdpMultimediaASample {
    CDP_MM_ASAMP_44K,           // The standard.
    CDP_MM_ASAMP_48K,           // HQ sampling.
    CDP_MM_ASAMP_32K,           // LQ sampling.
    CDP_MM_ASAMP_22K,           // Legacy freq.

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

    CDP_MM_IQ_MONOCHROME,       // Image is a bitmask (1BPP).
    CDP_MM_IQ_GRAYSCALE,        // Non colored image (8BPP).

    CDP_MM_IQ_COLOR,            // Colored image (16BPP).
    CDP_MM_IQ_HICOLOR,          // High colored image (24BPP).
    CDP_MM_IQ_TRUECOLOR,        // Truly colored image (32BPP).

    CDP_MM_IQ_OTHER = 7
};

enum _cdpMultimediaColorSpace {
    CDP_MM_COLSPA_RGB,          // Computer RGB colorspace.
    CDP_MM_COLSPA_RGBA,         // RGB with Alpha (transparency) channel.
    CDP_MM_COLSPA_YUV,          // Video YUV color shceme.
    CDP_MM_COLSPA_INDEX,        // Image uses a palette of 256 (or less) colors.

    CDP_MM_COLSPA_OTHER = 7
};

enum _cdpMultimediaFramerate {
    CDP_MM_FRATE_NONE,          // Static image.
    CDP_MM_FRATE_6,             // Used for animations.
    CDP_MM_FRATE_12,            // Used for smoother animations.
    CDP_MM_FRATE_24,            // Typical for old movies.
    CDP_MM_FRATE_30,            // Console framerate.
    CDP_MM_FRATE_60,            // Standard framerate.
    CDP_MM_FRATE_120,           // High framerate.

    CDP_MM_FRATE_OTHER = 7
};

enum _cdpMultimediaProjection {
    CDP_MM_PROJECTION_NONE,     // Unprojected.
    CDP_MM_PROJECTION_EQUIRECT, // Equirectangular projection (the most common).
    CDP_MM_PROJECTION_CUBEMAP,  // Skybox kind of projection.
    CDP_MM_PROJECTION_EQUIANG,  // Equiangular (used by Google).

    CDP_MM_PROJECTION_OTHER = 7
};

enum _cdpMultimediaSubtitle {
    CDP_MM_SUBS_NONE,           // No subtitles/captions.
    CDP_MM_SUBS_SRT,            // Subs in SRT (SubRip Text) format.
    CDP_MM_SUBS_SSA,            // Subs in SSA (SubStation Alpha) format.

    CDP_MM_SUBS_OTHER = 3
};


// Domain
#define CDP_WORD_MULTIMEDIA         CDP_IDC(0x0036ACA25A522420)      /* "multimedia"_ */
    
// Uses 
#define CDP_WORD_AUDIO              CDP_IDC(0x0006A44BC0000000)      /* "audio"______ */
#define CDP_WORD_IMAGE              CDP_IDC(0x0025A13940000000)      /* "image"______ */
#define CDP_WORD_ANIMATION          CDP_IDC(0x0005C9686897B800)      /* "animation"__ */
#define CDP_WORD_VIDEO              CDP_IDC(0x0059242BC0000000)      /* "video"______ */

    //CDP_MM_TAG_CAPTION,     // Textual overlay or subtitle.

    //CDP_MM_TAG_ICON,
    //CDP_MM_TAG_THUMBNAIL,
    //CDP_MM_TAG_PREVIEW,
    //CDP_MM_TAG_BACKGROUND,
    //CDP_MM_TAG_SCREENSHOT,

    //CDP_MM_TAG_SOUND_EFFECT,
    //CDP_MM_TAG_DIALOG,
    //CDP_MM_TAG_MUSIC,
    //CDP_MM_TAG_LOOP,
    //CDP_MM_TAG_A_RECORDING,

    //CDP_MM_TAG_MOVIE,
    //CDP_MM_TAG_CLIP,
    //CDP_MM_TAG_SCREEN_VCAP,

    //CDP_MM_TAG_SPRITE_ACTION,
    //CDP_MM_TAG_SPRITE_IDLE,

// Children
#define CDP_WORD_RESOLUTION         CDP_IDC(0x0048B37B2B44BDC0)     /* "resolution"_ */
#define CDP_WORD_DURATION           CDP_IDC(0x0012B20D12F70000)     /* "duration"___ */
#define CDP_WORD_FRAMES             CDP_IDC(0x001A416966000000)     /* "frames"_____ */
#define CDP_WORD_SAMPLES            CDP_IDC(0x004C2D830B300000)     /* "samples"____ */

    //CDP_MM_TAG_ANIM_NAME,       // Name/id of animations.
    //CDP_MM_TAG_ANIM_INDEX,      // Index of animation.

    //CDP_MM_TAG_METADATA         // Anex information related to media (eg, copywrite, license, etc).

    //CDP_MM_TAG_LANGUAGE,        // A list of per-language audio tracks.
    //CDP_MM_TAG_SUBTITLE,        // A list of per-language subtitle tracks.


// Agencies
#define CDP_WORD_PLAYER             CDP_IDC(0x004181C964000000)     /* "player"_____ */
#define CDP_WORD_MIXER              CDP_IDC(0x0035382C80000000)     /* "mixer"______ */
#define CDP_WORD_FILTER             CDP_IDC(0x00192CA164000000)     /* "filter"_____ */
#define CDP_WORD_BLENDER            CDP_IDC(0x000985710B200000)     /* "blender"____ */
#define CDP_WORD_SCALER             CDP_IDC(0x004C616164000000)     /* "scaler"_____ */
                                                                    
// Selectors                                                      
                                                                    
// Events                                                         
#define CDP_WORD_PLAY               CDP_IDC(0x004181C800000000)     /* "play"_______ */
#define CDP_WORD_PAUSE              CDP_IDC(0x0040359940000000)     /* "pause"______ */
#define CDP_WORD_REWIND             CDP_IDC(0x0048B74B88000000)     /* "rewind"_____ */
#define CDP_WORD_FORWARD            CDP_IDC(0x0019F2B864400000)     /* "forward"____ */
#define CDP_WORD_STOP               CDP_IDC(0x004E8F8000000000)     /* "stop"_______ */
                                                                    
#define CDP_WORD_END                CDP_IDC(0x0015C40000000000)     /* "end"________ */

    //CDP_MM_TAG_LOAD,
    //CDP_MM_TAG_UNLOAD,
    //CDP_MM_TAG_NEXT_PIXBUF,
    //CDP_MM_TAG_NEXT_AUDIOFRAME,
    //CDP_MM_TAG_CAN_REWIND,

#endif
