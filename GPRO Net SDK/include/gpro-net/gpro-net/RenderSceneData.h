#ifndef RENDER_SCENE_DATA_H
#define RENDER_SCENE_DATA_H

#include "cereal/cereal.hpp"


struct RenderSceneData
{
	float x, y, z;

	template <class Archive>
	void serialize(Archive& archive)
	{
		archive(x, y, z);
	}
};

#endif // !RENDER_SCENE_DATA_H

