
uniform sampler3D tex;			
					
void main (void)									
{
	int x = int(gl_TexCoord[0].s * 2.0);
	int y =  int(gl_TexCoord[0].t * 2.0);
	vec4 color = texture3D(tex, vec3(gl_TexCoord[0].s, gl_TexCoord[0].t, float(x + y) / 2.0));
	gl_FragColor = color;									
}															