/*
	Copyright 2011-2021 Daniel S. Buckstein

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

/*
	animal3D SDK: Minimal 3D Animation Framework
	By Daniel S. Buckstein
	
	sRenderer-object.h
	Example of demo utility header file.
*/

#ifndef _GPRO_NET_SRENDERER_OBJECT_H_
#define _GPRO_NET_SRENDERER_OBJECT_H_

#include "gpro-net/gpro-net-client/gpro-net-RakNet-Client.hpp"

// math library
#include "animal3D-A3DM/animal3D-A3DM.h"


//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif	// __cplusplus

	
//-----------------------------------------------------------------------------

// matrix stack for a single scene object/model
typedef struct sModelMatrixStack
{
	a3mat4 modelMat;						// model matrix (object -> world)
	a3mat4 modelMatInverse;					// model inverse matrix (world -> object)
	a3mat4 modelMatInverseTranspose;		// model inverse-transpose matrix (object -> world skewed)
	a3mat4 modelViewMat;					// model-view matrix (object -> viewer)
	a3mat4 modelViewMatInverse;				// model-view inverse matrix (viewer -> object)
	a3mat4 modelViewMatInverseTranspose;	// model-view inverse transpose matrix (object -> viewer skewed)
	a3mat4 modelViewProjectionMat;			// model-view-projection matrix (object -> clip)
	a3mat4 atlasMat;						// atlas matrix (texture -> cell)
} sModelMatrixStack;

// matrix stack for a viewer object
typedef struct sProjectorMatrixStack
{
	a3mat4 projectionMat;					// projection matrix (viewer -> clip)
	a3mat4 projectionMatInverse;			// projection inverse matrix (clip -> viewer)
	a3mat4 projectionBiasMat;				// projection-bias matrix (viewer -> biased clip)
	a3mat4 projectionBiasMatInverse;		// projection-bias inverse matrix (biased clip -> viewer)
	a3mat4 viewProjectionMat;				// view-projection matrix (world -> clip)
	a3mat4 viewProjectionMatInverse;		// view-projection inverse matrix (clip -> world)
	a3mat4 viewProjectionBiasMat;			// view projection-bias matrix (world -> biased clip)
	a3mat4 viewProjectionBiasMatInverse;	// view-projection-bias inverse matrix (biased clip -> world)
} sProjectorMatrixStack;

// named scale modes
typedef enum eScaleMode
{
	scale_disable,
	scale_uniform,
	scale_nonuniform,
} eScaleMode;

// scene object descriptor
typedef struct sSceneObjectData
{
	a3vec4 position;		// scene position for direct control
	a3vec4 euler;			// euler angles for direct rotation control
	a3vec3 scale;			// scale (not accounted for in update, use separate utilities)
	eScaleMode scaleMode;	// 0 = off; 1 = uniform; other = non-uniform (nightmare)
} sSceneObjectData;
typedef struct sSceneObjectComponent
{
	a3ui32 sceneHierarchyIndex;
	sSceneObjectData* dataPtr;
	sModelMatrixStack* modelMatrixStackPtr;
} sSceneObjectComponent;

// projector/camera/viewer
typedef struct sProjectorData
{
	a3real fovy;						// persp: vert field of view/ortho: vert size
	a3real aspect;						// aspect ratio
	a3real znear, zfar;					// near and far clipping planes
	a3real ctrlMoveSpeed;				// how fast controlled camera moves
	a3real ctrlRotateSpeed;				// control rotate speed (degrees)
	a3real ctrlZoomSpeed;				// control zoom speed (degrees)
	a3boolean perspective;				// perspective or orthographic
} sProjectorData;
typedef struct sProjectorComponent
{
	a3ui32 sceneHierarchyIndex;
	a3ui32 projectorIndex;
	sProjectorData* dataPtr;
	sProjectorMatrixStack* projectorMatrixStackPtr;
	sSceneObjectComponent const* sceneObjectPtr;
} sProjectorComponent;

// simple point light
typedef struct sPointLightData
{
	a3vec4 position;					// position in rendering target space
	a3vec4 worldPos;					// original position in world space
	a3vec4 color;						// RGB color with padding
	a3real radius;						// radius (distance of effect from center)
	a3real radiusSq;					// radius squared (if needed)
	a3real radiusInv;					// radius inverse (attenuation factor)
	a3real radiusInvSq;					// radius inverse squared (attenuation factor)
} sPointLightData;
typedef struct sPointLightComponent
{
	a3ui32 sceneHierarchyIndex;
	a3ui32 pointLightIndex;
	sPointLightData* dataPtr;
	sSceneObjectComponent const* sceneObjectPtr;
} sPointLightComponent;


//-----------------------------------------------------------------------------

inline void renderer_resetModelMatrixStack(sModelMatrixStack* modelMatrixStack);
inline void renderer_resetProjectorMatrixStack(sProjectorMatrixStack* projectionMatrixStack);
inline void renderer_resetSceneObjectData(sSceneObjectData* sceneObjectData);
inline void renderer_resetProjectorData(sProjectorData* projectorData);
inline void renderer_resetPointLightData(sPointLightData* pointLightData);

inline void renderer_setPointLightRadius(sPointLightData* pointLightData, a3real const radius);

inline void renderer_initSceneObject(sSceneObjectComponent* sceneObject, a3ui32 const sceneHierarchyIndex, sSceneObjectData* sceneObjectDataArray, sModelMatrixStack* modelMatrixStackArray);
inline void renderer_initProjector(sProjectorComponent* projector, a3ui32 const sceneHierarchyIndex, a3ui32 const projectorIndex, sProjectorData* projectorDataArray, sProjectorMatrixStack* projectorMatrixStackArray, sSceneObjectComponent const* sceneObjectArray);
inline void renderer_initPointLight(sPointLightComponent* pointLight, a3ui32 const sceneHierarchyIndex, a3ui32 const pointLightIndex, sPointLightData* pointLightDataArray, sSceneObjectComponent const* sceneObjectArray);

// update rotation of data only
inline void renderer_rotateProjectorSceneObject(sProjectorComponent const* projector, const a3real deltaX, const a3real deltaY, const a3real deltaZ);

// update position of data only
//	(uses stack to align movement with current orientation)
inline void renderer_moveProjectorSceneObject(sProjectorComponent const* projector, const a3real deltaX, const a3real deltaY, const a3real deltaZ);

// update model matrix and inverse only using object's transformation data
inline void renderer_updateSceneObject(sSceneObjectComponent const* sceneObject, const a3boolean useZYX);

// update projection and inverse matrices only
inline void renderer_updateProjector(sProjectorComponent const* projector);

// update view-projection and inverse matrices only given projector with updated projection matrices
//	(uses its camera object with updated model matrices)
inline void renderer_updateProjectorViewMats(sProjectorComponent const* projector);

// update scene object's full stack given scene object with updated model matrices and reference projector with updated projection matrix
inline void renderer_updateSceneObjectStack(sSceneObjectComponent const* sceneObject, sProjectorComponent const* projector_active);

// update projection bias matrices given projector with updated projection and view-projection matrices
inline void renderer_updateProjectorBiasMats(sProjectorComponent const* projector, a3mat4 const bias, a3mat4 const biasInv);


//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif	// __cplusplus


#endif	// !_GPRO_NET_SRENDERER_OBJECT_H_