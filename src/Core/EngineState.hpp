#include <cstdint>

enum class ExecutionState : std::uint8_t
{
	Init			= 0,
	Validation		= 1,
	Update			= 2,
};

enum class UpdateStatusCode : std::uint8_t
{
	Success		= 0,
	Exit		= 1,
	Error		= 2,
};


namespace Core { class Window; }
namespace AssetManagement { class AssetManager; }
namespace Rendering
{
	class GraphicsManager;
	struct GraphicsContext
	{
		Core::Window* m_Window = nullptr;
		GraphicsManager* m_GraphicsManager = nullptr;
	};
}

class CameraController;
class TimeKeeper;
class EngineState
{
private:
	ExecutionState m_executionState;
public:
	UpdateStatusCode m_LastUpdateStatus;
	Rendering::GraphicsContext m_GraphicsContext;
	CameraController* m_CameraController;
	AssetManagement::AssetManager* m_AssetManager;
	const TimeKeeper* m_TimeKeeper;

private:
public:
	EngineState();

	void SetExecutionState(ExecutionState newState);
	ExecutionState GetExecutionState() const;
};

