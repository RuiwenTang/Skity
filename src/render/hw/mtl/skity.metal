
#include <metal_stdlib>
using namespace metal;

namespace skity{
    typedef struct {
        float2 position [[ attribute(0) ]]; // x,y
        float4 texCoord [[ attribute(1) ]]; // mix, u , v , float3 is aliment to float4 for perfermance reason
    } Vertex;
    
    typedef struct {
        float4 position [[ position ]];
        float3 texCoord;
        float2 vPosition;
    } ColorInOut;
    
    
    vertex ColorInOut passThroughVertex(Vertex v [[stage_in]],
                                        constant float4x4 &mvp [[ buffer(1) ]],
                                        constant float4x4 &userTransfrom [[ buffer(2) ]],
                                        uint vid [[ vertex_id ]])
    {
//        Vertex v = in[vid];
        ColorInOut out;
        out.vPosition = v.position;
        out.position = mvp * userTransfrom * float4(v.position, 0.0, 1.0);
        out.texCoord = v.texCoord.xyz;
        return out;
    }
    
#define M_PI 3.1415926535897932384626433832795
    // Fixme to solve uniform array length
#define MAX_COLORS 32
    
#define PIPELINE_MODE_STENCIL 0
#define PIPELINE_MODE_UNIFORM_COLOR 1
#define PIPELINE_MODE_IMAGE_TEXTURE 2
#define PIPELINE_MODE_LINEAR_GRADIENT 3
#define PIPELINE_MODE_RADIAL_GRADIENT 4
#define PIPELINE_MODE_FBO_TEXTURE 5
#define PIPELINE_MODE_HORIZONTAL_BLUR 6
#define PIPELINE_MODE_VERTICAL_BLUR 7
#define PIPELINE_MODE_SOLID_BLUR 8
#define PIPELINE_MODE_OUTER_BLUR 9
#define PIPELINE_MODE_INNER_BLUR 10
    
#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_CIRCLE 2
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4
#define VERTEX_TYPE_TEXT 5
    
    
    
    float4 lerp_color(float current,
                      constant int2 &gradientCounts,
                      constant float4 &gradientBounds,
                      const device vector_float4 * gradientColors,
                      const device float *gradientStops)
    {
        current = metal::min(current, 1.0);
        int colorCount = gradientCounts[0];
        int stopCount = gradientCounts[1];
        int premulAlpha = 0;
        
        int startIndex = 0;
        int endIndex = 1;
        
        float step = 1.0 / float(colorCount - 1);
        int i = 0;
        float start, end;
        for (i = 0; i < colorCount - 1; i++) {
            if (stopCount > 0) {
                start = gradientStops[i];
                end = gradientStops[i + 1];
            } else {
                start = step * float(i);
                end = step * float(i + 1);
            }
            
            if (current >= start && current <= end) {
                startIndex = i;
                endIndex = i + 1;
                break;
            }
        }
        
        if (i == colorCount - 1 && colorCount > 0) {
            return gradientColors[colorCount - 1];
        }
        
        float total = (end - start);
        float value = (current - start);
        
        float mixValue = 0.5;
        if (total > 0.0) {
            mixValue = value / total;
        }
        
        float4 color;
        if (premulAlpha == 1) {
            color =
            mix(float4(gradientColors[startIndex].xyz * gradientColors[startIndex].w,
                       gradientColors[startIndex].w),
                float4(gradientColors[endIndex].xyz * gradientColors[endIndex].w,
                       gradientColors[endIndex].w),
                mixValue);
        } else {
            color = mix(gradientColors[startIndex], gradientColors[endIndex], mixValue);
        }
        
        return color;
    }
    
    float4 calculate_radial_color(ColorInOut &in,constant int2 &gradientCounts,
                                  constant float4 &gradientBounds,
                                  const device vector_float4 * gradientColors,
                                  const device float *gradientStops,constant float4x4 &userTransform) {
        float4 mappedCenter = userTransform * float4(gradientBounds.xy, 0.0, 1.0);
        float4 currentPoint = userTransform * float4(in.vPosition, 0.0, 1.0);
        
        float mixValue = distance(mappedCenter.xy, currentPoint.xy);
        return lerp_color(mixValue / gradientBounds.z,gradientCounts,gradientBounds,gradientColors,gradientStops);
    }
    
