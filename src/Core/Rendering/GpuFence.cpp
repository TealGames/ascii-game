#include "Core/Rendering/GpuFence.hpp"
#include "Utils/Debug.hpp"

#ifdef OPENGL
#include "Platform/OpenGl/OpenGlGpuFence.hpp"
#endif

namespace Engine::Rendering
{
	std::string ToString(const GpuFenceStatus status)
	{
		if (status == GpuFenceStatus::Inactive) return "Inactive";
		else if (status == GpuFenceStatus::Signaled) return "Signaled";
		else if (status == GpuFenceStatus::Unsignaled) return "Unsignaled";
		else if (status == GpuFenceStatus::Failed) return "Failed";

		LogError(std::format("Attempted to convert gpu fence status to string but it has no actions"));
		return "";
	}
	bool IsSignaledStatus(const GpuFenceStatus status)
	{
		return status == GpuFenceStatus::Signaled;
	}

	GpuFence::GpuFence(const bool insertFence, const GpuFenceCallbacks& callbacks)
		: m_callbacks(callbacks), m_implStorage(), m_hasInserted(false) 
	{
		if (insertFence) Insert();
	}
	GpuFence::GpuFence(GpuFence&& other) noexcept
		: m_callbacks(std::exchange(other.m_callbacks, {})),
		m_implStorage(std::exchange(other.m_implStorage, {})),
		m_hasInserted(std::exchange(other.m_hasInserted, false)) {}

	GpuFence::~GpuFence()
	{
		Destroy();
	}

	void GpuFence::Insert()
	{
		if (m_hasInserted)
		{
			GpuFenceStatus status = GetStatusDestructive(0);
			if (!IsSignaledStatus(status))
			{
				LogError(std::format("Attempted to insert gpu fence, but it already has "
					"been inserted (current status:{})", Rendering::ToString(status)));
				return;
			}
		}
		m_callbacks.m_InsertFunc(m_implStorage);
		m_hasInserted = true;
	}
	bool GpuFence::HasInserted() const
	{
		return m_hasInserted;
	}
	void GpuFence::Destroy()
	{
		if (!m_hasInserted)
			return;

		m_callbacks.m_DestroyFunc(m_implStorage);
		m_hasInserted = false;
	}
	GpuFenceStatus GpuFence::GetStatusDestructive(const std::uint64_t timeout)
	{
		const GpuFenceStatus status = GetStatus(timeout);
		if (status != GpuFenceStatus::Unsignaled)
		{
			Destroy();
		}

		return status;
	}
	GpuFenceStatus GpuFence::GetStatus(const std::uint64_t timeout) const
	{
		if (!m_hasInserted)
			return GpuFenceStatus::Inactive;

		GpuFenceStatus status = GpuFenceStatus::Failed;
		if (timeout == 0)
			status = m_callbacks.m_GetStatusFunc(m_implStorage);
		else status= m_callbacks.m_WaitStatusFunc(m_implStorage, timeout);

		if (status == GpuFenceStatus::Failed)
		{
			LogError(std::format("Attempted to get gpufence status but it resulted in failed status"));
		}
		return status;
	}
	bool GpuFence::IsSignaled()
	{
		GpuFenceStatus status = GetStatus(0);
		
		if (status == GpuFenceStatus::Inactive)
		{
			LogError(std::format("Attempted to check if a fence was signaled, but the fence:{} is not even active", 
				bit_cast<std::uint64_t>(m_implStorage)));
			return false;
		}
		return IsSignaledStatus(status);
	}
	void GpuFence::WaitUntilSignal()
	{
		GetStatus(-1);
	}

	GpuFence& GpuFence::operator=(GpuFence&& other) noexcept
	{
		m_callbacks = std::exchange(other.m_callbacks, {});
		m_implStorage = std::exchange(other.m_implStorage, {});
		m_hasInserted = std::exchange(other.m_hasInserted, false);
		return *this;
	}

	std::string GpuFence::ToString() const
	{
		return std::format("[GpuFence status:{} inserted:{}]", 
			Rendering::ToString(GetStatus(0)), m_hasInserted);
	}

	GpuFence CreateGpuFence(const bool insertFence)
	{
#ifdef OPENGL
		return OpenGl::CreateGpuFence(insertFence);
#endif
	}
}