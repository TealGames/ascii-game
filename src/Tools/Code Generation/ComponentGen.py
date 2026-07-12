import os
from pathlib import Path
from enum import Enum
import CodeGenUtils
import Utils

class ComponentType(Enum):
    Editor = 0
    UI = 1
    World = 2

ROOT_PATH: str= "C:/Users/lukas/Desktop/Projects/Cpp/ascii-game/src/"
COMPONENT_DIR_PATH: str = "ECS/Component/"
COMPONENT_TYPE_DIR_PATH:str = COMPONENT_DIR_PATH + "Types/"
SYSTEM_TYPE_DIR_PATH:str = "ECS/Systems/Types/"
COMPONENT_LIST_PATH = ROOT_PATH + COMPONENT_DIR_PATH + "component_list.txt"
COMPONENT_MACRO_HPP_PATH = ROOT_PATH + COMPONENT_DIR_PATH + "ComponentMacros.hpp"
ALL_COMPONENT_HEADER_PATH = ROOT_PATH+ COMPONENT_DIR_PATH + "AllComponentsHeader.hpp"

#Helpful hpp includes as global vars
COMPONENT_BASE_HPP_INCLUDE:str = COMPONENT_DIR_PATH + "Component.hpp"
PROFILER_TIMER_HPP_INCLUDE:str = "Core/Analyzation/ProfilerTimer.hpp"
SERIALIZER_HPP_INCLUDE:str = "Core/Serialization/Serializer.hpp"

COMPONENT_NAME: str = "Test"
COMPONENT_NAMESPACE: str = ""
ENGINE_NAMESPACE: str = "Engine"
COMPONENT_TYPE: ComponentType = ComponentType.World

def create_component():
	component_full_name: str = get_component_filename()
	component_local_hpp_path: str = get_component_local_path(CodeGenUtils.FileType.Hpp)
	component_full_namespace: str = get_component_full_namespace()

	with open(ROOT_PATH + component_local_hpp_path, "w") as f:
		f.write(f"""\
#pragma once
#include "{COMPONENT_BASE_HPP_INCLUDE}"

namespace {component_full_namespace}
{{
	class {component_full_name} : public ECS::Component
	{{
	private:
	public:
		{component_full_name}();

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	}};
}}""")

	with open(ROOT_PATH + get_component_local_path(CodeGenUtils.FileType.Cpp), "w") as f:
		f.write(f"""\
#include "pch.hpp"
#include "{component_local_hpp_path}"
#include "{SERIALIZER_HPP_INCLUDE}"

namespace {component_full_namespace}
{{
	{component_full_name}::{component_full_name}() : Component() {{}}

	void {component_full_name}::InitFields()
	{{
		m_Fields = {{}};
	}}
	void {component_full_name}::Serialize(Serialization::Serializer& serializer) const
	{{
	}}
	void {component_full_name}::Deserialize(Serialization::Deserializer& deserializer)
	{{
	}}
	std::string {component_full_name}::ToString() const
	{{
		return std::format("[{COMPONENT_NAME}]");
	}}
}}""")

	system_full_name: str = get_system_file_name()
	system_local_hpp_path: str = get_system_local_path(CodeGenUtils.FileType.Hpp)
	with open(ROOT_PATH + system_local_hpp_path, "w") as f:
		if COMPONENT_TYPE == ComponentType.UI:
			f.write(f"""\
#pragma once

namespace Engine::Scenes {{ class GlobalEntityManager; }}
namespace {component_full_namespace}
{{
	class {system_full_name}
	{{
	private:
	public:

	private:
	public:
		{system_full_name};
		void SystemUpdate(Scenes::GlobalEntityManager& globalEntityManager, const float& deltaTime);
	}};
}}""")
		else:
			f.write(f"""\
#pragma once

namespace Engine::Scenes {{ class Scene; }}
namespace Engine::Camera {{ class CameraComponent; }}
namespace {component_full_namespace}
{{
	class {system_full_name}
	{{
	private:
	public:

	private:
	public:
		{system_full_name}();

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& cam, const float& deltaTime);
	}};
}}""")

	with open(ROOT_PATH + get_system_local_path(CodeGenUtils.FileType.Cpp), "w") as f:
		if COMPONENT_TYPE == ComponentType.UI:
			f.write(f"""\
#include "pch.hpp"
#include "{system_local_hpp_path}"
#include "{component_local_hpp_path}"

#ifdef ENABLE_PROFILER
#include "{PROFILER_TIMER_HPP_INCLUDE}"
#endif 

namespace {component_full_namespace}
{{
	{system_full_name}::{system_full_name}() {{}}

	void UILayoutSystem::SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime)
	{{
		globalEntityManager.OperateOnComponents<{component_full_name}>(ALL_ACTIVE_ENABLED_FLAG,
			[&deltaTime]({component_full_name}& data)-> void
			{{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("{system_full_name}::SystemUpdate");
#endif 
			}});
	}}
}}""")
			
		else:
			f.write(f"""\
#include "pch.hpp"
#include "{system_local_hpp_path}"
#include "{component_local_hpp_path}"

#ifdef ENABLE_PROFILER
#include "{PROFILER_TIMER_HPP_INCLUDE}"
#endif 

namespace {component_full_namespace}
{{
	{system_full_name}::{system_full_name}() {{}}

	void {system_full_name}::SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& cam, const float& deltaTime)
	{{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("{system_full_name}::SystemUpdate");
#endif 

	}}
}}""")

	CodeGenUtils.add_object_to_text_file(COMPONENT_LIST_PATH, 
									  get_component_full_namespace_filename(), str(COMPONENT_TYPE.name))
	CodeGenUtils.update_macros(COMPONENT_MACRO_HPP_PATH, COMPONENT_LIST_PATH, 
							"Component", Utils.enum_to_strings(ComponentType))
	CodeGenUtils.update_all_header_file(COMPONENT_LIST_PATH, ALL_COMPONENT_HEADER_PATH, component_include_creator)
	
