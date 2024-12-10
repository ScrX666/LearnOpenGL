#ifndef _MYTEXTURE_H
#define _MYTEXTURE_H

class Texture
{
public:
	Texture(const char* path)
	{
		this->path = path;
	}

	unsigned int loadTexture();
	unsigned int loadHDRTexture();
private:
	const char* path;
};

#endif