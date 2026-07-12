#pragma once

namespace Engine
{
	class IValidateable
	{
	private:
	public:
		virtual ~IValidateable() = default;

		/// <summary>
		/// Returns true if was success and false if something went wrong
		/// </summary>
		/// <returns></returns>
		virtual bool Validate() = 0;
	};
}


