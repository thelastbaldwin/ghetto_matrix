// fragment shader
#version 410

uniform float time; //Parameter which we will pass from OF
uniform sampler2DRect tex0;
out vec4 outputColor;

// //the origin for the gl_FragCoord by default starts in the lower left
// //this sets it to the upper left
layout(origin_upper_left) in vec4 gl_FragCoord;


//following two functions from http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(){
    //extract just the x and y values from the current fragmentCoordinates
    vec2 pos = vec2(gl_FragCoord.x, gl_FragCoord.y);
    
    // //get sample color from texture
    vec4 sampleColor = texture(tex0, pos);
    
    //convert to hsv. rgb values not correspond to hsv
    vec3 hsvSample = rgb2hsv(sampleColor.rgb);
    hsvSample.r += time;
    
    //construct a new vec4 from the vec3 plus a hard-coded alpha
    outputColor = vec4(hsv2rgb(hsvSample), 1.0);
}