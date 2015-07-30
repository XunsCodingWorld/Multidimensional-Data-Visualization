First decide whether material properties are per-vertex or per-primitive.
The corresponding shader programs are in pva_matl and ppu_matl, respectively.

Next, decide whether the Phong lighting model is implemented in the vertex
shader or the fragment shader. To do lighting model in vertex shader, use:

	p??_matl/phong.vsh --> p??_matl/incomingColorAndTexture.fsh

To do lighting model in fragment shader, use:

	p??_matl/basic.vsh --> p??_matl/phong.fsh

NOTE: The versions of incomingColorAndTexture.fsh in ppu_matl and pva_matl are identical.

BASIC_PHONG_insert.txt has the phong model implementation. Any changes must
be made to this file and then the result is to be replaced inside all versions of
phong.?sh.

BASIC_TEXTURE_insert.txt - same idea for fragment shaders using common texture strategies.

The fragment shader "simpleColor.fsh" is used by a couple of demo programs
and simply passes the incoming color out with no processing at all.

ALSO: Keep all the shaders here in sync with those in ~/OpenGL/c/glslutil/shaders.
