#version 150 core
uniform sampler2D diffuseTex;
uniform sampler2D shadowTex;

in Vertex {
	vec2 texCoord;
} IN;

out vec4 gl_FragColor;

void main(void) {
/*
	vec3 sample;
	float f = 10000.0;
	float n = 1.0;
	float z;
	sample = texture2D(shadowTex, IN.texCoord).rgb;
	z = (2 * n) / (f + n - sample.x * (f - n));
	*/
	gl_FragColor = texture2D(diffuseTex, IN.texCoord);
}
