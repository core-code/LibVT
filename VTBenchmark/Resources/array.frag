#extension GL_EXT_texture_array : require
uniform sampler2DArray tex;								
void main (void)									
{
	int x = int(gl_TexCoord[0].s * 2.0);
	int y =  int(gl_TexCoord[0].t * 2.0);
	vec4 color = texture2DArray(tex, vec3(gl_TexCoord[0].s / 2.0, gl_TexCoord[0].t / 2.0, float((x + y))));
	gl_FragColor = color;									
}															