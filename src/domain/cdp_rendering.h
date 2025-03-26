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
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
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

#ifndef CDP_RENDERING_H
#define CDP_RENDERING_H


#include <cdp_record.h>


CDP_ATTRIBUTE_STRUCT(
    cdpRendering,
            dynamic:      1,    // Static object rendering can be pre-rendered to textures.

            // mesh
            uv:           1,    // Mesh has UV texture coordinates.
            normals:      1,    // Mesh has vertex normals.
            tangents:     1,    // Mesh has vertex tangents.

            // texture
            srgb:         1,    // Is in sRGB color space.
            wrap:         2,    // Texture wrap mode.
            filter:       3,    // Texture filtering mode.
            mipmap:       1,    // Mipmaping enabled.
            multitex:     1,    // Texture is an array of textures (animation/video).
            compressed:   1,    // Texture compression.

            // material
            pbr:          1,    // Material has PBR (physical based rendering).
            transparency: 1,    // Material uses transparency.
            doubles:      1,    // Material is double sided.

            // shader
            vertex:       1,    // Vertex shader is present.
            fragment:     1,    // Fragment shader present.
            //geometry:     1,    // Geometry shader present.
            compute:      1,    // Compute shader present.

            // light & shadows
            shadow:       1,    // Object casts shadows.
            light:        3,    // Type of light emitted by object.

            // room
            hdri:         1,    // Room uses HDRI texture for lighting and reflections.
            ssao:         1,    // Room has SSAO (screen-space ambient occlusion).
            ssr:          1,    // Room has SSR (screen-space reflections).
            rsq:          3,    // Shadow quality inside room.
            cascades:     1,    // Enables cascaded shadows.
            volumetric:   1,    // Volumetric clouds.
            skybox:       1,    // Skybox texture.
            cubemap:      1,    // Cube map for reflections (on/off).
            decals:       1,    // Decals enabled in this room.

            // camera
            orthographic: 1,    // Camera projection (perspective or "0" is the default).
            motionblur:   1,    // Motion blur enabled.
            depth:        1,    // Depth of field enabled.
            tonemap:      3,    // Tone mapping type.
            rtype:        3,    // Rendering type.

            _reserved:    7
);


enum _cdpRenderingWrap {
    CDP_RENDER_WRAP_REPEAT,     // Repeat pattern.
    CDP_RENDER_WRAP_CLAMP,      // Clamp pattern.
    CDP_RENDER_WRAP_MIRROR,     // Mirror pattern.

    CDP_RENDER_WRAP_OTHER = 3
};

enum _cdpRenderingFilter {
    CDP_RENDER_FILTER_NEAREST,  // Nearest neighbor.
    CDP_RENDER_FILTER_BILINEAR, // Bilinear (2x2 pixel) filtering.
    CDP_RENDER_FILTER_TRILINEAR,// Linear-mipmap-linear.
    CDP_RENDER_FILTER_ANISOTROPIC,      // Anisotropic x16.
    CDP_RENDER_FILTER_ANISOTX8, // Anisotropic x8.
    CDP_RENDER_FILTER_ANISOTX4, // Anisotropic x4.
    CDP_RENDER_FILTER_ANISOTX2, // Anisotropic x2.

    CDP_RENDER_FILTER_OTHER = 7
};

enum _cdpRenderingLOD {
    CDP_RENDER_LOD_HIGHEST,     // The default LOD for any mesh is the mesh itself.
    CDP_RENDER_LOD_MEDIUM,      // Medium (50%) details.
    CDP_RENDER_LOD_LOW,         // Low (25%) details.
    CDP_RENDER_LOD_LOWEST,      // Lowest (10%) details possible.
    CDP_RENDER_LOD_BILLBOARD,   // Object is rendered as a sprite.

    CDP_RENDER_LOD_OTHER = 7
};

enum _cdpRenderingLight {
    CDP_RENDER_LIGHT_NONE,      // No light emission (default).
    CDP_RENDER_LIGHT_AMBIENT,   // Indirect uniform light.
    CDP_RENDER_LIGHT_DIRECTIONAL, // Infinite light source (sun).
    CDP_RENDER_LIGHT_SPOTLIGHT, // In one direction (cone).
    CDP_RENDER_LIGHT_POINT,     // In all directions (bulb).

    CDP_RENDER_LIGHT_OTHER = 7
};

enum _cdpRenderingRShadowQ {
    CDP_RENDER_RSQ_NONE,        // No shadows in this room.
    CDP_RENDER_RSQ_LOW,         // Low shadow quality.
    CDP_RENDER_RSQ_MEDIUM,      // Medium shadow quality.
    CDP_RENDER_RSQ_HIGH,        // High quality.
    CDP_RENDER_RSQ_ULTRA,       // Ultra.

    CDP_RENDER_RSQ_OTHER = 7
};

enum _cdpRenderingToneMap {
    CDP_RENDER_TONEM_NONE,      // No tone mapping.
    CDP_RENDER_TONEM_LINEAR,    // Linear tone mapping.
    CDP_RENDER_TONEM_REINHARD,  // Reinhard tone mapping.
    CDP_RENDER_TONEM_FILMIC,    // Filmic tone mapping.

    CDP_RENDER_TONEM_OTHER = 7
};

enum _cdpRenderingType {
    CDP_RENDER_TYPE_DYNAMIC,    // Render textured objects with full dynamic lights and shadows.
    CDP_RENDER_TYPE_LIGHTED,    // Render textured objects with static lights and shadows.
    CDP_RENDER_TYPE_TEXTURED,   // Render textured objects (flat light, no shadows).
    CDP_RENDER_TYPE_GRADIENT,   // Render interpolated surface colors.
    CDP_RENDER_TYPE_FLAT,       // Render a single color per face.
    CDP_RENDER_TYPE_WIREFRAME,  // Render only edges of polygons.
    CDP_RENDER_TYPE_POINT,      // Render only vertex points.

    CDP_RENDER_TYPE_OTHER = 7
};


/*
    Domain:
        'rendering'

    Uses:
        'model'
        'light'
        'camera'
        'room'

    Properties:
        'mesh'
        'texture'
        'color'
        'metal'

        'FOV'
        'intensity'
        'exposure'

        'LOD-MAX': maximum vertexs allowed at maximum LOD.

    Agencies:
        'renderer'

    Events:
        'refresh'

    //#define CDP_ACRON_VBUFFERX2
    //#define CDP_ACRON_VBUFFERX3
    //#define CDP_ACRON_VSYNC
*/


#endif
