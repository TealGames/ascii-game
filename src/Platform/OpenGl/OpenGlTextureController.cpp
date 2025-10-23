#include "Platform/OpenGl/OpenGlTextureController.hpp"

#ifdef OPENGL
#include "Utils/Platform/OpenGlUtils.hpp"

namespace Rendering
{
	namespace OpenGl
	{
		static void SetTextureBindStatus(const RenderObjectId id, const USlotIndex index, const bool status)
		{
			GLint maxUnits = 0;
			GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));
			if (index < 0 || index >= maxUnits)
			{
				LogError(std::format("[OPENGL]: Attempted to set bind status for texture "
					"to index:{} status:{} but index is out of bounds", index, status));
				return;
			}

			if (status)
			{
				GL_CALL(glBindTextureUnit(index, id));
			}
			else
			{
				GL_CALL(glBindTextureUnit(index, 0));
			}
		}
		TextureSlotController CreateTextureController()
		{
			return TextureSlotController(TextureControllerCallbacks
				{
					SetTextureBindStatus
				});
		}

		static void SetImageBindStatus(const RenderObjectId id, const TexelStorageType storage, const USlotIndex index, 
			const bool status, const AccessPermissions permissions)
		{
			GLint maxUnits = 0;
			GL_CALL(glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxUnits));
			if (index < 0 || index >= maxUnits)
			{
				LogError(std::format("[OPENGL]: Attempted to set bind status for image texture "
					"to index:{} status:{} but index is out of bounds", index, status));
				return;
			}

			if (status)
			{
				GL_CALL(glBindImageTexture(index, id, 0, GL_FALSE, 0, 
					OpenGlUtils::GetAccessPermission(permissions), 
					OpenGlUtils::GetStorage(storage)));
			}
			else
			{
				//Since we are unbinding the texture, the arguments except index and first 0 do NOT matter
				GL_CALL(glBindImageTexture(index, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8));
			}
		}
		ImageSlotController CreateImageController()
		{
			return ImageSlotController(ImageControllerCallbacks
				{
					SetImageBindStatus
				});
		}
	}
}
#endif