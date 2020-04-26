#include "AssetLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../3dparty/include/stb_image.h"


Utility::AssetLoader::AssetLoader():
    m_fbxLoader(new FbxLoader)
{
}
 


bool Utility::AssetLoader::LoadTextureFromFile(const std::string& p_fileName, Gameplay::Texture& p_texture)
{
    //To set 3 components convert to 4 components set flip vertically
    stbi_set_flip_vertically_on_load(true);
    uint32_t componentRequired = 4;

    unsigned char * data = stbi_load(p_fileName.c_str(), &p_texture.width, &p_texture.height, &p_texture.componentSize, componentRequired);
    
    if (data)
    {
        p_texture.size = p_texture.width * p_texture.height * componentRequired;
        p_texture.data = new uint8_t[p_texture.size];
        memcpy(p_texture.data, data, p_texture.size);
        return true;
    }
	return false;
}

Utility::AssetLoader::~AssetLoader()
{
    
}