    float4 calculate_linear_color(ColorInOut &in,constant int2 &gradientCounts,
                                  constant float4 &gradientBounds,
                                  const device vector_float4 * gradientColors,
                                  const device float *gradientStops,constant float4x4 &userTransform) {
        float4 startPointMaped = userTransform * float4(gradientBounds.xy, 0.0, 1.0);
        float4 endPointMapped = userTransform * float4(gradientBounds.zw, 0.0, 1.0);
        float4 currentPoint = userTransform * float4(in.vPosition, 0.0, 1.0);
        
        float2 sc = float2(currentPoint.x - startPointMaped.x,
                           currentPoint.y - startPointMaped.y);
        float2 se = (endPointMapped - startPointMaped).xy;
        
        if (sc.x * se.x + sc.y * se.y < 0.0) {
            return lerp_color(0.0,gradientCounts,gradientBounds,gradientColors,gradientStops);
        }
        
        float mixValue = dot(sc, se) / length(se);
        float totalDist = length(se);
        return lerp_color(mixValue / totalDist,gradientCounts,gradientBounds,gradientColors,gradientStops);
    }
    
    float4 calculate_gradient_color(constant int &colorType,ColorInOut &in,constant int2 &gradientCounts,
                                    constant float4 &gradientBounds,
                                    const device vector_float4 * gradientColors,
                                    const device float *gradientStops,constant float4x4 &userTransform) {
        if (colorType == PIPELINE_MODE_LINEAR_GRADIENT) {
            return calculate_linear_color(in,gradientCounts,gradientBounds,gradientColors,gradientStops,userTransform);
        } else if (colorType == PIPELINE_MODE_RADIAL_GRADIENT) {
            return calculate_radial_color(in,gradientCounts,gradientBounds,gradientColors,gradientStops,userTransform);
        } else {
            return float4(1.0, 0.0, 0.0, 1.0);
        }
    }
    
    float2 calculate_uv(ColorInOut &in,constant float4 &gradientBounds) {
        float4 mappedLT = float4(gradientBounds.xy, 0.0, 1.0);
        float4 mappedBR = float4(gradientBounds.zw, 0.0, 1.0);
        
        float4 mappedPos = float4(in.vPosition.xy, 0.0, 1.0);
        
        float totalX = mappedBR.x - mappedLT.x;
        float totalY = mappedBR.y - mappedLT.y;
        
        float vX = (mappedPos.x - mappedLT.x) / totalX;
        float vY = (mappedPos.y - mappedLT.y) / totalY;
        return float2(vX, vY);
    }
    
    // all blur calculation is based on
    // https://www.geeks3d.com/20100909/shader-library-gaussian-blur-post-processing-filter-in-glsl/
    float calculate_blur_norm(constant float &strokeWidth) {
        float sigma = strokeWidth + 2.0;
        return 1.0 / (sqrt(2.0 * M_PI) * sigma);
    }
    
    float calculate_blur_coffe(float norm, float step,constant float &strokeWidth) {
        // blur mode kernelSize is passed through StrokeWidth
        float sigma = strokeWidth + 2.0;
        return norm * exp(-1.5 * step * step / (sigma * sigma));
    }
    
    float4 calculate_blur(float2 uv, float2 dir, float2 step_vec,constant float &strokeWidth,const sampler &sampler,texture2d<float> &userTexture) {
        float norm = calculate_blur_norm(strokeWidth);
        
        float total = norm;
        float4 acc = userTexture.sample(sampler, uv) * norm;
        
        int kernel_size = int(strokeWidth);
        for (int i = 1; i <= kernel_size; i++) {
            float coffe = calculate_blur_coffe(norm, float(i),strokeWidth);
            float f_i = float(i);
            
            acc += userTexture.sample(sampler, uv - f_i * step_vec * dir) * coffe;
            acc += userTexture.sample(sampler, uv + f_i * step_vec * dir) * coffe;
            
            total += 2.0 * coffe;
        }
        
        acc = acc / total;
        return acc;
    }
    
    float4 calculate_vertical_blur(float2 uv,constant float &strokeWidth, const sampler &sampler,texture2d<float> &userTexture,constant float4 &gradientBounds) {
        float2 step_vec = float2(1.0 / (gradientBounds.z - gradientBounds.x),
                                 1.0 / (gradientBounds.w - gradientBounds.y));
        float2 dir = float2(0.0, 1.0);
        
        return calculate_blur(uv, dir, step_vec,strokeWidth,sampler,userTexture);
    }
    
    float4 calculate_horizontal_blur(float2 uv,constant float &strokeWidth,const sampler &sampler,texture2d<float> &userTexture,constant float4 &gradientBounds) {
        float2 step_vec = float2(1.0 / (gradientBounds.z - gradientBounds.x),
                                 1.0 / (gradientBounds.w - gradientBounds.y));
        float2 dir = float2(1.0, 0.0);
        
        return calculate_blur(uv, dir, step_vec,strokeWidth,sampler,userTexture);
    }
    
    float4 calculate_solid_blur(float2 uv,const  sampler &sampler,texture2d<float> &userTexture,texture2d<float> &fontTexture) {
        uv = float2(uv.x, 1.0 - uv.y);
        
        float4 raw_color = fontTexture.sample(sampler, uv);
        if (raw_color.a > 0.0) {
            return raw_color;
        } else {
            return userTexture.sample(sampler, uv);
        }
    }
    
