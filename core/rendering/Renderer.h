#pragma once

#include "core/private_imports.h"
#include "core/scenes/Scene.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/MeshRenderer.h"

class Renderer
{
public:
    static void SetupRendering()
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
                    
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Configure alpha blending (something to do with transparency)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

	static void RenderScene(Scene* scene)
	{
		if (!scene)
			return;

		scene->GetRegistry().ForEach<Transform, MeshRenderer>([](Entity e, Transform& transform, MeshRenderer& meshRenderer) {
			if (e == NULL_ENTITY) return;

            meshRenderer.Draw();
		});
	}
};
