#pragma once
struct IValidateable
{
	virtual ~IValidateable() = 0;

	/// <summary>
	/// Returns true if was success and false if something went wrong
	/// </summary>
	/// <returns></returns>
	virtual bool Validate() = 0;
};

