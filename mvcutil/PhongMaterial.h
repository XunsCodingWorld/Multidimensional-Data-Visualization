// PhongMaterial.h - Utility struct with Phong model properties

#ifndef PHONGMATERIAL_H
#define PHONGMATERIAL_H

static float _PhongMaterial_defaultColors[3] = {0.7, 0.7, 0.7};

struct PhongMaterial
{
	float ka[3], kd[3], ks[3];
	float shininess, alpha;

	PhongMaterial(float* rgb=NULL, float af=0.2, float df=0.2, float sf=0.8,
		float shine=25.0, float a=1.0) :
		shininess(shine), alpha(a)
	{
		if (rgb == NULL)
			copyColors(_PhongMaterial_defaultColors, af, df, sf);
		else
			copyColors(rgb, af, df, sf);
	}

	PhongMaterial(float r, float g, float b, float af=0.2, float df=0.2, float sf=0.8,
		float shine=25.0, float a=1.0) :
		shininess(shine), alpha(a)
	{
		float rgb[] = { r, g, b};
		copyColors(rgb, af, df, sf);
	}

	void copyColors(float* rgb, float af=0.2, float df=0.2, float sf=0.8)
	{
		if (rgb == NULL)
			return;
		for (int i=0 ; i<3 ; i++)
		{
			ka[i] = af * rgb[i];
			kd[i] = df * rgb[i];
			ks[i] = sf * rgb[i];
		}
	}

	void set(float* rgb, float af=0.2, float df=0.2, float sf=0.8,
		float shine=25.0, float a=1.0)
	{
		if (rgb == NULL)
			copyColors(_PhongMaterial_defaultColors, af, df, sf);
		else
			copyColors(rgb, af, df, sf);
		shininess = shine;
		alpha = a;
	}

	void set(float r, float g, float b, float af=0.2, float df=0.2, float sf=0.8,
		float shine=25.0, float a=1.0)
	{
		float rgb[] = { r, g, b};
		copyColors(rgb, af, df, sf);
		shininess = shine;
		alpha = a;
	}

};

#endif