    float4 calculate_outer_blur(float2 uv,const sampler &sampler,texture2d<float> &userTexture,texture2d<float> &fontTexture) {
        uv = float2(uv.x, 1.0 - uv.y);
        float4 raw_color = fontTexture.sample(sampler, uv);
        float4 blur_color = userTexture.sample(sampler, uv);
        
        if (raw_color.a > 0.0 && raw_color.a >= blur_color.a) {
            return float4(0.0, 0.0, 0.0, 0.0);
        } else {
            return userTexture.sample(sampler, uv);
        }
    }
    
    float4 calculate_inner_blur(float2 uv,const sampler &sampler,texture2d<float> &userTexture,texture2d<float> &fontTexture) {
        uv = float2(uv.x, 1.0 - uv.y);
        
        float4 raw_color = fontTexture.sample(sampler, uv);
        float4 blur_color = userTexture.sample(sampler, uv);
        
        if (raw_color.a > 0.0) {
            return blur_color * (raw_color.a - blur_color.a);
        } else {
            return float4(0.0, 0.0, 0.0, 0.0);
        }
    }
    
    
    
    fragment float4 passThroughtFragment(ColorInOut in [[stage_in]],
                                         texture2d<float> userTexture [[ texture(0) ]],
                                         texture2d<float> fontTexture [[ texture(1) ]],
                                         constant float4x4 &userTransform [[buffer(0)]],
                                         constant float &globalAlpha [[buffer(1)]],
                                         constant float4 &userColor [[buffer(2)]],
                                         constant float &strokeWidth [[buffer(3)]],
                                         constant int &colorType [[buffer(4)]],
                                         constant int2 &gradientCounts [[buffer(5)]],
                                         constant float4 &gradientBounds[[buffer(6)]],
                                         const device vector_float4 * gradientColors [[ buffer(7) ]],
                                         const device float *gradientStops [[buffer(8)]]
//                                         const device float *mpv [[ buffer(5) ]]
                                         )
    {
        constexpr sampler textureSampler (mag_filter::linear,
                                          min_filter::linear);
        int vertex_type = int(in.texCoord.x);
        float4 FragColor = float4(0);
        if (colorType == PIPELINE_MODE_STENCIL) {
            FragColor = float4(0, 0, 0, 0);
        }
        else if (colorType == PIPELINE_MODE_UNIFORM_COLOR) {
            FragColor = float4(userColor.xyz * userColor.w, userColor.w) * globalAlpha;
        }
        else if (colorType == PIPELINE_MODE_IMAGE_TEXTURE ||
                   (colorType >= PIPELINE_MODE_FBO_TEXTURE &&
                    colorType <= PIPELINE_MODE_INNER_BLUR)) {
            // Texture sampler
            float2 uv = calculate_uv(in,gradientBounds);

            uv.x = clamp(uv.x, 0.0, 1.0);

            uv.y = clamp(uv.y, 0.0, 1.0);

            // TODO this code need optimize
            if (colorType == PIPELINE_MODE_HORIZONTAL_BLUR) {
                FragColor = calculate_horizontal_blur(uv,strokeWidth,textureSampler,userTexture,gradientBounds);
            }
            else if (colorType == PIPELINE_MODE_VERTICAL_BLUR) {
                FragColor = calculate_vertical_blur(uv,strokeWidth, textureSampler,userTexture,gradientBounds);
            }
            else if (colorType == PIPELINE_MODE_SOLID_BLUR) {
                FragColor = calculate_solid_blur(uv,textureSampler,userTexture,fontTexture);
            }
            else if (colorType == PIPELINE_MODE_OUTER_BLUR) {
                FragColor = calculate_outer_blur(uv,textureSampler,userTexture,fontTexture);
            }
            else if (colorType == PIPELINE_MODE_INNER_BLUR) {
                FragColor = calculate_inner_blur(uv,textureSampler,userTexture,fontTexture);
            }

            if (colorType == PIPELINE_MODE_FBO_TEXTURE) {
                uv.y = 1.0 - uv.y;
            }
            FragColor = userTexture.sample(textureSampler, uv) * globalAlpha;
        }
        else {
            float4 g_color = calculate_gradient_color(colorType,in,gradientCounts,
                                                      gradientBounds,
                                                      gradientColors,
                                                      gradientStops,userTransform);
            FragColor = float4(g_color.xyz * g_color.w, g_color.w) * globalAlpha;
        }
        
        if (vertex_type == VERTEX_TYPE_TEXT) {
            float r = fontTexture.sample(textureSampler, in.texCoord.yz).r;
            FragColor = FragColor * r;
        }
        return FragColor;
    }
}





