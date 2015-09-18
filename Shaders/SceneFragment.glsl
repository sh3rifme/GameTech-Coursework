#version 150 core

uniform sampler2D 			diffuseTex;
uniform sampler2D 			bumpTex;
uniform sampler2DShadow	shadowTex;

uniform vec3 	cameraPos;
uniform vec4 	lightColour;
uniform vec3 	lightPos;
uniform float 	lightRadius;

in Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
} IN;

out vec4 gl_FragColor[2];

void main(void) {
	vec4 diffuse		= texture2D(diffuseTex, IN.texCoord);
	diffuse				= IN.colour + diffuse;
	
	mat3 TBN			= mat3(IN.tangent, IN.binormal, IN.normal);
	
	//vec3 normal		= normalize(TBN * (texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0));
	vec3 normal		= IN.normal;//normalize(TBN * (diffuse.rgb * 2.0 - 1.0));
	
	vec3 incident		= normalize(lightPos - IN.worldPos);
	float lambert		= max(0.0, dot(incident, IN.normal)); //should be "normal", but no bump map.
	
	float dist			= length(lightPos - IN.worldPos);
	float atten			= 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	
	vec3 viewDir		= normalize(cameraPos - IN.worldPos);
	vec3 halfDir		= normalize(incident + viewDir);
	
	float rFactor		= max(0.0, dot(halfDir, IN.normal)); //should be "normal", but no bump map.
	float sFactor		= pow(rFactor, 33.0);
	
	float shadow		= 1.0;
	
	if (IN.shadowProj.w > 0.0) {
		shadow = textureProj(shadowTex, IN.shadowProj);
	}
	
	lambert *= shadow;
	
	vec3 colour			= 	 (diffuse.rgb * lightColour.rgb);
	colour				+= (lightColour.rgb * sFactor) * 0.33;
	vec4 outV 			= vec4(colour * atten * lambert, diffuse.a);
	outV.rgb				+= (diffuse.rgb * lightColour.rgb) * 0.55;
	
	gl_FragColor[0]	= outV; //
	gl_FragColor[1]	= vec4(normal.xyz * 0.5 + 0.5, 1.0);
	//gl_FragColor = vec4(normal.xyz * 0.5 + 0.5, 1.0) * outV;
}