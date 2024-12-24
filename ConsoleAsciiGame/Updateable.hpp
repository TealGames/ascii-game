#pragma once

struct Updateable
{
	/// <summary>
	/// Start is called before the first frame update
	/// </summary>
	virtual void Start() = 0;

	/// <summary>
	/// Update is called on every frame update
	/// </summary>
	/// <param name="deltaTime"></param>
	virtual void Update(float deltaTime) = 0;
};