def get_component_type_str_from_index(index: int) -> str:
	return ComponentType(index).name

def get_component_filename() -> str:
	if COMPONENT_TYPE == ComponentType.Editor:
		return COMPONENT_NAME + "EditorComponent"
	else:
		return COMPONENT_NAME + "Component"

def get_component_full_namespace_filename() -> str:
	partial_name: str = get_component_partial_namespace_filename()
	if ENGINE_NAMESPACE == "":
		return partial_name
	else:
		return ENGINE_NAMESPACE + "::" + partial_name

def get_component_partial_namespace_filename() -> str:
	component_filename: str = get_component_filename()
	if COMPONENT_NAMESPACE == "":
		return component_filename
	else:
		return COMPONENT_NAMESPACE + "::" + component_filename
	
def get_component_full_namespace() -> str:
	full_namespace: str = COMPONENT_NAMESPACE
	#if component or engine or both are empty it means we do not need :: separator
	if full_namespace == "" or ENGINE_NAMESPACE == "":
		return ENGINE_NAMESPACE + full_namespace
	else:
		return ENGINE_NAMESPACE + "::" + full_namespace

def get_system_file_name() -> str:
	return COMPONENT_NAME + "System"

def get_type_dir() -> str:
    return str(COMPONENT_TYPE.name)+ "/"

def get_component_local_path(file_type: CodeGenUtils.FileType) -> str:
	return COMPONENT_TYPE_DIR_PATH + get_type_dir() + get_component_filename() + CodeGenUtils.get_extension(file_type)

def get_system_local_path(file_type: CodeGenUtils.FileType) -> str:
	return SYSTEM_TYPE_DIR_PATH + get_type_dir() + get_system_file_name() + CodeGenUtils.get_extension(file_type)

def component_include_creator(category_index: int, namespace:str, component_name: str) -> str:
	return COMPONENT_TYPE_DIR_PATH + get_component_type_str_from_index(category_index) + "/" + component_name + CodeGenUtils.get_extension(CodeGenUtils.FileType.Hpp)

create_component()
	