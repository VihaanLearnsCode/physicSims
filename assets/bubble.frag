// assets/bubble.frag

uniform float u_radius;
uniform vec2  u_center;   // in window coords, origin BOTTOM-left
uniform vec4  u_color;    // all in [0,1]
uniform float u_time;

void main()
{
    vec2 frag = gl_FragCoord.xy;

    float d = distance(frag, u_center);
    float r = u_radius;

    // outside bubble
    if (d > r) {
        discard;
    }

    float t = clamp(d / r, 0.0, 1.0); // 0 center, 1 edge

    float body = 1.0 - t * t;
    float rim  = smoothstep(0.8, 1.0, t);
    float shimmer = 0.15 * sin(u_time * 3.0 + d * 0.05);

    vec3 baseColor  = u_color.rgb;
    vec3 innerColor = baseColor * 1.3;
    vec3 col = mix(innerColor, baseColor, t);

    float intensity = body + rim * 0.3 + shimmer;
    intensity = clamp(intensity, 0.0, 1.3);

    float alpha = (1.0 - t) * u_color.a; // u_color.a already 0..1

    gl_FragColor = vec4(col * intensity, alpha);
}
