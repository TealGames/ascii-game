#pragma once
#include <array>
#include <string>

namespace Rendering
{
	constexpr size_t IMPL_SIZE = 8;
	using ImplStorage = std::array<std::byte, IMPL_SIZE>;

	enum class GpuFenceStatus : std::uint8_t
	{
		/// <summary>
		/// A status that indicates the gpu fence has not been inserted
		/// NOTE: no thread blocking occurs
		/// </summary>
		Inactive		= 0,
		/// <summary>
		/// The fence was signaled either with no timeout or within the timeout
		/// </summary>
		Signaled		= 1,
		/// <summary>
		/// The fence was not signaled within the timeout specified
		/// NOTE: thread blocking occurs for time= 'timeout arg'
		/// </summary>
		Unsignaled			= 2,
		/// <summary>
		/// Indicates an error has occured while waiting for the timeout
		/// NOTE: thread blocking occurs for 0 <= time < 'timeout arg' AND
		/// the fence will automatically be deleted
		/// </summary>
		Failed			= 3
	};
	std::string ToString(const GpuFenceStatus status);
	bool IsSignaledStatus(const GpuFenceStatus status);

	struct GpuFenceCallbacks
	{
		void(*m_InsertFunc)(ImplStorage& storage);
		GpuFenceStatus(*m_WaitStatusFunc)(const ImplStorage& storage, const std::uint64_t timeout);
		GpuFenceStatus(*m_GetStatusFunc)(const ImplStorage& storage);
		void(*m_DestroyFunc)(const ImplStorage& storage);
	};

	class GpuFence
	{
	private:
		GpuFenceCallbacks m_callbacks;
		ImplStorage m_implStorage;
		bool m_hasInserted;
	public:

	private:
		void Destroy();
	public:
		GpuFence(const bool insertFence, const GpuFenceCallbacks& callbacks);
		GpuFence(const GpuFence&) = delete;
		GpuFence(GpuFence&& other) noexcept;
		~GpuFence();

		void Insert();
		bool HasInserted() const;
		/// <summary>
		/// If inserted, blocks the thread of the time specified and returns
		/// a result of the wait time for the fence
		/// Not inserted -> no blocking -> returns Inactive
		/// Note: Signal status -> no blocking
		/// NOTE: -1 value for timeout means wait until finished
		/// NOTE: will destroy the fence object if status did not timeout
		/// </summary>
		/// <param name="timeout"></param>
		/// <returns></returns>
		GpuFenceStatus GetStatusDestructive(const std::uint64_t timeout);
		GpuFenceStatus GetStatus(const std::uint64_t timeout) const;
		//GpuFenceStatus GetStatus(const std::uint64_t timeout) const;
		/// <summary>
		/// Exectues get status with timeout of 0 ns (so no blocking occurs)
		/// and returns true if the status is satififed or signaled
		/// </summary>
		/// <returns></returns>
		bool IsSignaled();
		void WaitUntilSignal();

		GpuFence& operator=(GpuFence&& other) noexcept;

		std::string ToString() const;
	};

	GpuFence CreateGpuFence(const bool insertFence);
}
