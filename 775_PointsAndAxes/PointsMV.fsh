#version 420 core

in PVA
{
	vec4 pvaSet1;
	vec4 pvaSet2;
} pva_in;

out vec4 fragmentColor;

uniform vec4 color; 
uniform float attrToCutForRed, attrToCutForGreen;
uniform int attrToUseForColor;
void main()
{
	// TODO: Add one or more uniforms to:
	// 1) Decide whether to use the uniform "color" or a PVA.
	// 2) If a PVA, have a uniform specify which one, rather
	//    than arbitrarily selecting [2] of pvaSet1.
	if (pva_in.pvaSet1[attrToUseForColor] < attrToCutForRed) //red
		fragmentColor = color;
	else if (pva_in.pvaSet1[attrToUseForColor] < attrToCutForGreen) //green
		fragmentColor = vec4(0.0, 1.0, 0.0, 1.0);
	else	//blue
		fragmentColor = vec4(0.0, 0.0, 1.0, 1.0);
}
