
uniform sampler2D one;			
uniform sampler2D two;
uniform sampler2D three;
uniform sampler2D four;
					
void main (void)									
{
	int x = int(gl_TexCoord[0].s * 2.0);
	int y =  int(gl_TexCoord[0].t * 2.0);
	
	
	vec4 color1 = texture2D(one, vec2(gl_TexCoord[0].s / 2.0, gl_TexCoord[0].t / 2.0));
	vec4 color2 = texture2D(two, vec2(gl_TexCoord[0].s / 2.0, gl_TexCoord[0].t / 2.0));
//	vec4 color3 = texture2D(three, vec2(gl_TexCoord[0].s / 2.0, gl_TexCoord[0].t / 2.0));
//	vec4 color4 = texture2D(four, vec2(gl_TexCoord[0].s / 2.0, gl_TexCoord[0].t / 2.0));
	
	
	gl_FragColor = color1 * float(x) + color2 * float(x+1);		
}															