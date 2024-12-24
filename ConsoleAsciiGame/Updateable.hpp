#pragma once

struct Updateable
{
	/// <summary>
	/// Start is called before the first frame update
	/// </summary>
	virtual void Start() = 0;

	/// <summary>
	/// UpdateStart is called on start every frame update
	/// </summary>
	/// <param name="deltaTime"></param>
	virtual void UpdateStart(float deltaTime) = 0;

	/// <summary>
	/// UpdateEnd is called on every frame update end.
	/// Note: every UpdateStart will have finished by this point
	/// </summary>
	/// <param name="deltaTime"></param>
	virtual void UpdateEnd(float deltaTime) = 0;
};