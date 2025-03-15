#pragma once
#include "nlohmann/json.hpp"
using Json = nlohmann::json;

struct IJsonSerializable
{
	virtual ~IJsonSerializable() = default;

	/// <summary>
	/// Deserialize is meant to modify the fields of the derived type which should be the 
	/// one which is the template argument type
	/// </summary>
	/// <param name="json"></param>
	/// <returns></returns>
	/*virtual T& Deserialize(const Json& json) = 0;
	virtual Json Serialize(const T& component) = 0;*/

	virtual void Deserialize(const Json& json) = 0;
	virtual Json Serialize() = 0;
};

