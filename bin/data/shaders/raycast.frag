#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect backfaceTexture;
uniform sampler3D volumeTexture;

uniform vec3 volumeDimensions;
uniform vec2 backfaceDimensions;

uniform float zOffset;
uniform float quality;
uniform float threshold;
uniform float density;

varying vec4 pos;

void main()
{
    vec3 start = gl_TexCoord[0].xyz;  // start position of the ray
    vec4 back_position = texture2DRect(backfaceTexture, (((pos.xy / pos.w) + 1.0) / 2.0) * backfaceDimensions); 
    // the coordinates for the lookup in the backface texture is the position converted to the backface dimensions:
    // pos.xy / w = [-1, 1]
    // + 1.0 / 2.0 = [0 , 1]
    // * backfaceDimensions = [0, backfaceDimensions]
//        gl_FragColor = vec4(back_position.rgb, 1.); return;
    vec3 dir = back_position.rgb - start;   // get the direction vector     (direction of the ray)
    float len = length(dir);                // and it's length              (lenght of the ray)
    float steps = len * length(volumeDimensions) * quality;    // get the number of steps: the lenght of the ray * the length of the volume dimensions * the 'quality'. the lower the quality, the lower the amount of steps
    float stepsize = len / steps;
    vec3 delta_dir = normalize(dir) * stepsize; // this will be added every step
    
    vec3 vec = start;                       // current position
    vec4 col_acc = vec4(0.);                // current color
    float alpha_acc = 0.;                   // current alpha
    float aScale =  density * (1.0/quality);
    vec4 color_sample;
    float alpha_sample;

    //raycast
    for(int i = 0; i < int(steps); i++) {
        color_sample = texture3D(volumeTexture,vec + vec3(0.0, 0.0, zOffset / volumeDimensions.z));      // get a color sample of the 3d texture
        if(color_sample.a > threshold) {                                                        // if it's alpha value is high enough
            alpha_sample = color_sample.a * aScale;                                       // scale the alpha (using density and quality)
            color_sample.a = 1.0;
            col_acc += (1.0 - alpha_acc) * color_sample * alpha_sample * 2.0;                   // add it to the current color
            alpha_acc += alpha_sample;                                                          // and to the current color
        }
        vec += delta_dir;       // move along

        if(alpha_acc >= 1.0) {
            break; // terminate if opacity > 1
        }
    }

	gl_FragColor = col_acc;
}
