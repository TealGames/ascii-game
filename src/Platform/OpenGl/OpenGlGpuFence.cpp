#include "Platform/OpenGl/OpenGlGpuFence.hpp"

#ifdef OPENGL
#include "Utils/Platform/OpenGlUtils.hpp"

namespace Engine::Rendering::OpenGl
{
	static void Insert(ImplStorage& storage)
	{
		GLsync sync;
		GL_CALL(sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
		//NOTE: this step is important to ensure that the fence commands gets flushed to ensure we 
		//have a fence and we do not override memory during write because the fence init command has not been sent to gpu
		GL_CALL(glFlush());
		storage = std::bit_cast<ImplStorage>(sync);
	}
	static GpuFenceStatus WaitStatus(const ImplStorage& storage, const std::uint64_t timeout)
	{
		GLsync fence = std::bit_cast<GLsync>(storage);
		if (glIsSync(fence) == GL_FALSE)
		{
			LogError("[OPENGL]: Tried to get status of glsync object but it is not a valid object");
			return GpuFenceStatus::Failed;
		}
		GLenum waitResult = 0;
		GL_CALL(waitResult = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, timeout));

		GpuFenceStatus status = GpuFenceStatus::Inactive;
		if (waitResult == GL_ALREADY_SIGNALED || waitResult == GL_CONDITION_SATISFIED)
			status = GpuFenceStatus::Signaled;
		else if (waitResult == GL_TIMEOUT_EXPIRED) status = GpuFenceStatus::Unsignaled;
		else if (waitResult == GL_WAIT_FAILED) status = GpuFenceStatus::Failed;

		if (status == GpuFenceStatus::Inactive)
		{
			LogError(std::format("[OPENGL]: attempted to get active status of fence but it had no actions"));
			return GpuFenceStatus::Failed;
		}

		return status;
	}
	static GpuFenceStatus GetStatus(const ImplStorage& storage)
	{
		GLsync fence = std::bit_cast<GLsync>(storage);
		if (glIsSync(fence) == GL_FALSE)
		{
			LogError("[OPENGL]: Tried to get status of glsync object but it is not a valid object");
			return GpuFenceStatus::Failed;
		}
		GLint syncStatus = 0;
		GL_CALL(glGetSynciv(fence, GL_SYNC_STATUS, sizeof(syncStatus), nullptr, &syncStatus));

		GpuFenceStatus resultStatus = GpuFenceStatus::Unsignaled;
		if (syncStatus == GL_SIGNALED) resultStatus = GpuFenceStatus::Signaled;

		return resultStatus;
	}
	static void Destroy(const ImplStorage& storage)
	{
		GLsync fence = std::bit_cast<GLsync>(storage);
		GL_CALL(glDeleteSync(fence));
	}

	GpuFence CreateGpuFence(const bool insertFence)
	{
		return GpuFence(insertFence, GpuFenceCallbacks(Insert, WaitStatus, GetStatus, Destroy));
	}
}

#endif