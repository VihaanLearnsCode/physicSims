uniform vec2  u_center;
uniform float u_radius;
uniform vec4  u_color;
uniform float u_time;

void main()
{
    vec2 p = gl_FragCoord.xy;
    float dist = length(p - u_center);

    if (dist > u_radius) {
        discard;
    }

    float x = dist / u_radius; // 0 center -> 1 edge

    float edge  = smoothstep(0.8, 1.0, x);
    float glow  = 1.0 - smoothstep(0.0, 0.6, x);

    vec3 base  = u_color.rgb;
    vec3 inner = base * 1.4;
    vec3 color = mix(inner, base, x);

    color += 0.15 * glow;

    float alpha = u_color.a / 255.0 * (1.0 - 0.6 * edge);
    gl_FragColor = vec4(color, alpha);
}
