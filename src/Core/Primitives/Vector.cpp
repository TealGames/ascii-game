#include "Core/Primitives/Vector.hpp"

namespace Engine::Math
{
	std::string ToString(const VectorForm& form)
	{
		if (form == VectorForm::Component) return "Component";
		else if (form == VectorForm::MagnitudeDirection) return "Magnitude@Direction";
		else if (form == VectorForm::Unit) return "Unit";
		else return "UNDEFINED";
	}
}