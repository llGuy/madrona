#include <metal_stdlib>
using namespace metal;

struct v2f {
    float4 position [[position]];
};

constexpr sampler fsSampler(coord::normalized,
                            address::clamp_to_edge,
                            filter::linear);

v2f vertex vertMain(uint16_t idx [[vertex_id]])
{
    v2f o;

	float2 uv = float2((idx << 1) & 2, idx & 2);
	o.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);

    return o;
}

half4 fragment fragMain(v2f in [[stage_in]],
                        texture2d_array<float> texture [[texture(0)]])
{
    float2 uv = in.position.xy / 2.f + 1.f;

    float4 color = texture.sample(fsSampler, uv, 0);

    return half4(color);
}
